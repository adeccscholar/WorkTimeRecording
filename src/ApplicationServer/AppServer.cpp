// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file 
  \brief Entry point for the application server implementation for the distributed time-tracking system 
 
  \details This project is developed in the adecc Scholar project as free learning project. Its
           demonstrates the development of a distributed, cross-platform  time-tracking system for enterprise 
           environments. It is part of the open-source educational initiative **adecc Scholar**, which began in 
           autumn 2020 during the COVID-19 pandemic.
 
  \details The goal is to teach modern C++ techniques (including C++23) in conjunction with open-source technologies 
           such as Boost, CORBA/TAO, Qt6, and embedded programming on Raspberry Pi.
 
  \details The system consists of several components:
            - Application Server (Ubuntu Linux, CORBA/ODBC)
            - Database Server (Windows Server with MS SQL)
            - Desktop Clients (Qt6-based UI)
            - Raspberry Pi Clients (RFID, sensors, GPIO, I2C, SPI)
 
  \details Communication is handled via CORBA (TAO); finite state machines are implemented using Boost.SML;
           GUI components are created with Qt6. Development is done in Visual Studio.
 
  \note The project also supports Embarcadero C++ Builder (VCL, FMX) to illustrate integration of legacy systems 
        into modern distributed architectures.
 
  \author Volker Hillmann (adecc Systemhaus GmbH)
 
  \copyright Copyright © adecc Systemhaus GmbH 2021–2025
 
  \license This project is mostly licensed under the GNU General Public License v3.0.
           For more information, see the LICENSE file.
 
  \version 1.0
  \date    09.05.2025
 */


// later delete stubs from directory and make this to standard include

#include "OrganizationC.h"

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <tao/PortableServer/LifespanPolicyC.h>
#include <tao/PortableServer/LifespanPolicyA.h>

#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <format>
#include <print>

#include <csignal>

#ifdef _WIN32
   #include <Windows.h>
#endif

/**
  \brief Indicates whether a shutdown has been requested.
 
  \details This atomic boolean flag is set to `true` by the signal handler when a termination signal is received.
           It can be safely read from multiple threads to check whether the application should initiate a graceful shutdown.
 
  \note This flag is intended to be polled in long-running loops or services that need to respond to external termination requests.
 
  \see signal_handler
 */
std::atomic<bool> shutdown_requested = false;

/**
  \brief Generates a formatted timestamp string from a given time point.
 
  \details This function converts a given `std::chrono::time_point` to a human-readable, formatted
           timestamp string in the local time zone. If no time point is provided, it defaults to the
           current system time.
 
  \details The resulting string includes the date, time (with hours, minutes, and seconds), and milliseconds,
           formatted as `DD.MM.YYYY HH:MM:SS,mmm`.
 
  \param now The time point to format. Defaults to the current system time if not provided.
             Must be a valid time point based on `std::chrono::system_clock`.
 
  \return A formatted string representing the local timestamp, including milliseconds.
 
  \pre  The system must support `std::chrono::current_zone()` to resolve the local time zone.
  \post No side effects; the function only returns a formatted string.
 
  \note This function requires C++20 for `std::format` and `std::chrono::current_zone()`.
 */
inline std::string getTimeStamp(std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now()) {
   auto const cz_ts  = std::chrono::current_zone()->to_local(now);
   auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
   return std::format("{:%d.%m.%Y %X},{:03d}", cz_ts, millis.count());
   }

/**
  \brief Converts an Organization::YearMonthDay to a std::chrono::year_month_day.
 
  \details This function takes a `YearMonthDay` struct, typically generated from the IDL definition in
           `Organization.idl`, and converts it into a C++20 `std::chrono::year_month_day` object.
           It is intended to bridge between IDL-generated C-style time representations and the modern
           C++ date/time facilities.
 
  \param ymd A reference to an `Organization::YearMonthDay` struct containing year, month, and day values.
              - `year` is interpreted directly as a Gregorian calendar year.
              - `month` and `day` are expected to be valid and in 1-based indexing.
 
  \return A `std::chrono::year_month_day` object representing the same date.
 
  \pre The `ymd.month` must be in the range [1, 12] and `ymd.day` must be in a valid day range for the given month/year.
       No range checking is performed inside this function.
 
  \post The returned object is suitable for use with the C++20 `<chrono>` library.
 
  \note This function does not validate calendar correctness (e.g., February 30 will not raise an error).
 
  \see Organization::YearMonthDay
  \see std::chrono::year_month_day
 */
inline std::chrono::year_month_day convertTo(Organization::YearMonthDay const& ymd) {
   return std::chrono::year_month_day { std::chrono::year  { ymd.year },
                                        std::chrono::month { static_cast<unsigned int>(ymd.month) },
                                        std::chrono::day   { static_cast<unsigned int>(ymd.day) }
                                       };
   }

/**
  \brief Converts an Organization::TimePoint to a std::chrono::system_clock::time_point.
 
  \details This function transforms a CORBA-generated `Organization::TimePoint` struct, which represents
           a Unix timestamp as milliseconds since the epoch, into a `std::chrono::system_clock::time_point`.
           This allows the timestamp to be used with modern C++ time utilities.
 
  \param tp A reference to an `Organization::TimePoint` containing a timestamp as milliseconds since the Unix epoch.
            This value is interpreted as the number of milliseconds since 1970-01-01T00:00:00Z (UTC).
 
  \return A `std::chrono::system_clock::time_point` representing the same moment in time.
 
  \pre The `tp.milliseconds_since_epoch` must be a valid 64-bit timestamp. No range checking is performed.
 
  \post The returned time_point can be used in standard C++ time calculations, formatting, and conversions.
 
  \note This function assumes that the CORBA timestamp is in UTC.
 
  \see Organization::TimePoint
  \see std::chrono::system_clock::time_point
 */
inline std::chrono::system_clock::time_point convertTo(Organization::TimePoint const& tp) {
   return std::chrono::system_clock::time_point(std::chrono::milliseconds(tp.milliseconds_since_epoch));
   }

/**
   \brief Converts a raw CORBA string (char*) to a std::string and frees the allocated memory.
 
   \details This function takes a raw `char*` string returned by a CORBA operation, converts it into a `std::string`,
            and frees the memory that was allocated by the CORBA ORB using `CORBA::string_free`.
            It ensures proper memory management by deallocating the memory after copying the string's contents to 
            the `std::string`.
 
   \details This method is only required on the client side if you want to use a `std::string` instead of 
            a `CORBA::String_var`. This is worthwhile if you want to continue using it in the further course.

   \param s A raw `char*` string returned by a CORBA method. If `s` is `nullptr`, an empty `std::string` is returned.
   \return A `std::string` containing the same contents as the provided `char*`, or an empty string if `s` is `nullptr`.
 
   \warning The provided `char*` pointer will be freed using `CORBA::string_free`. This function assumes ownership
            of the memory and should only be used with strings allocated by CORBA operations.
 */
inline std::string toString(char* s) {
   std::string result = s ? std::string{ s } : std::string{};
   CORBA::string_free(s);  // important for CORBA::String type return values, the memory would allocated from orb!
   return result;
}

/**
  \brief Converts a CORBA::COMM_FAILURE exception to a detailed human-readable string.
 
  \details This utility function serializes a `CORBA::COMM_FAILURE` exception and appends
           diagnostic suggestions to help users understand and potentially resolve the communication issue.
 
  \param ex A constant reference to the `CORBA::COMM_FAILURE` exception object.
 
  \return A `std::string` containing the formatted exception message, along with hints for troubleshooting.
 
  \pre The `CORBA::COMM_FAILURE` type must support insertion into an `std::ostream` (`operator<<`).
  \post The returned string includes the exception details and suggestions such as checking
        server and NameService availability.
 
  \note Useful for log output or displaying user-facing diagnostics when communication with the CORBA server fails.
 
  \see CORBA::COMM_FAILURE
 */
inline std::string toString(CORBA::COMM_FAILURE const& ex) {
   std::ostringstream os;
   os << "CORBA Communication Failure: " << ex << '\n'
      << "Is the server running and reachable?\n"
      << "Is the NameService running and reachable via ORBInitRef?";
   return os.str();
}

/**
  \brief Converts a CORBA::TRANSIENT exception to a detailed human-readable string.
 
  \details This utility function serializes a `CORBA::TRANSIENT` exception and adds explanatory
           notes to help identify transient issues such as server startup/shutdown delays.
 
  \param ex A constant reference to the `CORBA::TRANSIENT` exception object.
 
  \return A `std::string` containing the formatted exception message, including context-specific advice.
 
  \pre The `CORBA::TRANSIENT` type must support insertion into an `std::ostream` (`operator<<`).
  \post The returned string explains the transient nature of the exception and recommends retrying the operation.
 
  \note This function is intended for improving error reporting and debugging in distributed CORBA environments.
 
  \see CORBA::TRANSIENT
 */
inline std::string toString(CORBA::TRANSIENT const& ex) {
   std::ostringstream os;
   os << "CORBA Transient Exception: " << ex << '\n'
      << "The server might be starting up, shutting down, or busy.\n"
      << "Try again later.";
   return os.str();
   }

/**
  \brief Converts a CORBA::Exception to a human-readable string.
 
  \details This utility function serializes a CORBA exception into a string using an output string stream.
           It is useful for logging or displaying exception details in a user-friendly format.
 
  \param ex A constant reference to a `CORBA::Exception` object to be converted.
 
  \return A `std::string` containing the formatted exception information.
 
  \pre The `CORBA::Exception` type must support insertion into an `std::ostream` (i.e., `operator<<` must be overloaded).
  \post The returned string will contain the serialized content of the exception, suitable for logs or error messages.
 
  \note This function performs no formatting or parsing — it directly relies on the `operator<<` overload for `CORBA::Exception`.
 
  \see CORBA::Exception
 */
inline std::string toString(CORBA::Exception const& ex) {
   std::ostringstream os;
   os << ex;
   return os.str();
   }


/**
  \brief Handles incoming POSIX signals and triggers a graceful shutdown.
 
  \details This function is designed to be registered as a signal handler (e.g., via `std::signal`). 
           Upon receiving a signal, it logs the reception and sets a global shutdown flag (`shutdown_requested`) 
           to `true`, signaling the application to begin a graceful shutdown procedure.
 
  \param sig_num The signal number received (e.g., SIGINT, SIGTERM).
 
  \return None.
 
  \pre  This function must be registered using `std::signal` or an equivalent mechanism.
  \post The global variable `shutdown_requested` will be set to `true`.
 
  \note This function writes directly to `std::cout` using `std::println`. It is **not** async-signal-safe,
        and should be used only in controlled environments (e.g., small CLI tools) where this is acceptable.
 
  \warning Avoid calling non-signal-safe functions in production signal handlers; for critical applications,
           use safer constructs like `sig_atomic_t` and defer processing to the main loop.
 
  \see shutdown_requested
  \see std::signal
 */
void signal_handler(int sig_num) {
   std::println(std::cout);
   std::println(std::cout, "[signal handler {}] signal {} received. Requesting shutdown ...", getTimeStamp(), sig_num);
   shutdown_requested = true;
   }

int main(int argc, char *argv[]) {
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   // 1. Corba initializieren, Achtung, Parameter werden azzimiliert
   std::println("[Application TT {}] Server starting ...", getTimeStamp());
   CORBA::ORB_var orb_global = CORBA::ORB_init(argc, argv);
   try {
      if (CORBA::is_nil(orb_global.in())) throw std::runtime_error("Failed to initialized the global ORB Object.");
      std::println("[Application TT {}] Corba is intialized.", getTimeStamp());

      // 2. Referenz zum RootPOA (Portable Object Adapter)
      CORBA::Object_var poa_object = orb_global->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object.in());
      if (CORBA::is_nil(root_poa.in())) throw std::runtime_error("Failed to narrow the POA");

      PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
      poa_manager->activate();
      std::println("[Application TT {}] RootPOA obtained and POAManager is activated.", getTimeStamp());

      // 3. Policies für die beiden POAs erstellen
      // Policy für den persistenten PA (für Company /Organization)
      CORBA::PolicyList comp_pol;
      comp_pol.length(1);
      comp_pol[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);

      // Policy für den transienten POA (für Employee)
      CORBA::PolicyList empl_pol;
      empl_pol.length(2);
      empl_pol[0] = root_poa->create_lifespan_policy(PortableServer::TRANSIENT);
      empl_pol[1] = root_poa->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);

      // 4. POAs als "Kind" des RootPOA erstellen
      PortableServer::POA_var company_poa  = root_poa->create_POA("CompanyPOA", poa_manager.in(), comp_pol);
      PortableServer::POA_var employee_poa = root_poa->create_POA("EmployeePOA", poa_manager.in(), empl_pol);

      for (uint32_t i = 0; i < 1; ++i) comp_pol[i]->destroy();
      for (uint32_t i = 0; i < 2; ++i) empl_pol[i]->destroy();

      std::println("[Application TT {}] Persistent CompanyPOA and transient EmployeePOA created.", getTimeStamp());

      // 5. Servant erstellen

      }
   catch(CORBA::Exception const& ex) {
      std::println(std::cerr, "[Application TT {}] CORBA Exception caught: {}", getTimeStamp(), toString(ex));
      }
   catch(std::exception const& ex) {
      std::println(std::cerr, "[Application TT {}] std::exception caught: {}", getTimeStamp(), ex.what());
      }
   return 0;
   }
