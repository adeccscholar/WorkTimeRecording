// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Doxygen Documentation Page for the CORBA Application Server

 \details
 \subpage appserver "CORBA Application Server – Overview"
 */
 
/**
 \page appserver CORBA Application Server – Overview

 \section appserver_intro Introduction

 The CORBA Application Server is a core component of the distributed enterprise time-tracking system based on CORBA
 developed as part of the **adecc Scholar** educational initiative. It runs on **Ubuntu Linux** and serves
 as a central coordination and data hub in a heterogeneous CORBA-based network.
 
 It manages central components such as companies and provides servants to interact with external clients like time 
 terminals and administration tools.

 Key features:
  - Company servant accessible via CORBA Naming Service
  - Persistent and transient POA separation
  - Multi-threaded ORB event loop
  - Clean shutdown and resource handling

 See \ref app_lifecycle for a detailed flow of initialization and shutdown.

 \section appserver_arch Architecture and Function

 This Application Server fulfills multiple roles:

 - Acts as a **CORBA server** for GUI clients on Windows and Linux desktop systems  
 - Provides employee and time-tracking data by connecting to a Microsoft SQL Server  
 - Uses **ODBC** for database access via the **Qt6**-based wrapper `adecc_Database`
 - Exposes business logic (e.g. salary sums, active employees) through the CORBA interface `Organization::Company`
 - Acts as a **CORBA client** to communicate with embedded devices (e.g. Raspberry Pi-based terminals)
 - Controls and monitors terminals by activating/deactivating access control services remotely

 \section appserver_middleware Middleware and POA Configuration

 - Uses **TAO (The ACE ORB)** for CORBA communication
 - Relies on both **persistent** and **transient POAs**:
   - Persistent POA for long-lived objects like Company or Organization services
   - Transient POA for dynamically activated objects like temporary Employee servants
 - Integrates with the **CORBA Naming Service** for object discovery

 \section appserver_goals Educational Purpose

 The application server illustrates how to:
 - Build a robust, distributed backend system using **modern C++23**
 - Integrate platform-independent technologies (Linux, Windows, Embedded)
 - Use CORBA for distributed object communication and lifetime control
 - Interface with SQL databases in an enterprise environment
 - Bridge modern and legacy infrastructure components

 \section appserver_part Project Context

 This server is part of the open-source adecc Scholar project.
 It is designed to teach modern, real-world distributed application development using
 C++, CORBA/TAO, Qt, and SQL in a heterogeneous network of clients and servers.

 \author Volker Hillmann (adecc Systemhaus GmbH)
 \version 1.0
 \date 2025-05-14
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.
*/

/**
  \page app_lifecycle Application Lifecycle
  \@brief Detailed breakdown of the server application's runtime behavior.
 
  This document describes the control flow of the server, including CORBA setup, POA configuration,
  servant creation, and shutdown handling.
 
  ### 1. Signal Handling
  - Registers signal handlers for graceful termination (SIGINT/SIGTERM).
 
  ### 2. ORB Initialization
  - Initializes the global ORB using command line arguments.
  - Handles and logs errors if the ORB is not created.
 
  ### 3. Root POA Acquisition
  - Resolves "RootPOA" reference and narrows it.
  - Activates its POA manager to allow request processing.
 
  ### 4. POA Policies
  - Defines a persistent POA (`CompanyPOA`) for long-lived servants.
  - Defines a transient POA (`EmployeePOA`) for short-lived objects.
 
  ### 5. Child POA Creation
  - Creates `CompanyPOA` and `EmployeePOA` as children of the root POA.
 
  ### 6. Company Servant Activation
  - Instantiates and activates a servant implementing the `Company` interface.
  - Obtains the object reference for use in the Naming Service.
 
  ### 7. Naming Service Registration
  - Registers the `Company` servant with the CORBA Naming Service under a known name.
 
  ### 8. ORB Event Loop
  - Starts the `orb->run()` loop in a background thread to handle client requests.
 
  ### 9. Shutdown Wait
  - Main thread blocks and periodically checks for shutdown signal.
 
  ### 10. Naming Service Unbind
  - Attempts to remove the object reference from the Naming Service.

  ### 11. Deactivate Company Servant and count ref down to deletr
  - Delete the servant object finally.
  
  ### 12. ORB Shutdown
  - Calls `shutdown(false)` to unblock the ORB thread.
 
  ### 13. POA Destruction
  - Destroys both child POAs and the RootPOA in order.
 
  ### 14. ORB Destruction
  - Finally, the ORB is destroyed to free all resources.
 */
