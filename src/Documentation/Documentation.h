/**
  \file
  \brief Documentation file for the main page of project documentation
  \details This file contain the main page and additional informations for the project.
  \copyright copyright © 2025 adecc Systemhaus GmbH
*/

/**
  @mainpage adecc Scholar: Distributed Enterprise Time-Tracking System

  @section intro_sec Introduction

  This open-source project is being developed as part of the free educational offer **adecc Scholar**, which started in the fall of 2020 during the COVID-19 pandemic with
  free streaming on the [adecc Scholar Twitch channel](https://www.twitch.tv/volker_adecc).
  The goal of this project is to teach the development of modern distributed systems in **C++23** through a hands-on example - an enterprise time-tracking system.

  @section main_goals_sec Main Objectives
  - Teaching modern C++ techniques (including **C++23**) and frameworks
  - Designing and implementing a distributed system across various computing platforms (servers, desktops, Raspberry Pi, mobile devices)
  - Access to databases using SQL
  - Integration of sensor and device control (GPIO, I2C, SPI)
  - Discussion of project management, database design, and normalization
  - Demonstrating cross-platform development (Windows Server, Ubuntu Server, Windows/Linux Desktops, Embedded)

  @section architecture_sec Architecture

  1. **Application Server**
     - Operating System: **Ubuntu Linux**
     - CORBA/TAO as middleware for distributed objects
     - ODBC access to MS SQL Server
     - Central implementation of business logic

  2. **Database Server**
     - Operating System: **Windows Server**
     - Microsoft SQL Server
     - Relational modeling and normalization

  3. **Workstation Clients**
     - Windows and Linux desktops
     - CORBA/TAO as middleware
     - Qt6-based GUI for time-tracking and reporting

  4. **Embedded Clients (Raspberry Pi)**
     - CORBA/TAO (client and server)
     - Access to GPIO pins for discrete signals
     - Use of I2C/SPI for sensors, displays, and RFID readers
     - Standalone roles as client and (local) server
     - RFID-based access control using finite state machines (Boost.SML)

  5. **Future Extensions**
     - Web-REST server for integrating mobile devices
     - Discussion of architectural questions, opportunities, and risks

  @section tech_sec Technologies and Libraries Used

  - **C++23**: Latest language features for efficiency and platform independence
  - **Visual Studio C++**: Development environment for streams and debugging
     - VS C++ is modern and offers cross-compiling capabilities
     - Available for free in the Community Edition
  - **GCC14** via cross-compiling on Linux platforms, including Raspberry Pi
  - **CMake** build processing
     - Integration of custom routines, including CORBA workflow
  - **Boost**
     - **Boost.SML** for finite state machines
     - **Boost.JSON** for JSON processing (in future extensions)
     - **Boost.Beast** for alternative HTTP bindings (in future extensions)
  - **CORBA/TAO**: Latest TAO version as middleware
     - Working with ORB and POA, core technologies, transient and non-transient servlets
     - CORBA Naming Services
     - CORBA Event Service, later TAO Events for direct communication
  - **Qt 6**: Database access (Qt SQL) and GUI development
     - QDatabase and QWidgets
     - Free version available for open-source projects, with options to download and compile independently
  - **Embarcadero C++ Builder**
     - Integration of older tools (VCL, FMX)
     - Bridge between legacy code and modern systems
  - **MS SQL Server**: Relational database
     - Free Express version or Development Edition for developers

  @section builder_sec Embarcadero C++ Builder

  With support for **Embarcadero C++ Builder** (VCL and FMX), this project demonstrates how even older or outdated tools can be integrated into a modern system architecture.
  This allows for the connection of existing legacy applications and the gradual migration to modern systems.

  @section getting_started_sec Getting Started

  1. Clone the repository:
     ```bash
     git clone https://github.com/adeccscholar/WorkTimeRecording.git
     ```

  2. Open the C++ server components with Visual Studio and deploy to Ubuntu Server
  3. Compile and run the Qt clients
  4. Flash the Raspberry Pi images and configure them

  @section contribute_sec Contributing

  - Fork the repository
  - Create feature branches (e.g., `feat/my-feature`)
  - Submit pull requests with detailed descriptions
  - Use issues for bugs and improvement suggestions

  @section license_sec License and Copyright

  ```text
  Copyright © adecc Systemhaus GmbH 2020–2025

  This repository is mostly under the GNU General Public License v3.0 (GPL-3.0).
  Different open-source licenses may apply in sub-projects.
  ```
*/

/**
  \dir Src
  \brief Source directory containing all source code files for all projects.
 
  \details This directory holds all C++ source files, including the implementation of core 
           functionality, services, and utility classes that power the system.
 */

 /**
  \dir Src/IDL
  \brief Contains the IDL (Interface Definition Language) files.
  
  \details This directory includes all IDL files used for defining interfaces between distributed 
           components in the system. The IDL files are crucial for defining CORBA objects and the 
           interfaces exposed to clients and servers.
  \details All corba server and clients needing the directory and idl files to generate the
           skeletons and stubs for the application.
 */

/**
  \dir Src/Tools
  \brief Contains common Header files 

  \details This directory includes all header files with common tools for projects in the 
           distributed time tracking system. This are header who used to map the corba types to
		   C++ and common used files for the programs.
*/

/**
  \dir Src/ApplicationServer
  \brief Contains the Source files for the Implementation of the main server for the time tracking

  \details This directory includes all source files for the implementation of the corba server
*/