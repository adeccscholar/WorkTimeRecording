# adecc Scholar: Distributed Enterprise Time-Tracking System

## Overview

This open-source project is developed as part of the free training initiative **adecc Scholar**, which began in autumn 2020 during the COVID-19 pandemic with free livestreams on the [adecc Scholar Twitch channel](https://www.twitch.tv/volker_adecc).

The goal of this project is to teach the development of modern distributed systems in **C++23** through a practical example — a time-tracking system within a company.

## Main Objectives

- Teach modern C++ techniques (including **C++23**) and frameworks  
- Design and implement a distributed system across various device classes (server, desktop, Raspberry Pi, mobile devices)  
- Access databases using SQL  
- Integrate sensor and device control (GPIO, I2C, SPI)  
- Discuss project management, database design, and normalization  
- Demonstrate cross-platform development (Windows Server, Ubuntu Server, Windows/Linux desktops, embedded devices)

## Architecture

### 1. Application Server

- Operating system: **Ubuntu Linux**
- CORBA/TAO as middleware for distributed objects
- ODBC access to MS SQL Server
- Central implementation of business logic

### 2. Database Server

- Operating system: **Windows Server**
- Microsoft SQL Server
- Relational modeling and normalization

### 3. Desktop Clients

- Windows and Linux desktops
- CORBA/TAO as middleware
- Qt6-based GUI for time tracking and reporting

### 4. Embedded Clients (Raspberry Pi)

- CORBA/TAO (client and server)
- Access to GPIO pins for discrete signals
- Use of I2C/SPI for sensors, displays, and RFID readers
- Operate independently as both client and (local) server
- RFID-based access control using a finite state machine (Boost.SML)

### 5. Future Extensions

- Web REST server for mobile device integration
- Discussion of architectural decisions, risks, and opportunities

## Technologies and Libraries Used

- **C++23**: Latest language features for efficiency and platform independence  
- **Visual Studio C++**: IDE used for livestream development and debugging  
  - Modern and supports cross-compilation  
  - Free community version available  
- **GCC14** via cross-compilation on Linux platforms, including Raspberry Pi  
- **CMake** for build processing  
  - Custom routines, including CORBA workflows  
- **Boost**
  - **Boost.SML** for finite state machines  
  - **Boost.JSON** for JSON processing (in future extension)  
  - **Boost.Beast** for alternative HTTP communication (in future extension)  
- **CORBA/TAO**: Latest TAO version as middleware  
  - ORB and POA basics, transient and persistent servants  
  - CORBA Naming Services  
  - CORBA Event Service, and later TAO Events for direct communication  
- **Qt 6**: Database access (Qt SQL) and GUI development  
  - QDatabase and QWidgets  
  - Free version available for open-source projects, downloadable and buildable from source  
- **Embarcadero C++ Builder**
  - Integration of legacy tools (VCL, FMX)  
  - Bridge between legacy code and modern systems  
- **MS SQL Server**: Relational database  
  - Free Express version or Developer Edition available for developers  

### Disclaimer

In this video/stream, I use software provided through a Visual Studio Professional Subscription from Microsoft. 
The software shown (e.g., Visual Studio, SQL Server) is used exclusively for demonstration, development, and 
testing purposes. No production use or redistribution is involved. This content is intended for free educational 
purposes and is not commercially affiliated with Microsoft.


### Embarcadero C++ Builder

By supporting **Embarcadero C++ Builder** (VCL and FMX), the project demonstrates how older or less modern tools can be integrated into a modern system architecture. This opens up possibilities for connecting existing legacy applications and migrating them step-by-step.

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/adeccscholar/WorkTimeRecording.git
   ```

2. Open the C++ server components with Visual Studio and deploy them to the Ubuntu Server
3. Compile and launch the Qt clients
4. Flash Raspberry Pi images and complete the configuration

## License and Copyright

```text
Copyright © adecc Systemhaus GmbH 2020–2025

This repository is mostly licensed under the GNU General Public License v3.0 (GPL-3.0).
Alternative open-source licenses may apply in subprojects.
```
