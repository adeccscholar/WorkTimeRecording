﻿// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief CORBA servant implementation header for Organization::Company interface.
 
  \details This file declares the `Company_i` class, which implements the CORBA interface `Organization::Company`.
           It provides functionality to access company data and manage employee objects.
           Employees are represented using the placeholder type `EmployeeData` and stored in an in-memory map.
           Each employee object is instantiated as a separate CORBA servant via the `Employee_i` implementation.
 
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

#pragma once

#include "OrganizationS.h" // Skeleton Header

#include "Tools.h"

#include "Employee_i.h"

#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <format>
#include <print>

using namespace std::string_literals;

/// In-memory representation of employee data using a map indexed by employee ID.
using employee_test_data_ty = std::map<CORBA::Long, EmployeeData>;

/**
  \brief CORBA servant implementation for Organization::Company.
 
  \details This class implements the Organization::Company CORBA interface. It provides methods
           for accessing company information and managing employee records. It also creates
           and activates CORBA servants for each employee.
  
   \note   The map with the Long key and the data source (`EmployeeData`) is temporary and 
           simulate a database. This will later be replaced with a system-backed implementation 
           (e.g., connected to a database).
 */
class Company_i : public virtual POA_Organization::Company {
private:
   const std::string strCompanyName = "Pfefferminza AG"s; ///< name of company for corba interface / implmentation.

   employee_test_data_ty employee_database_;  ///< In-memory employee data (as fast start for tests, later access to database.

   PortableServer::POA_var employee_poa_;     ///< POA responsible for Employee servants
   PortableServer::POA_var company_poa_;      ///< POA responsible for Company servant

public:

   /**
     \brief Constructor for the Company_i class.
     \param company_poa POA used to activate the company servant.
     \param employee_poa POA used to activate employee servants.
    */
   Company_i(PortableServer::POA_ptr company_poa, PortableServer::POA_ptr employee_poa);

   /**
     \brief Destructor for the Company_i servant.
     \details Cleans up the Company servant instance. Since CORBA reference counting
              and POA object lifecycle management are used, this destructor typically does not
              need to release individual CORBA resources manually. If dynamically allocated resources
              or external connections are introduced in the future, cleanup logic should be placed here.
     \details The destructor is only used to log the destruction.
    */
   virtual ~Company_i();

   /**
     \brief Assigns the Portable Object Adapter (POA) used for employee servants.
   
     \details This method sets the `employee_poa_` member to the given `PortableServer::POA_ptr`.
              It is expected to be called only once during the initialization phase after the
              `Company_i` object has been created. The provided POA will later be used to activate
              transient employee servants (e.g., for `getEmployee()` responses).
 
     \param employee_poa Pointer to the POA that will manage employee servant instances.
 
     \throws std::logic_error If this method is called more than once or if the POA has already been set.
 
     \note The POA reference is duplicated using `PortableServer::POA::_duplicate` to properly manage its lifetime.
     \note This setter allows deferred configuration, separating employee POA injection from constructor logic.
   */
   void set_employee_poa(PortableServer::POA_ptr employee_poa) {
      if (!CORBA::is_nil(employee_poa_.in()))
         throw std::logic_error(std::format("[{} {}] Employee POA has already been set.", "Company_i::set_employee_poa", ::getTimeStamp()));
      employee_poa_ = PortableServer::POA::_duplicate(employee_poa);
      }


   /**
     \brief Returns the name of the company.
     \return CORBA string representing the company name.
    */
   virtual char* nameCompany() override;
   
   /**
     \brief Gets the current timestamp of the server.
     \return CORBA::TimePoint structure with current time.
    */
   virtual Basics::TimePoint getTimeStamp() override;

   /**
     \brief Returns all employees as CORBA object references.
     \return A sequence of CORBA Employee object references.
    */
   virtual Organization::EmployeeSeq* getEmployees() override;
  
   /**
     \brief Returns all active employees as CORBA object references.
     \return A sequence of CORBA Employee object references for active employees.
    */
   virtual Organization::EmployeeSeq* getActiveEmployees() override;

   /**
      \brief Returns the CORBA object reference for a given employee ID.
      \param personId The unique ID of the employee.
      \throws Organization::EmployeeNotFound
      \return A CORBA Employee object reference.
     */
   virtual Organization::Employee* getEmployee(CORBA::Long personId);

   /**
     \brief Returns the raw employee data for a given employee ID.
     \param personId The unique ID of the employee.
     \throws Organization::EmployeeNotFound
     \return A pointer to an Organization::EmployeeData structure.
    */
   virtual Organization::EmployeeData* getEmployeeData(CORBA::Long personId);

   /**
     \brief Calculates the total salary of all active employees.
     \return Sum of all active employee salaries.
    */
   virtual double                  getSumSalary() override;

private:
   /**
     \brief Initializes the in-memory employee database with test data.
   */
   void initializeDatabase();

   /**
     \brief Builds a CORBA sequence of Employee object references from a range.
     \tparam range_ty A range of EmployeeData elements.
     \param range Input range from which to build the sequence.
     \return CORBA sequence of Employee object references.
    */
   template <std::ranges::input_range range_ty>
   Organization::EmployeeSeq* buildEmploySequenceFromRange(range_ty &&range) {
      Organization::EmployeeSeq_var employees_seq = new Organization::EmployeeSeq;
      CORBA::Long current_index = 0;

      for(auto const& data : range) {
         try {
            Employee_i* employee_servant = new Employee_i(data, employee_poa_.in());
            PortableServer::ObjectId_var oid = employee_poa_->activate_object(employee_servant);
            CORBA::Object_var obj_ref = employee_poa_->id_to_reference(oid.in());
            Organization::Employee_var employee_ref = Organization::Employee::_narrow(obj_ref.in());

            if(CORBA::is_nil(employee_ref)) {
               std::println(std::cerr, "Company_i {}] error while narrowing employee for id {}", ::getTimeStamp(), employee_servant->personId());
               continue;
               }
            employee_servant->set_oid(oid);
            employees_seq->length(current_index + 1);
            (*employees_seq)[current_index++] = employee_ref._retn();
            }
         catch(CORBA::Exception const& ex) {
            std::println(std::cerr, "[Company_i {}] Corba Exception for Employee {}: {}", ::getTimeStamp(), data.personID, toString(ex));
            }
         catch(std::exception const& ex) {
            std::println(std::cerr, "[Company_i {}] C++ Exception for Employee {}: {}", ::getTimeStamp(), data.personID, ex.what());
            }
         }
      std::println(std::cout, "[Company_i {}] Returnning {} employees references.", ::getTimeStamp(), employees_seq->length());
      return employees_seq._retn();
      }

};

