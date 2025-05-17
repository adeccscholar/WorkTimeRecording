// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Implementation of the CORBA skeleton interface Organization::Employee

 \details This file defines the `Employee_i` class, which serves as a CORBA servant
          for the `Organization::Employee` interface. It currently uses the placeholder
          structure `EmployeeData` to simulate employee records. This implementation
          is a preliminary step and will later be replaced by a concrete backend
          using persistent storage (e.g., a database).

 \version 1.0
 \date    12.05.2025
 \author Volker Hillmann (adecc Systemhaus GmbH)
 \copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH  
            This program is free software: you can redistribute it and/or modify it  
            under the terms of the GNU General Public License, version 3.  
            See <https://www.gnu.org/licenses/>.

 \note This Software is part of the adecc Scholar project – Free educational materials for modern C++.
*/
#pragma once

#include "OrganizationS.h" // Skeleton Header

#include "EmployeeData.h"

#include <tao/ORB_Core.h>
#include <tao/PortableServer/PortableServer.h>

/**
  \class
  \brief   Concrete servant class implementing the CORBA interface `Organization::Employee`.
  \details This class maps an `EmployeeData` structure to the CORBA `Employee` interface.
           It supports access to basic employee attributes and is bound to a POA (Portable Object Adapter).

  \note   CORBA string-returning methods (e.g., `firstName()`) return `char*` allocated
          with `CORBA::string_dup()`; callers are responsible for freeing memory using
          `CORBA::string_free()` or wrapping the result in `CORBA::String_var`.
  \note   The data source (`EmployeeData`) is temporary and will later be replaced with
          a system-backed implementation (e.g., connected to a database).

 */
class Employee_i : public virtual PortableServer::RefCountServantBase,
                   public virtual POA_Organization::Employee {

public:
   /**
     \brief Constructs an Employee servant with the given data and POA.
     \param data The employee data used to populate attributes.
     \param poa  The Portable Object Adapter responsible for this servant.
    */
   Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa);

   /**
     \brief Destructor for the Employee_i servant.
     \details Cleans up the Employee_i servant instance. Since CORBA reference counting
              and POA object lifecycle management are used, this destructor typically does not
              need to release individual CORBA resources manually. If dynamically allocated resources
              or external connections are introduced in the future, cleanup logic should be placed here.
     \details The destructor is only used to log the destruction and illustrate the workflow.
    */
   virtual ~Employee_i();

   // =========================================================================
   /**
      \name IDL Attribute Methods
      \details These methods implement the read-only CORBA attributes defined in the IDL.
               They are required by the CORBA interface and should not be used directly
               in modern C++ code. Prefer the provided `std::string` helper methods instead.
    */
   /// \{
   
   virtual CORBA::Long personId() override;

   /**
     \brief Gets the employee's first name (CORBA).
     \return Pointer to a newly allocated CORBA string. Caller must free with `CORBA::string_free()`
             or use `CORBA::String_var`.
   */
   virtual char* firstName() override;
   virtual char* name() override;
   virtual Organization::EGender gender() override;

   // IDL Operation
   virtual char* getFullName() override;

   // IDL Attribute von Employee
   virtual CORBA::Double salary() override;
   virtual Organization::YearMonthDay startDate() override;
   virtual CORBA::Boolean isActive() override;

   /// \}

   void destroy() override;


   // helper to set oid fpr concrete object
   void set_oid(PortableServer::ObjectId const& oid);


private:
   EmployeeData                data_; ///< Holds the employee data associated with this servant instance.
   PortableServer::POA_var      poa_; ///< Reference to the Portable Object Adapter managing this servant.
   PortableServer::ObjectId_var oid_; ///< Object ID assigned to this servant instance.

};
