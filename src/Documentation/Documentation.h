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

  5. **State Machine Weather Server**
     - Implements a robust asynchronous server using a finite state machine architecture (boost::statechart)
     - Handles client requests, processes states for connection, query, and response handling
     - Retrieves and parses live weather data from Open Meteo using HTTP (boost::beast) and JSON processing (boost::json)
     - Extensible for additional APIs or business logic through new state or event definitions
     - Ensures clear separation of concerns: networking, state transitions, and external API interaction

  6. **Future Extensions**
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
  \dir src
  \brief Source directory containing all source code files for all projects.
 
  \details This directory holds all C++ source files, including the implementation of core 
           functionality, services, and utility classes that power the system.
 */

 /**
  \dir src/IDL
  \brief Contains the IDL (Interface Definition Language) files.
  
  \details This directory includes all IDL files used for defining interfaces between distributed 
           components in the system. The IDL files are crucial for defining CORBA objects and the 
           interfaces exposed to clients and servers.
  \details All corba server and clients needing the directory and idl files to generate the
           skeletons and stubs for the application.
 */

/**
  \dir src/Tools
  \brief Contains common Header files 

  \details This directory includes all header files with common tools for projects in the 
           distributed time tracking system. This are header who used to map the corba types to
		   C++ and common used files for the programs.
*/

/**
  \dir src/RaspberryTools
  \details The files in this directoy are part of a header-only library for the **RaspberryTools** component library,
           a modular, header-only C++ collection for accessing and abstracting low-level GPIO, I²C, and SPI hardware
           interfaces on Raspberry Pi and compatible single-board systems.
*/

/**
  \dir src/ApplicationServer
  \brief Contains the Source files for the Implementation of the main server for the time tracking

  \details This directory includes all source files for the implementation of the corba server
*/

/**
  \dir src/Client
  \brief Contains the Source files for the Client Application for enterprise use
  \details This directory contains all source files for the Client application which is used for
           employees to see own data and informations.
  \details The target platform for this application is Windows and Linux.
*/

/**
  \dir src/RaspberryTerminal
  \brief Contains the Source files for the Raspberry PI Terminal Application
  \details This directory contains all source files for the Raspberry PI Application with the RFID
           Reader for the time tracking and with environmental sensors.
*/

/**
  \dir src/Documentation
  \brief Contains the header files with the documentation

  \details This directory includes only header files for the documentation of the enterprise 
           time tracking system.
*/

/**
   \dir src/BoostTools
   \brief Core utilities for robust, extensible, and type-safe Boost.JSON handling in C++ projects.

   \details
   The `src/BoostTools` directory provides a collection of modern C++ components for seamless integration of Boost.
   JSON with domain-specific C++ types and logic.
   It implements advanced patterns for:
   - **Type-safe value conversion** from JSON to arbitrary C++ types, with robust error handling.
   - **Policy-based validation**, allowing decoupled domain checks and reusable validation strategies.
   - **ADL-based object mapping** for clean, extensible deserialization of user-defined types via tag-dispatch.

   These utilities are designed for use in backend servers, data pipelines, and systems that require reliable parsing, validation, 
   and mapping of JSON data from APIs or other external sources.
   All components follow a modern, trait- and policy-driven architecture, focusing on extensibility, safety, and clarity.

   The directory typically includes:
   - Traits and converters for primitive and chrono-based types.
   - Validator policies for ranges, date checks, and application rules.
   - Infrastructure for generic deserialization of complex objects.
*/

/**
   \dir src/BoostTools/include
   \brief Public interface headers for BoostTools utilities.

   \details
   This directory contains all public header files of the BoostTools component suite, providing modern C++ interfaces for type-safe 
   JSON conversion, validation policies, and extensible object mapping based on Boost.JSON.

   These headers are intended for direct inclusion in application and library code.
*/

/**
\dir src/WeatherAPI
\brief Shared library and public interface for weather data access and formatting based on Open-Meteo.

\details
This directory contains the complete implementation and public interface of the WeatherAPI shared library, which provides robust, type-safe access to weather data from the Open-Meteo service.
It includes all data structures, parsing, reporting, and utility functions for working with weather requests and responses in C++ projects.

**Key files:**
- \c WeatherAPI.h : Import/export macro and platform handling for library usage.
- \c WeatherData.h / .cpp : Data structures and type definitions for all weather value types.
- \c WeatherReader.h / .cpp : High-level parsing, Open-Meteo API access, error handling, and JSON mapping.
- \c WeatherOutput.h / .cpp : Reporting and formatted print functions for all weather types.

**Build and integration:**
- The \c CMakeLists.txt defines this directory as a shared library target, exporting all public symbols and including this folder as a header search directory for dependent projects.
- Downstream projects can consume WeatherAPI by linking the shared library and including headers from this directory.

**Design highlights:**
- Strict separation of data, parsing, and reporting logic for maintainable code.
- Integration with BoostTools for extensible, policy-based JSON conversion and validation.
- Modular structure for easy extension as Open-Meteo expands its API.

\warning
Do not add application-specific or UI logic to this directory; keep it strictly for data and API definitions, shared library implementation, and core reporting.

\todo Add further build options or targets to CMake for static library, testing, or additional tools.
*/

/**
\dir src/WeatherAPIServer
\brief Server-side weather data acquisition and event processing using Open-Meteo and Statechart.

\details
This directory contains the implementation of a modular server that retrieves, processes, and manages weather data via the Open-Meteo REST API.
The server leverages the WeatherAPI shared library for all parsing, validation, and formatting of weather responses, and uses Boost.Statechart to model and control the system’s state transitions and asynchronous event handling.

**Architecture highlights:**
- Integrates Open-Meteo’s REST API for live and forecast weather data retrieval.
- Uses the WeatherAPI shared library for all core data types, JSON mapping, and reporting.
- Employs \c boost::statechart for robust, extensible modeling of the server’s internal finite state machine (FSM), ensuring reliable state and error management.
- System events and transitions are triggered and scheduled by the central \c Scheduler class, enabling periodic updates, retries, and event-driven workflow.

**Key components:**
- WeatherProxy: Provides all weather data structures, conversion, and print routines.
- Scheduler: Triggers and orchestrates FSM events (such as data requests, updates, and error handling).
- Statechart FSM: Manages states such as idle, fetching, error, updating, etc., allowing clear extensibility and testability.

\warning
This directory is focused on backend/service logic only; client, GUI, or business-layer integration should be handled elsewhere.

\todo Extend the FSM for additional error recovery, health checks, or push-style notification.
\todo Add interfaces for logging and operational metrics.

*/