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

 The CORBA Application Server is a core component of the distributed enterprise time-tracking system
 developed as part of the **adecc Scholar** educational initiative. It runs on **Ubuntu Linux** and serves
 as a central coordination and data hub in a heterogeneous CORBA-based network.

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