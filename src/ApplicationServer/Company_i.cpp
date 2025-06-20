// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later
/**
  \file
  \brief Implementation of the CORBA servant class Organization::Company (Company_i)
 
  \details This file contains the implementation of the `Company_i` class, which serves as the
           server-side CORBA servant for the `Organization::Company` interface.
           It manages employee records using an in-memory map of `EmployeeData` and provides access
           to individual employee servants via CORBA object references.
 
           For demonstration purposes, this implementation uses test data. A future version
           will connect to a database backend. The class also demonstrates how to dynamically
           activate CORBA servants (employees) using the Portable Object Adapter (POA).
 
  \note   The map with the Long key and the data source (`EmployeeData`) is temporary and
          simulate a database. This will later be replaced with a system-backed implementation
          (e.g., connected to a database).

  \version 1.0
  \date    16.05.2025
  \author  Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH
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

 */

#include "Company_i.h"
#include "Tools.h"
#include "my_logging.h"

#include <ranges>
#include <numeric>
#include <algorithm>

Company_i::Company_i(PortableServer::POA_ptr company_poa, PortableServer::POA_ptr employee_poa)
   : employee_poa_(PortableServer::POA::_duplicate(employee_poa)), company_poa_(PortableServer::POA::_duplicate(company_poa)) {
   initializeDatabase();
   log_trace<4>("[Company_i {}] Company Servant {} created", ::getTimeStamp(), strCompanyName);
   }

Company_i::~Company_i() {
   log_trace<4>("[Company_i {}] Company Servant {} destroyed", ::getTimeStamp(), strCompanyName);
   }


void Company_i::initializeDatabase() {
   using namespace std::chrono;
   CORBA::Long emp_no = 99;
   employee_database_[emp_no] = { ++emp_no, "Max",        "Muster",   Organization::MALE,   55'000.00, {2020y, May,       1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Petra",      "Power",    Organization::FEMALE, 62'000.00, {2019y, March,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Klaus",      "Klein",    Organization::MALE,   48'000.00, {2022y, November,  1d}, false };
   employee_database_[emp_no] = { ++emp_no, "Johannes",   "Gerlach",  Organization::MALE,   63'230.00, {2020y, May,       1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Matthias",   "Fehse",    Organization::MALE,   65'500.00, {2020y, December,  1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Gabriele",   "Sommer",   Organization::FEMALE, 70'320.50, {2017y, October,   1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Sandra",     "Mayer",    Organization::FEMALE, 55'100.00, {2020y, February,  1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Vanessa",    "Schmitt",  Organization::FEMALE, 45'500.25, {2020y, April,     1d}, false };
   employee_database_[emp_no] = { ++emp_no, "Christel",   "Rau",      Organization::FEMALE, 52'300.00, {2020y, September, 1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Torsten",    "Gutmann",  Organization::MALE,   73'500.00, {2016y, March,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Stefanie",   "Berger",   Organization::FEMALE, 63'352.25, {2020y, March ,    1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Sarah",      "Mayer",    Organization::FEMALE, 53'250.00, {2020y, August,    1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Harry",      "Deutsch",  Organization::MALE,   61'720.50, {2020y, May,       1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Katharina",  "Keller",   Organization::FEMALE, 71'500.00, {2020y, July,      1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Sophie",     "Hoffmann", Organization::FEMALE, 51'650.25, {2020y, June,      1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Anna",       "Schmidt",  Organization::FEMALE, 63'751.10, {2020y, February,  1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Lea",        "Peters",   Organization::FEMALE, 67'200.00, {2020y, March,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Julian",     "Ziegler",  Organization::MALE,   69'756.20, {2020y, September, 1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Finn",       "Noris",    Organization::MALE,   65'100.75, {2020y, October,   1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Maximilian", "Lang",     Organization::MALE,   67'111.20, {2020y, May,       1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Tim - Leon", "Ziegler",  Organization::MALE,   64'900.60, {2020y, January,   1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Julian",     "Gerlach",  Organization::MALE,   54'222.00, {2020y, March,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Hans",       "Mayer",    Organization::MALE,   66'360.10, {2020y, February,  1d}, false };
   employee_database_[emp_no] = { ++emp_no, "Reinhard",   "Schmidt",  Organization::MALE,   61'200.00, {2019y, October,   1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Petra",      "Winther",  Organization::FEMALE, 72'650.00, {2017y, April,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Julia",      "Schmidt",  Organization::FEMALE, 68'250.00, {2020y, March,     1d}, true };
   employee_database_[emp_no] = { ++emp_no, "Mark",       "Krämer",   Organization::MALE,   46'700.20, {2020y, February,  1d}, true };

   log_trace<4>("[Company_i {}] Database initialized with {} employees.", ::getTimeStamp(), employee_database_.size());
   }

char* Company_i::nameCompany() {
   return CORBA::string_dup(strCompanyName.c_str());
   }

// candidate for own function converTo !!!
Basics::TimePoint Company_i::getTimeStamp() {
   auto now = std::chrono::system_clock::now();
   auto value_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
   Basics::TimePoint tp { .milliseconds_since_epoch = value_milliseconds };
   return tp;
   }



Organization::EmployeeSeq* Company_i::getEmployees() {
   std::println(std::cout, "[Company_i {}] getEmployees() called by client.", ::getTimeStamp());
   auto all_employees_view = employee_database_ | std::views::values;
   return buildEmploySequenceFromRange(all_employees_view);
   }

Organization::EmployeeSeq* Company_i::getActiveEmployees() {
   log_trace<4>("[Company_i {}] getActiveEmployees() called by client.", ::getTimeStamp());
   auto active_employees_view = employee_database_ | std::views::values
                                                   | std::views::filter([](EmployeeData const& e) { return e.isActive; });
   return buildEmploySequenceFromRange(active_employees_view);
   }


double Company_i::getSumSalary() {
   log_trace<4>("[Company_i {}] getSumSalary() called by client.", ::getTimeStamp());

   auto active_salaries = employee_database_ | std::views::values 
                                             | std::views::filter([](EmployeeData const& e) { return e.isActive; })
                                             | std::views::transform([](EmployeeData const& e) { return e.salary; });
   return std::accumulate(active_salaries.begin(), active_salaries.end(), 0.0);
   }

Organization::Employee* Company_i::getEmployee(CORBA::Long personId) {
   log_trace<4>("[Company_i {}] getEmployee() called by client for ID = {}.", ::getTimeStamp(), personId);

   // 1st seek in db
   if (auto it = employee_database_.find(personId); it != employee_database_.end()) [[likely]] {
      try {
         Employee_i* employee_servant = new Employee_i(it->second, employee_poa_.in());

         PortableServer::ObjectId_var oid = employee_poa_->activate_object(employee_servant);
         employee_servant->set_oid(oid);

         CORBA::Object_var obj_ref = employee_poa_->id_to_reference(oid.in());
         Organization::Employee_var employee_ref = Organization::Employee::_narrow(obj_ref.in());

         if(CORBA::is_nil(employee_ref)) {
            std::println(std::cerr, "[Company_i {}] getEmployee(), CORBA Error while narrowing Reference for ID {}",
                                 ::getTimeStamp(), personId);
            return nullptr; // oder eine qualifizierte Fehlerbehandlung ToDo
            }

         log_trace<4>("[Company_i {}] getEmployee() returning Employee* for ID = {}.", ::getTimeStamp(), employee_ref->personId());
         // BESITZWECHLER
         return employee_ref._retn();
         }
      catch(CORBA::Exception const& ex) {
         log_error("[Company_i {}] getEmployee(), CORBA Exception creating dynamic employee ref for ID {}: {}",
                                      ::getTimeStamp(), personId, toString(ex));
         throw CORBA::INTERNAL();
         }
      catch(std::exception const& ex) {
         log_error("[Company_i {}] getEmployee(), C++ Exception creating dynamic employee ref for ID {}: {}",
                                      ::getTimeStamp(), personId, ex.what());
         throw CORBA::INTERNAL();
         }
      }
   else {
      log_error("[Company_i {}] Employee ID with {} not found. Throwing EmployeeNotFound", ::getTimeStamp(), personId);
      Organization::EmployeeNotFound ex;
      ex.requestedId = personId;
      ex.requestedAt = getTimeStamp();
      throw ex;
      }

   }

Organization::EmployeeData* Company_i::getEmployeeData(CORBA::Long personId) {
   log_trace<4>("[Company_i {}] getEmployeeData() called by client for ID = {}.", ::getTimeStamp(), personId);

   // 1st seek employee in company database
   if(auto it = employee_database_.find(personId); it != employee_database_.end()) [[likely]] {
      // 2nd employee found prepare data for transmission
      Organization::EmployeeData* employee_data = createFrom(it->second);
      log_trace<4>("[Company_i {}] getEmployeeData() returning EmployeeData for ID = {}.", ::getTimeStamp(), employee_data->personId);
      return employee_data;
      }
   else {
      log_error("[Company_i {}] Employee ID with {} not found. Throwing EmployeeNotFound", ::getTimeStamp(), personId);
      Organization::EmployeeNotFound ex;
      ex.requestedId = personId;
      ex.requestedAt = getTimeStamp();
      throw ex;
     }

   }
