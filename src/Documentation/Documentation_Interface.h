// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Doxygen Documentation Page – Organization IDL Interface
  
  \details
  Contains documentation page idl_organization 
 */

/**
 \page idl_organization Organization Module – IDL Specification
 \brief CORBA IDL definition for the Organization namespace (Employee/Company Example)
  
 \section idl_organization_intro Introduction
  
 \details This IDL definition specifies the CORBA interfaces for the `Organization` module
          used in the **adecc Scholar** time-tracking example application.
  
 \details The design is intentionally simplified for educational purposes:
           - The interfaces expose only **read-only attributes**
           - No inheritance is used between `Employee` and `Person`
           - The `Employee` interface does not modify state (read access only)
  
 \details These simplifications help learners focus on essential CORBA concepts like:
           - Defining attributes and methods
           - Using sequences
           - Raising custom exceptions
           - Working with object references vs. direct data structs
 
 \details The module includes:
           - An enumeration for gender values
           - Two basic structs for date/time representations
           - A custom exception with diagnostic data
           - A struct for transmitting employee data directly
           - A read-only `Employee` interface (with `destroy()` support)
           - A `Company` interface providing access to employee references and data
  
  \note The example omits CORBA inheritance (e.g., `Employee : Person`) and setter methods to keep the IDL concise.
  
  \section idl_organization_code Full IDL Listing
  \code{.idl}
   module Organization {
      // --------------------
      //    Additional Types
      // --------------------
      enum EGender {
         MALE,
         FEMALE,
         OTHER
         };

      struct TimePoint {
         long long milliseconds_since_epoch;  // Unix timestamp in milliseconds
         };

      struct YearMonthDay {
         long  year;
         short month;
         short day;
         };

      // --------------------------
      //  User-defined Exception
      // --------------------------
      exception EmployeeNotFound {
         long      requestedId;     // ID of the missing employee
         TimePoint requestedAt;     // Timestamp of the failed lookup
         };

      // ---------------------------------------
      //  Struct with values for direct read
      // ---------------------------------------
      struct EmployeeData {
         long          personId;
         string        firstName;
         string        name;
         EGender       gender;
         double        salary;
         YearMonthDay  startDate;
         boolean       isActive;
         };

      // -------------------------
      //     Interface Employee
      // -------------------------
      interface Employee {
         // Read-only attributes for simplicity
         readonly attribute long         personId;
         readonly attribute string       firstName;
         readonly attribute string       name;
         readonly attribute EGender      gender;

         readonly attribute double       salary;
         readonly attribute YearMonthDay startDate;
         readonly attribute boolean      isActive;

         // Also available via attribute combination
         string getFullName();

         // Required to clean up the server-side object (transient servant)
         void destroy();
         };

      // ---------------------------------
      //    Sequence Type for Employees
      // ---------------------------------
      typedef sequence<Employee> EmployeeSeq;

      // -------------------------------------------
      //     Main CORBA Service Interface
      // -------------------------------------------
      interface Company {
         readonly attribute string nameCompany;

         // Server timestamp for diagnostic / sync
         TimePoint getTimeStamp();

         // List of all employees (transient objects)
         EmployeeSeq getEmployees();
         EmployeeSeq getActiveEmployees();

         // Retrieve specific employee by ID (as CORBA object)
         Employee getEmployee(in long personId) raises (EmployeeNotFound);

         // Sum of salaries
         double getSumSalary();

         // Retrieve snapshot data instead of object reference
         EmployeeData getEmployeeData(in long personId) raises (EmployeeNotFound);
         };
      };
  \endcode
  
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \version 1.0
  \date 2025-06-01
  \copyright Copyright © 2020–2025 adecc Systemhaus GmbH  
             This program is free software: you can redistribute it and/or modify it  
             under the terms of the GNU General Public License, version 3.  
             See <https://www.gnu.org/licenses/>.
 
  \note This documentation is part of the adecc Scholar project —  
        Free educational materials for distributed systems in modern C++.
 */
