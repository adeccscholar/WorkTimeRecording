// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Doxygen Documentation Page for POA Architecture
 
  \details
  Contains the Documentation page \ref poa_usage "Portable Object Adapter (POA) Usage in CORBA Project"
 */

/**
  \page poa_usage Portable Object Adapter (POA) Usage in CORBA Project
  \brief Explanation of how POAs are structured and used in the application.
 
  \section poa_intro Introduction
  \details The Portable Object Adapter (POA) is a fundamental part of the CORBA server architecture.
           It is responsible for the lifecycle, object activation, and request dispatching of CORBA servant objects.
           In this project, multiple POAs are used to separate concerns and optimize servant management:
 
           - A **persistent POA** for long-lived objects like the `Company` servant.
           - A **transient POA** for short-lived or per-request servants like `Employee`.
 
  This separation allows clean shutdown, controlled memory management, and support for complex interaction scenarios.
 
  \section poa_config POA Configuration Overview
 
  \details POAs are configured during server startup using different policy sets:
 
  - `LifespanPolicy::PERSISTENT` is used for `CompanyPOA`, allowing the company object to be consistently resolvable via the Naming Service.
  - `LifespanPolicy::TRANSIENT` and `ServantRetentionPolicy::RETAIN` are used for `EmployeePOA`, which dynamically creates and destroys employee servants.
 
  \details Example creation:
  \code{.cpp}
  CORBA::PolicyList empl_pol;
  empl_pol.length(2);
  empl_pol[0] = root_poa->create_lifespan_policy(PortableServer::TRANSIENT);
  empl_pol[1] = root_poa->create_servant_retention_policy(PortableServer::RETAIN);
 
  PortableServer::POA_var employee_poa = root_poa->create_POA("EmployeePOA", poa_manager.in(), empl_pol);
  \endcode
 
  \section poa_assignment Deferred POA Assignment
 
  \details The `Company_i` servant uses deferred POA assignment for employees via the method:
           \ref Company_i::set_employee_poa
 
  This allows constructing the company object with minimal configuration and injecting the employee POA later
  after proper initialization. Attempting to set the POA more than once throws an exception to ensure correctness.
 
  \section poa_notes Notes
  - Each servant is explicitly activated via the assigned POA.
  - Transient servants like `Employee_i` should implement `destroy()` to cleanly deactivate themselves from the POA.
  - It is critical to duplicate POA pointers (`_duplicate`) before storing them in member variables.
  - Correct use of `PortableServer::ObjectId` and POA deactivation ensures that resources are reclaimed safely.
 
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \version 1.0
  \date 2025-06-11
  \copyright
  Copyright © 2020–2025 adecc Systemhaus GmbH  
  This program is free software: you can redistribute it and/or modify it  
  under the terms of the GNU General Public License, version 3.  
  See <https://www.gnu.org/licenses/>.
 
  \note Part of the adecc Scholar project – educational resources for distributed C++ systems.
 */