/**
  \file
  \brief CORBA client-side utility functions for accessing employee data.

  \details
  This file defines a set of helper functions to interact with the Organization module
  of the CORBA server. It includes logic for retrieving and displaying single employees
  or full lists, formatting timestamps, and logging diagnostic information.
  The functions use TAO's CORBA implementation and follow RAII and logging standards
  established in the adecc Scholar project.

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
  \date 2025-06-01
 */

#pragma once

#include "Tools.h"
#include "my_logging.h"

#include "Corba_Interfaces.h"

#include <OrganizationC.h>

#include <tao/corba.h>
#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <format>
#include <print>

using namespace std::string_literals;

/**
 \brief Converts a Basics::TimePoint to a formatted string.

 \param tp The CORBA TimePoint to convert.
 \return A human-readable timestamp string.
 */
inline std::string toString(Basics::TimePoint tp) {
   return getTimeStamp(convertTo(tp));
   }

/**
 \brief Retrieves the current timestamp from a given Company object.

 \param comp_in Pointer to the CORBA Company interface.
 \return A timestamp string retrieved from the company server object.
 \throws std::runtime_error if the company reference is nil.
 */
inline std::string getTimeStamp(Organization::Company_ptr comp_in) {
   if (!CORBA::is_nil(comp_in)) [[likely]] {
      return getTimeStamp(convertTo(comp_in->getTimeStamp()));
      }
   else throw std::runtime_error("company is nil");
   }

/**
 \brief Displays key information for a given employee object.

 \param out Output stream to write the formatted data to.
 \param employee CORBA employee object to show.
 \throws std::runtime_error if the employee reference is nil.
 */
inline void ShowEmployee(std::ostream& out, Organization::Employee_ptr employee) {
   //if (!CORBA::is_nil(employee)) [[likely]] {
      //CORBA::String_var fullName = employee->getFullName();   // fullName.in()
      //CORBA::Boolean active = employee->isActive();
      std::println(out, "ID: {:>4}, Name: {:<25}, Status: {:<3}, Salary: {:>10.2f}", employee->personId(),
                         toString(employee->getFullName()), (employee->isActive() ? "Yes" : "No"), employee->salary());
   //   }
   //else {
   //   throw std::runtime_error(std::format("[ShowEmployee: {}] ERROR: employee is nil unexpectedly.", ::getTimeStamp()));
   //   }
   }

/**
 \brief Requests and displays a single employee by ID from the Company object.

 \param comp_in Company CORBA object to query.
 \param seekId Numeric ID of the employee to retrieve.

 \note Handles and logs CORBA exceptions, including EmployeeNotFound.
 */
inline void GetEmployee(Organization::Company_ptr comp_in, CORBA::Long seekId) {
   static const constinit std::string strScope = "GetEmployee()"s;
   log_trace<2>("[{} {}] Requesting employee with ID: {}", strScope, getTimeStamp(comp_in), seekId);
   //Organization::Employee_var employee_var = Organization::Employee::_nil(); // outside declared
   try {
      log_trace<3>("[{} {}]  Entering scope for Employee_var (ID: {}) ...", strScope, getTimeStamp(comp_in), seekId);
      auto employee_var = make_destroyable(comp_in->getEmployee(seekId));
      ShowEmployee(std::cout, employee_var.in());
      log_trace<3>("[{} {}] Leaving scope for Employee_var (ID: {}), Reference released.", strScope, getTimeStamp(comp_in), seekId);
      }
   catch (Organization::EmployeeNotFound const& ex) {
      log_error("[{} {}] ERROR: Caught EmployeeNotFound for ID {}", strScope, getTimeStamp(comp_in), ex.requestedId);
      }
   catch (CORBA::Exception const& ex) {
      log_error("[{} {}] ERROR: CORBA Exception during getEmployee({}): {}", strScope, getTimeStamp(comp_in), seekId, toString(ex));
      }
   catch (std::exception const& ex) {
      log_error("[{} {}] ERROR: C++ Exception during getEmployee({}): {}", strScope, getTimeStamp(comp_in), seekId, ex.what());
      }
   catch(...) {
      log_error("[{} {}] CRITICAL ERROR: unexpected Exception during getEmployee({})", strScope, getTimeStamp(comp_in), seekId);
      }

   // HIER zerstören wir employee_var, so dass die Referenz auf dem Server freigegeben wird.
   // Server zerstört den transienten Servant.
   //if (!CORBA::is_nil(employee_var.in())) {
   //   employee_var->destroy();
   //   employee_var = Organization::Employee::_nil();
   //   }
   }

/**
 \brief Retrieves and prints the list of all employees from the company.

 \param comp_in Company CORBA object providing the employee sequence.

 \note The function cleans up the returned references by calling \c destroy() on each.
 */
inline void GetEmployees(Organization::Company_ptr comp_in) {
   static const constinit std::string strScope = "GetEmployees()"s;
   log_trace<2>("[{} {}] Requesting employees.", strScope, getTimeStamp(comp_in));

   Organization::EmployeeSeq_var employees_seq = comp_in->getEmployees();
   std::println(std::cout, "[{} {}] Received sequence with {} employee references.", 
          strScope, getTimeStamp(comp_in), employees_seq->length());

   for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
      Organization::Employee_ptr current_employee = employees_seq[i];
      if (CORBA::is_nil(current_employee)) {
         log_error("[{} {}] WARNING: Nil reference encountered in sequence at index {}", 
                  strScope, getTimeStamp(comp_in), i);
         continue;
         }

      try {
         std::print(std::cout, "  Employee [{:>3}] - ", i);
         ShowEmployee(std::cout, current_employee);

         }
      catch (CORBA::Exception const& ex) {
         log_error("[{} {}] Error accessing employee data at index {}: {}", 
                        strScope, getTimeStamp(comp_in), i, toString(ex));
         }
      }

   std::println(std::cout, "[Client] End of scope for EmployeeSeq_var. Releasing references in sequence...");

   for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
      if (!CORBA::is_nil(employees_seq[i].in())) {
         employees_seq[i]->destroy();
         employees_seq[i] = Organization::Employee::_nil();
         }
      }
   }
