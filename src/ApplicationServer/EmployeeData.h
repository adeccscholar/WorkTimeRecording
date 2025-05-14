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
             This program is free software: you can redistribute it and/or modify it
             under the terms of the GNU General Public License, version 3.
             See <https://www.gnu.org/licenses/>.
 
  \note This software is part of the adecc Scholar project – Free educational materials for modern C++.
 */

#include "OrganizationC.h"

#include <tao/ORB_Core.h>

#include <string>

using namespace std::string_literals;

/**
  \struct 
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
  \struct 
  \brief Extended person structure used for simulating employee records.
 
  \details In addition to the basic personal information, this structure includes
           employment-specific attributes such as salary, employment start date,
           and active status.
 */
struct EmployeeData {
   CORBA::Long personID                 = -1;                   ///< Unique identifier for the employee
   std::string firstname                = ""s;                  ///< First name
   std::string name                     = ""s;                  ///< Last name
   Organization::EGender gender         = Organization::OTHER;  ///< Gender (as defined in IDL)
   double salary                        = 0.0;                  ///< Current salary (in company currency unit)
   Organization::YearMonthDay startDate = { 0, 0, 0 };          ///< Start date in Year-Month-Day format
   CORBA::Boolean isActive              = false;                ///< Employment status (active/inactive)
   };
