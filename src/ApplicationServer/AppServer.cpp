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

#include "Tools.h"

#include "OrganizationC.h"

#include "Company_i.h"
#include "Corba_Interfaces.h"

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <tao/PortableServer/LifespanPolicyC.h>
#include <tao/PortableServer/LifespanPolicyA.h>

#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <format>
#include <print>

#include <csignal>

#ifdef _WIN32
   #include <Windows.h>
#endif

using namespace std::string_literals;

/**
  \brief Indicates whether a shutdown has been requested.
 
  \details This atomic boolean flag is set to `true` by the signal handler when a termination signal is received.
           It can be safely read from multiple threads to check whether the application should initiate a graceful shutdown.
 
  \note This flag is intended to be polled in long-running loops or services that need to respond to external termination requests.
 
  \see signal_handler
 */
std::atomic<bool> shutdown_requested = false;


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

static_assert(CORBASkeleton<Company_i>, "Company_i erfüllt nicht das CORBASkeleton-Concept");

int main(int argc, char *argv[]) {

   // -----------------------------------------------------------------------------------
   // (1) Setup: Install signal handlers for graceful shutdown on SIGINT or SIGTERM
   // -----------------------------------------------------------------------------------
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

   const std::string strAppl = "Time Tracking App Server"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);  
#endif
   std::string strName = "GlobalCorp/CompanyService"s;
   try {
      CorbaServer<Company_i> server(strAppl, argc, argv);

      CORBA::PolicyList empl_pol;
      empl_pol.length(2);
      empl_pol[0] = server.root_poa()->create_lifespan_policy(PortableServer::TRANSIENT);
      empl_pol[1] = server.root_poa()->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);

      PortableServer::POA_var employee_poa = server.root_poa()->create_POA("EmployeePOA", server.poa_manager(), empl_pol);
      for (uint32_t i = 0; i < empl_pol.length(); ++i) empl_pol[i]->destroy();

      server.RegisterServant(strName, [poa = std::move(employee_poa)]() mutable {
                                         if(!CORBA::is_nil(poa.in())) {
                                            poa->destroy(true, true);
                                            log_trace<2>("[independent Lambda Fuction {}] Employee POA destroyed.", ::getTimeStamp());
                                            }
                                         }, 
                             new Company_i(server.servant_poa(), employee_poa.in()));

      server.run(shutdown_requested);
      }
   catch (CORBA::Exception const& ex) {
      log_error("[{} {}] CORBA Exception caught: {}", strAppl, ::getTimeStamp(), toString(ex));
      }
   catch (std::exception const& ex) {
      log_error("[{} {}] std::exception caught: {}", strAppl, ::getTimeStamp(), ex.what());
      }
   catch (...) {
      log_error("[{} {}] Unknown exception caught.", strAppl, ::getTimeStamp());
      return 1;
      }

   log_state("[{} {}] Server exited successfully.", strAppl, ::getTimeStamp());
   return 0;
   }
