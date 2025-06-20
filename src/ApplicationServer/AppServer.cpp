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


// later delete stubs from directory and make this to standard include

#include "Tools.h"

#include <adecc_Database/MyDatabase.h>
#include <adecc_Database/MyDatabaseExceptions.h>

#include "OrganizationC.h"

#include "Company_i.h"
#include "Corba_Interfaces.h"
#include "Corba_CombiInterface.h"

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <tao/PortableServer/LifespanPolicyC.h>
#include <tao/PortableServer/LifespanPolicyA.h>

#include <orbsvcs/CosNamingC.h>

#include <QtCore/QCoreApplication>
#include <QHostInfo>

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
//using namespace std::chrono_literals;

/**
  \brief Indicates whether a shutdown has been requested.
 
  \details This atomic boolean flag is set to `true` by the signal handler when a termination signal is received.
           It can be safely read from multiple threads to check whether the application should initiate a graceful shutdown.
 
  \note This flag is intended to be polled in long-running loops or services that need to respond to external termination requests.
 
  \see signal_handler
 */
std::atomic<bool> shutdown_requested = false;

using concrete_db_server = TMyMSSQL;
using concrete_framework = TMyQtDb<concrete_db_server>;
using concrete_db_connection = TMyDatabase<TMyQtDb, concrete_db_server>;
using concrete_query = TMyQuery<TMyQtDb, concrete_db_server>;


void Connect(concrete_db_connection& database,
             std::string const& strDatabase, std::string const& strServer, std::string const& strDomain,
             std::string const& strInstance) {

   auto BuildServerName = [&strDomain](std::string const& strServer, std::string const& strInstance) {
      if (strDomain.empty()) {
         if (strInstance.empty()) return strServer;
         else return strServer + "\\"s + strInstance;
         }
      else {
         if (strInstance.empty()) return strServer + "."s + strDomain;
         else return strServer + "."s + strDomain + "\\"s + strInstance;
         }
      };

   TMyCredential my_cred;

   database = { TMyMSSQL { strDatabase } };
   my_cred = { "", "", true };
   
   database += std::forward<TMyCredential>(my_cred);
   database.Open();
   std::println(std::cout, "Application is connected to: {}", database.GetInformations());
}


/**
  \brief Handles incoming POSIX signals and triggers a graceful shutdown.
 
  \details This function is designed to be registered as a signal handler (e.g., via `std::signal`). 
           Upon receiving a signal, it logs the reception and sets a global shutdown flag (`shutdown_requested`) 
           to `true`, signaling the application to begin a graceful shutdown procedure.
 
  \param sig_num The signal number received (e.g., SIGINT, SIGTERM).
 
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
   QCoreApplication a(argc, argv);




   // -----------------------------------------------------------------------------------
   // (1) Setup: Install signal handlers for graceful shutdown on SIGINT or SIGTERM
   // -----------------------------------------------------------------------------------
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

   const std::string strAppl = "Time Tracking App Server"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);  
   QCoreApplication::addLibraryPath("D:/Qt/6.6.2/msvc2019_64/plugins");
#endif
   std::string strName = "GlobalCorp/CompanyService"s;

   // Determine computer name and network domain
   QString   fullHostName = QHostInfo::localHostName();
   QHostInfo hostInfo = QHostInfo::fromName(fullHostName);
   std::string hostname = hostInfo.hostName().toStdString(); // computer name
   std::string localDomain = hostInfo.localDomainName().toStdString(); // used network domain, does not work under Windows 

   std::println("Host: {}", hostname);
   std::println("Domäne: {}", localDomain);

   concrete_db_connection database;
   Connect(database, "Test_Personen"s, "DESKTOP-UR8733U"s, localDomain, ""s);

   auto query = database.CreateQuery("SELECT ID, Name, Firstname, BirthName, FormOfAddress, "s +
      "FamilyStatus, FamilyStatusSince, Birthday, Notes, FullName "s +
      "FROM Person "s
      "WHERE Name = :keyName AND Firstname = :keyFirstname"s);

   if (query.Execute({ { "keyName", "Braun", true },   {"keyFirstname", "Isabelle", true } }); !query.IsEof()) {
      std::println("{}: {}", query.Get <int>("ID").value_or(0), query.Get<std::string>("FullName").value_or(""));
      }


   {
      /*
      CORBAClientServer<Skel<Company_i>, Stub<Organization::Company>> wrapper("CORBA Factories"s, argc, argv, "GlobalCorp/CompanyService"s );
      //CORBAClientServer<Skel<Company_i>> wrapper("CORBA Factories"s, argc, argv);

      CORBA::PolicyList empl_pol;
      empl_pol.length(2);
      empl_pol[0] = wrapper.root_poa()->create_lifespan_policy(PortableServer::TRANSIENT);
      empl_pol[1] = wrapper.root_poa()->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);

      PortableServer::POA_var employee_poa = wrapper.root_poa()->create_POA("EmployeePOA", wrapper.poa_manager(), empl_pol);
      for (uint32_t i = 0; i < empl_pol.length(); ++i) empl_pol[i]->destroy();

      wrapper.register_servant<0>(strName, [poa = std::move(employee_poa)]() mutable {
                                 if (!CORBA::is_nil(poa.in())) {
                                    poa->destroy(true, true);
                                    log_trace<2>("[independent Lambda Fuction {}] Employee POA destroyed.", ::getTimeStamp());
                                    }
                                 },
                                  new Company_i(wrapper.servant_poa(), employee_poa.in()));
      auto company = [&wrapper]() { return wrapper.client().get<0>();  };
      
      wrapper.run(shutdown_requested);

      */
   }
   
   try {
      CORBAServer<Company_i> server(strAppl, argc, argv, std::chrono::milliseconds(500));
 
      auto CreateTransient = [](PortableServer::POA_ptr poa) {
         CORBA::PolicyList pol_list;
         pol_list.length(2);
         pol_list[0] = poa->create_lifespan_policy(PortableServer::TRANSIENT);
         pol_list[1] = poa->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);
         return pol_list;
         };

      auto empl_pol = CreateTransient(server.root_poa());
      PortableServer::POA_var employee_poa = server.root_poa()->create_POA("EmployeePOA", server.poa_manager(), empl_pol);
      for (uint32_t i = 0; i < empl_pol.length(); ++i) empl_pol[i]->destroy();

      server.register_servant<0>(strName, [poa = std::move(employee_poa)]() mutable {
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
