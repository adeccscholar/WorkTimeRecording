// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Data structures used as temporary backend for CORBA Employee implementation
 
  \details  This file defines the `PersonData` and `EmployeeData` structures, which are
            used as initial placeholder types to simulate backend data for the CORBA
            `Organization::Employee` interface.
 
  \details These structures serve as in-memory representations of personnel and employee
           information and will later be replaced by a persistent storage solution, such
           as a relational database system (e.g., Microsoft SQL Server via ODBC).
 
  \version 1.0
  \date    12.05.2025
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
 
  \note This software is part of the adecc Scholar project – Free educational materials for modern C++.
 */

#include "BasicsC.h"
#include "OrganizationC.h"

#include <Tools.h>
#include <BasicUtils.h>
#include <CorbaUtils.h>

#include <tao/ORB_Core.h>

#include <string>

using namespace std::string_literals;

/**
  \brief Basic person information used for identification and naming.
 
  \details This structure holds minimal person-related attributes and is intended as
           a base for other types such as EmployeeData.
 */
struct PersonData {
   CORBA::Long personID   = -1;                        ///< Unique identifier for the person
   std::string firstname  = ""s;                       ///< First name of the person
   std::string name       = ""s;                       ///< Last name of the person
   Organization::EGender gender = Organization::OTHER; ///< Gender as defined in the IDL enum
   };

/**
  \brief Extended person structure used for simulating employee records.
 
  \details In addition to the basic personal information, this structure includes
           employment-specific attributes such as salary, employment start date,
           and active status.
 */
struct EmployeeData : public PersonData {
   double salary                         = 0.0;          ///< Current salary (in company currency unit)
   std::chrono::year_month_day startDate = { };          ///< Start date in Year-Month-Day format
   CORBA::Boolean isActive               = false;        ///< Employment status (active/inactive)
   };

/**
 \brief copy an EmployeeData Element to a CORBA Organization::EmployeeData_ptr
 \details intern used function to convert / copy a element of the type EmployeeData to
 an element of Organization::EmployeeData and return it to the CORBA middleware as Pointer.
 \param data const reference to an element of EmployeeData
 \returns Pointer to an Organization::EmployeeData structure from the idl
*/
inline Organization::EmployeeData* createFrom(EmployeeData const& data) {
   Organization::EmployeeData* employee_data = new Organization::EmployeeData();
   employee_data->personId  = data.personID;
   employee_data->firstName = CORBA::string_dup(data.firstname.c_str());
   employee_data->name      = CORBA::string_dup(data.name.c_str());
   employee_data->gender    = data.gender;  
   employee_data->isActive  = data.isActive;
   employee_data->startDate = convert<Basics::Date>(data.startDate);
   employee_data->salary    = data.salary;
   return employee_data;
   }
