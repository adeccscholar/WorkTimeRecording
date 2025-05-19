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

#include "Tools.h"

#include <iostream>
#include <print>
#include <stdexcept>


Employee_i::Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa) : 
                    data_(data), poa_(PortableServer::POA::_duplicate((poa))) {
   std::println(std::cout, "[Employee_i {}] Object created for id: {}", ::getTimeStamp(), personId());
   }

Employee_i::~Employee_i() {
   std::println(std::cout, "[Employee_i {}] Object destroyed for id: {}", ::getTimeStamp(), personId());
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

 Organization::EGender Employee_i::gender() {
    return data_.gender;
    }

 
 char* Employee_i::getFullName() {
    std::string strName = std::string{ data_.firstname } + " "s + data_.name;
    return CORBA::string_dup(strName.c_str());
    }

 CORBA::Double Employee_i::salary() {
    return data_.salary;
    }

 Organization::YearMonthDay Employee_i::startDate() {
    return data_.startDate;
    }

 CORBA::Boolean Employee_i::isActive() {
    return data_.isActive;
    }

// helper to set oid for the concrete object
void Employee_i::set_oid(PortableServer::ObjectId const& oid) {
   oid_ = oid;
   }

// destroy the concrete object on the server (transient)
void Employee_i::destroy() {
   std::println(std::cout, "[Employee_i {}] destroy() called for ID {}", ::getTimeStamp(), personId());

   try {
      poa_->deactivate_object(oid_);  // Objekt deregistrieren
      }
   catch(PortableServer::POA::ObjectNotActive const& ex) {
      // Is thrown if the OID is not active (i.e. already deactivated). We can ignore this.
      std::println(std::cout, "[Employee_i {}] OID is not active (i.e. already deactivated): {}", ::getTimeStamp(), toString(ex));
      }
   catch (CORBA::Exception const& ex) {
      std::println(std::cerr, "[Employee_i {}] Exception during deactivate_object: {}", ::getTimeStamp(), toString(ex));
      }

   _remove_ref();  // Führt zu delete this bei RefCount 0
   }

