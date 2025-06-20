// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Implementation of the CORBA servant classes for Organization::Person and Organization::Employee

  \details This source file implements the CORBA servant classes `Person_i` and `Employee_i`,
           which correspond to the IDL interfaces `Organization::Person` and `Organization::Employee`.

  \details The class `Employee_i` inherits from `Person_i`, reflecting the inheritance in the IDL.
           `Person_i` provides the core personal attributes and operations such as name and gender,
           while `Employee_i` adds employment-specific extensions like salary, start date, and status.

  \details The servant classes are bound to the Portable Object Adapter (POA) and use memory-local
           structs (`PersonData`, `EmployeeData`) to return results. These are mock representations
           for initial development and testing.

  \details Key responsibilities include:
   - Handling object lifecycle and POA integration
   - Providing CORBA-conformant access to entity data (including duplication of strings)
   - Supporting future extensibility for persistence backends
   - Logging servant creation/destruction with structured trace output

  \note The implementation uses `CORBA::string_dup` for client-safe string return and follows TAO's
        memory conventions. All IDL-derived types are implemented in accordance with the servant pattern.

  \see Employee_i.h
  \see Organization::Person (IDL)
  \see Organization::Employee (IDL)

  \author Volker Hillmann (adecc Systemhaus GmbH)
  \date   12.05.2025
  \version 1.0

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

#include "Employee_i.h"

#include "Tools.h"
#include "my_logging.h"

#include <iostream>
#include <print>
#include <stdexcept>

/**
  \brief Constructs a `Person_i` servant with basic identity data.
 
  \details Initializes a CORBA servant representing a person. The constructor receives
           basic information such as ID, first name, last name, and gender via a `PersonData` structure.
           The POA pointer is passed to the base class `DestroyableInterface_i` to bind the servant correctly
           to the CORBA lifecycle.
 
  \param person A `PersonData` struct containing the person's identifier and name attributes.
  \param poa A pointer to the `PortableServer::POA` instance managing the servant.
 
  \note The constructor logs creation with trace level 4 to aid in tracing and debugging servant lifecycles.
*/
Person_i::Person_i(PersonData const& person, PortableServer::POA_ptr poa) : DestroyableInterface_i(poa) {
   personID_  = person.personID;
   firstname_ = person.firstname;
   name_      = person.name;
   gender_    = person.gender;
   log_trace<4>("[Person_i {}] Object created for ID: {}", ::getTimeStamp(), personId());
   }

/**
  \brief Destructor with diagnostic logging.
 
  \details The destructor logs when the `Person_i` servant is being destroyed.
           This is particularly useful in development and testing to ensure that CORBA
           servants are properly cleaned up and no memory leaks remain.
*/
Person_i::~Person_i() {
   log_trace<4>("[Person_i {}] Object destroyed for ID: {}", ::getTimeStamp(), personId());
   }

/// \brief Returns unique identifier of the person.
CORBA::Long Person_i::personId() {
   return personID_;
   }

/// \brief Returns CORBA-duplicated first name.
///
/// \details The returned string is allocated using \c CORBA::string_dup and must be handled
///          according to CORBA memory management rules. The caller (client stub) is responsible
///          for freeing the returned string when appropriate, typically via CORBA::_var types.
///
/// \return A newly allocated C-style string containing the first name.
///
/// \note Clients should use \c CORBA::String_var or ensure proper deallocation to prevent leaks.
char* Person_i::firstName() {
   return CORBA::string_dup(firstname_.c_str());
   }

/// \brief Returns CORBA-duplicated name.
///
/// \details The returned string is allocated using \c CORBA::string_dup and must be handled
///          according to CORBA memory management rules. The caller (client stub) is responsible
///          for freeing the returned string when appropriate, typically via CORBA::_var types.
///
/// \return A newly allocated C-style string containing the name.
///
/// \note Clients should use \c CORBA::String_var or ensure proper deallocation to prevent leaks.
char* Person_i::name() {
   return CORBA::string_dup(name_.c_str());
}

/// \brief Returns gender of the person (enumeration from IDL).
///
/// \details This value is stored as an `Organization::EGender` enum as defined in the IDL.
/// It is initialized during servant construction and remains constant throughout the lifetime.
///
/// \return The enumerated gender value of this person (e.g., `MALE`, `FEMALE`, `DIVERSE`).
Organization::EGender Person_i::gender() {
   return gender_;
}

/**
  \brief Returns full name (first + last) as single CORBA string.
 
  \details Combines `firstname_` and `name_` into a single string with a separating space.
           Memory is allocated using `CORBA::string_dup`, following CORBA conventions for 
           client/server interaction.
 
  \return A newly allocated `char*` string containing the full name of the person.
 
  \note Clients must manage the returned memory correctly (typically using `CORBA::String_var`)
        to avoid leaks. Do not manually free the result with `delete` or `free`.
 */
char* Person_i::getFullName() {
   std::string strName = std::string{ firstname_ } + " "s + name_;
   return CORBA::string_dup(strName.c_str());
   }


// -------------------------------------------------------------------------------------

/**
  \brief Constructs an Employee_i servant from given EmployeeData.
 
  \details This constructor initializes all employee attributes from the provided data structure,
           including salary, employment start date, and active status. It also initializes the
           base class `Person_i` with the common person-related data.
 
  \param data Struct with full employee information.
  \param poa Pointer to the POA managing this servant instance.
 
  \note The servant is intended for use within the Portable Object Adapter (POA) framework.
        Logging will indicate object lifecycle events for tracing/debugging.
 */
Employee_i::Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa) : 
                     Person_i(static_cast<PersonData const&>(data), poa), DestroyableInterface_i(poa) {
   salary_    = data.salary;
   startDate_ = convertTo(data.startDate); 
   isActive_  = data.isActive;
   log_trace<4>("[Employee_i {}] Object created for id: {}", ::getTimeStamp(), personId());
   }

/**
   \brief Destructor with diagnostic output.
 
   \details Called automatically when the servant is destroyed by the POA.
            Emits a log message indicating object removal for visibility and 
            resource tracking.
 */
Employee_i::~Employee_i() {
   log_trace<4>("[Employee_i {}] Object destroyed for id: {}", ::getTimeStamp(), personId());
   }

/// \brief Returns current salary of the employee.
///
/// \return The numeric salary value stored in the servant.
CORBA::Double Employee_i::salary() {
   return salary_;
   }

 /// \brief Returns the employment start date.
///
/// \return A `Basics::YearMonthDay` object representing the starting date of employment.
Basics::YearMonthDay Employee_i::startDate() {
   return startDate_;
   }

/// \brief Returns whether the employee is currently active.
///
/// \return `true` if the employee is marked as active; otherwise `false`.
CORBA::Boolean Employee_i::isActive() {
   return isActive_;
   }





