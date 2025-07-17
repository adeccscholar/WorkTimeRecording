// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief CORBA servant interface declarations for Organization::Person and Organization::Employee

\details This header declares the servant classes `Person_i` and `Employee_i`, which implement
         the CORBA interfaces `Organization::Person` and `Organization::Employee`, respectively.

\details These classes serve as POA - based implementations and are designed for early stages
         of development.Instead of a persistent backend, both classes currently use
         simplified in - memory structures(`PersonData`, `EmployeeData`) to represent
         the underlying state.

   \details Key features :
             - Lifecycle - managed servants via `DestroyableInterface_i`
             - CORBA - compliant return types using `CORBA::string_dup`
             - Trace - logging of object construction and destruction
             - Safe access to attributes such as ID, name, gender, salary, etc.

\note Clients must take ownership of CORBA - allocated strings returned by these servants.
      Memory handling is based on CORBA duplication semantics.

\author Volker Hillmann(adecc Systemhaus GmbH)
\date    12.05.2025
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

\see Employee_i.cpp
\see DestroyableInterface_i
\see Organization::Person(IDL)
\see Organization::Employee(IDL)

\note This Software is part of the adecc Scholar project – Free educational materials for modern C++.
*/

#pragma once

#include "OrganizationS.h" // Skeleton Header
#include "Basics_i.h"
#include "EmployeeData.h"

#include <tao/ORB_Core.h>
#include <tao/PortableServer/PortableServer.h>

   /**
     \brief CORBA servant implementation for the IDL interface `Organization::Person`.
    
     \details This class provides a concrete CORBA servant for the base `Person` interface,
              as defined in the Organization IDL. It inherits from `DestroyableInterface_i`
              to support explicit lifecycle management, and from `POA_Organization::Person`
              to fulfill the CORBA skeleton interface requirements.
    
     \details The class encapsulates common person attributes such as:
       - Person ID (`personId`)
       - First and last name
       - Gender
    
     \details All string-based return values are safely duplicated with `CORBA::string_dup`
              to follow CORBA memory conventions and avoid dangling pointers on the client side.
    
     \note The `Person_i` class is a base class of `Employee_i` and designed for extension.
           It is intended for use in early prototyping and testing.
    
     \see Organization::Person (IDL)
     \see Employee_i
    */
class Person_i : public virtual DestroyableInterface_i,
                 public virtual POA_Organization::Person {
private:
   CORBA::Long personID_         =  -1;                 ///< Unique identifier for the person
   std::string firstname_        = ""s;                 ///< First name of the person
   std::string name_             = ""s;                 ///< Last name of the person
   Organization::EGender gender_ = Organization::OTHER; ///< Gender as defined in the IDL enum
public:
   Person_i() = delete;
   Person_i(PersonData const& person, PortableServer::POA_ptr poa);
   virtual ~Person_i();

   /**
      \name IDL Attribute Methods
      \details These methods implement the read-only CORBA attributes defined in the IDL.
               They are required by the CORBA interface and should not be used directly
               in modern C++ code. Prefer the provided `std::string` helper methods instead.
    */
    /// \{

   virtual CORBA::Long personId() override;

   virtual char* firstName() override;
   virtual char* name() override;
   virtual Organization::EGender gender() override;

   virtual char* getFullName() override;
   /// \}

   };


/**
  \brief   Concrete servant class implementing the CORBA interface `Organization::Employee`.
  \details This class maps an `EmployeeData` structure to the CORBA `Employee` interface.
           It supports access to basic employee attributes and is bound to a POA (Portable Object Adapter).

  \note   CORBA string-returning methods (e.g., `firstName()`) return `char*` allocated
          with `CORBA::string_dup()`; callers are responsible for freeing memory using
          `CORBA::string_free()` or wrapping the result in `CORBA::String_var`.
  \note   The data source (`EmployeeData`) is temporary and will later be replaced with
          a system-backed implementation (e.g., connected to a database).

 */
class Employee_i : public virtual POA_Organization::Employee,
                   public virtual Person_i {

public:
   Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa);
   virtual ~Employee_i();

   // =========================================================================
   /**
      \name IDL Attribute Methods
      \details These methods implement the read-only CORBA attributes defined in the IDL.
               They are required by the CORBA interface and should not be used directly
               in modern C++ code. Prefer the provided `std::string` helper methods instead.
    */
   /// \{

   virtual CORBA::Double salary() override;
   virtual Basics::Date startDate() override;
   virtual CORBA::Boolean isActive() override;

   /// \}

private:
   double         salary_ = 0.0;              ///< Current salary (in company currency unit)
   Basics::Date   startDate_ = { 0, 0, 0 };  ///< Start date in Year-Month-Day format
   CORBA::Boolean isActive_ = false;         ///< Employment status (active/inactive)
};
