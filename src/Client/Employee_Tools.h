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

inline std::string toString(Basics::TimePoint tp) {
   return getTimeStamp(convertTo(tp));
   }

inline std::string getTimeStamp(Organization::Company_ptr comp_in) {
   if (!CORBA::is_nil(comp_in)) [[likely]] {
      return getTimeStamp(convertTo(comp_in->getTimeStamp()));
      }
   else throw std::runtime_error("company is nil");
   }

inline void ShowEmployee(std::ostream& out, Organization::Employee_ptr employee) {
   if (!CORBA::is_nil(employee)) {
      CORBA::String_var fullName = employee->getFullName();
      CORBA::Boolean active = employee->isActive();
      std::println(out, "ID: {:>4}, Name: {:<25}, Status: {:<3}, Salary: {:>10.2f}", employee->personId(),
                         fullName.in(), (active ? "Yes" : "No"), employee->salary());
      }
   else {
      throw std::runtime_error(std::format("[ShowEmployee: {}] ERROR: employee is nil unexpectedly.", ::getTimeStamp()));
      }
   }

inline void GetEmployee(Organization::Company_ptr comp_in, CORBA::Long seekId) {
   static const constinit std::string strScope = "GetEmployee()"s;
   log_trace<2>("[{} {}] Requesting employee with ID: {}", strScope, getTimeStamp(comp_in), seekId);
   Organization::Employee_var employee_var = Organization::Employee::_nil(); // outside declared
   try {
      log_trace<3>("[{} {}]  Entering scope for Employee_var (ID: {}) ...", strScope, getTimeStamp(comp_in), seekId);
      employee_var = comp_in->getEmployee(seekId);
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

   // HIER zerstören wir employee_var, so dass die Referenz auf dem Server freigegeben wird.
   // Server zerstört den transienten Servant.
   if (!CORBA::is_nil(employee_var.in())) {
      employee_var->destroy();
      employee_var = Organization::Employee::_nil();
      }
   }

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
