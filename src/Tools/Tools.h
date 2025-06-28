// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Header-only utility functions for CORBA and time conversions in the distributed time-tracking system

  \details This header provides a collection of reusable utility functions used throughout the distributed
           time-tracking system developed in the **adecc Scholar** project. These utilities simplify and standardize
           the handling of CORBA exceptions and types, as well as modern C++ time formatting and conversion.

  \details Key functionality includes:
           - Formatting time points into localized human-readable strings using C++20 `<chrono>` and `std::format`.
           - Converting IDL-defined CORBA time structures into `std::chrono` types for modern C++ time handling.
           - Safely transforming CORBA strings and exceptions into `std::string` objects with proper memory handling.
           - Providing detailed diagnostics for CORBA communication exceptions.

  \details This project is developed in the adecc Scholar project as free learning project. It demonstrates the
           development of a distributed, cross-platform time-tracking system for enterprise environments. It is
           part of the open-source educational initiative **adecc Scholar**, which began in autumn 2020 during
           the COVID-19 pandemic.

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

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.


  \version 1.0
  \date    09.05.2025
*/

#pragma once

#include "BasicsC.h"

#include <tao/corba.h>

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <format>

using namespace std::string_literals;
using namespace std::string_view_literals;

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
   //auto const cz_ts = std::chrono::locate_zone("Europe/Berlin"sv)->to_local(now);
   //auto const cz_ts = std::chrono::current_zone()->to_local(now);
   auto const cz_ts = now;

   auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
   return std::format("{:%d.%m.%Y %X},{:03d}", cz_ts, millis.count());
}

/**
  \brief Converts an Basics::YearMonthDay to a std::chrono::year_month_day.

  \details This function takes a `YearMonthDay` struct, typically generated from the IDL definition in
           `BAsics.idl`, and converts it into a C++20 `std::chrono::year_month_day` object.
           It is intended to bridge between IDL-generated C-style time representations and the modern
           C++ date/time facilities.

  \param ymd A reference to an `Basics::YearMonthDay` struct containing year, month, and day values.
              - `year` is interpreted directly as a Gregorian calendar year.
              - `month` and `day` are expected to be valid and in 1-based indexing.

  \return A `std::chrono::year_month_day` object representing the same date.

  \pre The `ymd.month` must be in the range [1, 12] and `ymd.day` must be in a valid day range for the given month/year.
       No range checking is performed inside this function.

  \post The returned object is suitable for use with the C++20 `<chrono>` library.

  \note This function does not validate calendar correctness (e.g., February 30 will not raise an error).

  \see Basics::YearMonthDay
  \see std::chrono::year_month_day
 */
inline std::chrono::year_month_day convertTo(Basics::YearMonthDay const& ymd) {
   return std::chrono::year_month_day{ std::chrono::year  { ymd.year },
                                       std::chrono::month { static_cast<unsigned int>(ymd.month) },
                                       std::chrono::day   { static_cast<unsigned int>(ymd.day) }
                                     };
   }

/**
  \brief Converts a std::chrono::year_month_day to an Organization::YearMonthDay.

  \details This function transforms a C++20 `std::chrono::year_month_day` object into a CORBA-compatible
           `Organization::YearMonthDay` struct, enabling the use of modern C++ date types in CORBA interfaces.

  \param ymd A `std::chrono::year_month_day` object representing a specific calendar date.
             - `ymd.year()`, `ymd.month()`, and `ymd.day()` are extracted and converted to standard integers.

  \return An `Organization::YearMonthDay` struct containing the same date information, using 1-based indexing.

  \pre The input `ymd` should represent a valid date. No range or calendar validation is performed.

  \post The returned struct is suitable for transmission over CORBA.

  \note This function assumes Gregorian calendar conventions.

  \see std::chrono::year_month_day
  \see Organization::YearMonthDay
 */
inline Basics::YearMonthDay convertTo(std::chrono::year_month_day const& ymd) {
   Basics::YearMonthDay result;
   result.year  = static_cast<CORBA::Long>(int(ymd.year()));
   result.month = static_cast<CORBA::Short>(unsigned(ymd.month()));
   result.day   = static_cast<CORBA::Short>(unsigned(ymd.day()));
   return result;
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
inline std::chrono::system_clock::time_point convertTo(Basics::TimePoint const& tp) {
   return std::chrono::system_clock::time_point(std::chrono::milliseconds(tp.milliseconds_since_epoch));
   }

/**
  \brief Converts a std::chrono::system_clock::time_point to an Organization::TimePoint.

  \details This function transforms a C++ `std::chrono::system_clock::time_point` into a CORBA-compatible
           `Organization::TimePoint` structure by computing the number of milliseconds since the Unix epoch.
           This allows time values to be transmitted over CORBA interfaces.

  \param now A `std::chrono::system_clock::time_point` representing the moment in time to convert.
             Defaults to the current system time if no parameter is provided.

  \return An `Organization::TimePoint` structure containing the number of milliseconds since 1970-01-01T00:00:00Z (UTC).

  \pre The `now` parameter must represent a valid point in time from `std::chrono::system_clock`.

  \post The returned `Organization::TimePoint` can be safely sent through CORBA interfaces.

  \note This function assumes that the input time_point is in UTC.

  \see std::chrono::system_clock::time_point
  \see Organization::TimePoint
 */
inline Basics::TimePoint converTo(std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now()) {
   auto value_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
   Basics::TimePoint tp{ .milliseconds_since_epoch = value_milliseconds };
   return tp;
   }

/**
 \brief Converts a Basics::TimePoint to a formatted string.

 \param tp The CORBA TimePoint to convert.
 \return A human-readable timestamp string.
 */
inline std::string toString(Basics::TimePoint tp) {
   return getTimeStamp(convertTo(tp));
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
   \brief Converts a `CORBA::String_var` to a `std::string` without taking ownership.

   \details This function safely converts a `CORBA::String_var`, which manages its own memory,
            into a standard C++ `std::string`. It uses the `in()` method to obtain a
            `const char*` representation without altering or freeing the underlying CORBA-managed memory.

   \details Unlike the overload that takes a raw `char*`, this variant does not free any memory,
            as `CORBA::String_var` handles its own resource management through RAII.

   \param s A constant reference to a `CORBA::String_var` returned by a CORBA operation.
   \return A `std::string` containing the same contents as the CORBA string.

   \note This method is useful when working with CORBA strings in modern C++ codebases and avoids manual memory handling.
 */
inline std::string toString(CORBA::String_var const& s) {
   return std::string{ s.in() };  // safe: in() returns const char*, ownership retained
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
   os << "CORBA Exception: " << ex;
   return os.str();
}
