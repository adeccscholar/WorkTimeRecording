// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Client application for employee self-service access to personal data.

  \details This module implements the CORBA-based client used by employees to 
           view and interact with their own time tracking and organizational data. 
           It connects to the appropriate CORBA services via the TAO ORB and 
           performs secure, read-only access to individual records as published by 
           the Application Server.

  \note This software component is part of the adecc Scholar project and is designed
        for educational and productive use in distributed, service-based C++ systems.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \license This program is free software: you can redistribute it and/or modify it
           under the terms of the GNU General Public License, version 3,
           as published by the Free Software Foundation.

           This program is distributed in the hope that it will be useful,
           but WITHOUT ANY WARRANTY; without even the implied warranty of
           MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
           See the GNU General Public License for more details.

           You should have received a copy of the GNU General Public License
           along with this program. If not, see <https://www.gnu.org/licenses/>.

  \version 1.0
  \date 2025-05-26
 */
#include "Tools.h"
#include "my_logging.h"
#include "Corba_CombiInterface.h"

#include <BasicUtils.h>
#include <CorbaUtils.h>

#include "Employee_Tools.h"

#include <OrganizationC.h>

#include <tao/corba.h>
#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <print>

using namespace std::string_literals;

#ifdef _WIN32
#include <Windows.h>
#endif

static_assert(CORBAStub<Organization::Company>, "Organization::Company does not satisfy the CORBAStub concept");
static_assert(CORBAStub<Organization::Employee>, "Organization::Employee does not satisfy the CORBAStub concept");
static_assert(CORBAStubWithDestroy<Organization::Employee>, "Organization::Employee does not satisfy the CORBAStubWithDestroy concept");

int main(int argc, char *argv[]) {
   const std::string strMainClient = "Client"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   // Platzhalter für Variable
   log_state("[{} {}] Client Testprogram for Worktime Tracking started.", strMainClient, ::getTimeStamp());
   try {
      CORBAClientServer<Stub<Organization::Company>> factories("CORBA Factories", argc, argv, "GlobalCorp/CompanyService"s);

      for (auto const& name : factories.get_names()) std::println(std::cout, "{}", name);

      //auto company = [&factories]() { return std::get<0>(factories.vars());  };
      auto company = [&factories]() { return factories.client().get<0>();  };
      std::println(std::cout, "Server TimeStamp: {}", getTimeStamp(convert<std::chrono::system_clock::time_point>(company()->getTimeStamp())));
      std::println(std::cout, "Company {}, to paid salaries {:.2f}", company()->nameCompany(), company()->getSumSalary());
      GetEmployee(company(), 105);
      GetEmployees(company());
      Organization::Employee_var employee = company()->getEmployee(180);

      }
   catch(Organization::EmployeeNotFound const& ex) {
      // Safety net, in case the exception occurs outside the specific try-catch block
      log_error("[{} {}] unhandled 'EmployNotFound'- Exception with Employee ID: {} at {}.", 
                 strMainClient, ::getTimeStamp(), ex.requestedId, getTimeStamp(convert<std::chrono::system_clock::time_point>(ex.requestedAt)));
      return 1;
      }
   catch(CORBA::COMM_FAILURE const& ex) {
      log_error("[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 2;
      }
   catch(CORBA::TRANSIENT const& ex) {
      log_error("[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 3;
      }
   catch(CORBA::Exception const& ex) {
      log_error("[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 4;
      }
   catch(std::exception const& ex) {
      log_error("[{} {}] C++ Standard Exception: {}", strMainClient, ::getTimeStamp(), ex.what());
      }
   catch(...) {
      log_error("[{} {}] unknown exception caught, critical error.", strMainClient, ::getTimeStamp());
      return 101;
      }


   log_state("[{} {}] Client exited gracefully.", strMainClient, ::getTimeStamp());

   // Placeholder for deferred output
   return 0;
    }