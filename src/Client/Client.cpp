// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Client application for employee self-service access to personal data.

  \details
  This module implements the CORBA-based client used by employees to view and
  interact with their own time tracking and organizational data. It connects to
  the appropriate CORBA services via the TAO ORB and performs secure, read-only
  access to individual records as published by the Application Server.

  \note
  This software component is part of the adecc Scholar project and is designed
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
#include "Corba_Interfaces.h"

#include <OrganizationC.h>

#include "Employee_Tools.h"

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

static_assert(CORBAStub<Organization::Company>, "Organization::Company erfüllt nicht das CORBAStub-Concept");
static_assert(CORBAStub<Organization::Employee>, "Organization::Employee erfüllt nicht das CORBAStub-Concept");
static_assert(CORBAStubWithDestroy<Organization::Employee>, "Organization::Employee erfüllt nicht CORBAStubWithDestroy");

int main(int argc, char *argv[]) {
   const std::string strMainClient = "Client"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   // Platzhalter für Variable
   log_state("[{} {}] Client Testprogram for Worktime Tracking started.", strMainClient, ::getTimeStamp());
   try {
      //CORBAStubHolder<Organization::Company, Organization::Employee> test("Test", argc, argv, "Param1", "Param2");
      //*
      ORBClient<Organization::Company> orb("ORB + Company"s, argc, argv, "GlobalCorp/CompanyService"s);
      std::println(std::cout, "Server TimeStamp: {}", getTimeStamp(convertTo(orb.factory()->getTimeStamp())));
      std::println(std::cout, "Company {}, to paid salaries {:.2f}", orb.factory()->nameCompany(), orb.factory()->getSumSalary());
      GetEmployee(orb.factory(), 105);
      //GetEmployees(orb.factory());
      Organization::Employee_var employee = orb.factory()->getEmployee(180);
      //*/

      /*
      // 1st ORB initialisieren
      CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
      std::println(std::cout, "[{} {}] ORB initialized.", strMainClient, ::getTimeStamp());
      
      // 2nd connecting to nameservice
      CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());
      if (CORBA::is_nil(naming_context.in())) throw std::runtime_error("Failed to narrow Naming Context.");
      std::println(std::cout, "[{} {}] Naming Service Context obtained.", strMainClient, ::getTimeStamp());

      // 3rd Company Object
      std::string strName = "GlobalCorp/CompanyService"s;
      CosNaming::Name name;
      name.length(1);
      name[0].id = CORBA::string_dup(strName.c_str());
      name[0].kind = CORBA::string_dup("Object");

      std::println(std::cout, "[{} {}] Resolving {}.", strMainClient, ::getTimeStamp(), strName);
      CORBA::Object_var company_obj = naming_context->resolve(name);
      Organization::Company_var company_var = Organization::Company::_narrow(company_obj.in());
      if (CORBA::is_nil(company_var.in())) throw std::runtime_error("Failed to narrow Company reference.");
      std::println(std::cout, "[{} {}] Successfully obtained reference to company {}.", strMainClient, ::getTimeStamp(), strName);

      // 4th using the company reference
      std::println(std::cout, "Server TimeStamp: {}", getTimeStamp(convertTo(company_var->getTimeStamp())));
      std::println(std::cout, "Company {}, to paid salaries {:.2f}", company_var->nameCompany(), company_var->getSumSalary());

      Organization::Employee_var employee = company_var->getEmployee(180);
      // Problematisch, überspringt Freigabe

      // ----------------------------------------------------------------------------------------------------------------------

      while (orb->work_pending()) orb->perform_work();
      orb->destroy();
      std::println(std::cout, "[{} {}] ORB destroyed.", strMainClient, ::getTimeStamp());
      */
      }
   catch(Organization::EmployeeNotFound const& ex) {
      // Notleine falls Exception vorher nicht behandelt wurde
      log_error("[{} {}] unhandled 'EmployNotFound'- Exception with Employee ID: {} at {}.", 
                 strMainClient, ::getTimeStamp(), ex.requestedId, getTimeStamp(convertTo(ex.requestedAt)));
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

   // Platzhalter für verzögerte Ausgabe
   return 0;
    }