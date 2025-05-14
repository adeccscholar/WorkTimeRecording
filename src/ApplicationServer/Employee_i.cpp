// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Implementation of the CORBA servant class for Organization::Employee
 
  \details This source file provides the implementation of the `Employee_i` class, which
           acts as the CORBA servant for the IDL-defined interface `Organization::Employee`.
 
  \details The class is responsible for returning employee-specific information such as
           names and identifiers, based on a simplified in-memory structure `EmployeeData`.
 
  \details In this early project stage, the implementation is a mock representation of
           employee entities and will be replaced in future iterations by a persistent
           backend (e.g., database access via ODBC or Qt SQL).
 
  \details Key responsibilities include:
   - Handling object lifecycle (construction, destruction, logging)
   - Providing access to employee attributes defined in the IDL
   - Duplication of CORBA strings to ensure memory safety in client/server interaction
 
  \details Logging is done using `std::println` to provide feedback upon servant creation
           and destruction. This aids in debugging and understanding object management
           within the POA (Portable Object Adapter) context.
 
  \note  The implementation assumes ownership management of CORBA strings and POA duplication,
         and follows CORBA memory conventions using `CORBA::string_dup`.
 
  \see Employee_i.h
  \see Organization::Employee (IDL)
 
  \version 1.0
  \date    12.05.2025
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \copyright  Copyright © 2020 - 2025 adecc Systemhaus GmbH
              This program is free software: you can redistribute it and/or modify it
              under the terms of the GNU General Public License, version 3.
              See <https://www.gnu.org/licenses/>.

  \note This Software is part of the adecc Scholar project – Free educational materials for modern C++.
 */

#include "Employee_i.h"

#include <iostream>
#include <print>
#include <stdexcept>


Employee_i::Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa) : 
                    data_(data), poa_(PortableServer::POA::_duplicate((poa))) {
   std::println(std::cout, "[Employee_i ] Object created for id: {}", personId());
   }

Employee_i::~Employee_i() {
   std::println(std::cout, "[Employee_i ] Object destroyed for id: {}", personId());
   }

CORBA::Long Employee_i::personId() {
   return data_.personID;
   }

char* Employee_i::firstName() {
   return CORBA::string_dup(data_.firstname.c_str());
   }

char* Employee_i::name() {
   return CORBA::string_dup(data_.name.c_str());
   }

/*


   // IDL Attribute
   virtual Organization::EGender gender() override;

   // IDL Operation
   virtual char* getFullName() override;

   // IDL Attribute von Employee
   virtual CORBA::Double salary() override;
   virtual Organization::YearMonthDay startDate() override;
   virtual CORBA::Boolean isActive() override;

   // helper to set oid fpr concrete object
   void set_oid(PortableServer::ObjectId const& oid);

   void destroy() override;
*/