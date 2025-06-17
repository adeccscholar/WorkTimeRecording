// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * \file 
   \brief Documentation overview for the RaspberryTools hardware abstraction layer.
 
   \details
   This header provides an overview and documentation grouping for the **RaspberryTools** component library,
   a modular, header-only C++ collection for accessing and abstracting low-level GPIO, I²C, and SPI hardware
   interfaces on Raspberry Pi and compatible single-board systems.
 
   The goal of this library is to enable modern, platform-specific interaction with a variety of sensors and modules,
   using idiomatic, readable C++ without requiring complex runtime dependencies. The library is designed to be
   extensible and cross-compatible with common open-source hardware used in embedded and IoT environments.
 
   \note
   This is a **documentation-only** file. It contains no functional code.
 
   \ingroup RaspberryTools
 
   ---
 
   ### Included interfaces and drivers:
 
   - **Bus Abstractions**
     - `SPI_Interface.h` — SPI bus abstraction layer
     - `I2C_Interface.h` — I²C bus abstraction layer
 
   - **Sensor/Device Drivers**
     - `BME280_Driver.h` — Bosch BME280 sensor for temperature, humidity, and pressure
     - `BH1750_Driver.h` — ROHM BH1750 ambient light intensity sensor (lux)
     - `SSD1306_Display.h` — OLED display controller using I²C
     - `MFRC522_RFID.h` — NXP MFRC522 RFID reader/writer via SPI
 
    ---
 
    ### Design goals:
    - Header-only and portable, built with modern C++17/20
    - Hardware-specific yet reusable across Raspberry Pi versions
    - Clean API to isolate GPIO/I²C/SPI logic from application layer
    - Based on or extended from proven open-source projects under permissive licenses
 
    ---
 
    ### Licensing:
    This library combines or adapts source code from multiple open-source projects.
    All license texts are provided in the `LICENSES/` directory of the repository.
    Unless otherwise noted, modifications and extensions are licensed under the MIT License.
 
    Example sources:
    - BME280 sensor logic derived from Bosch Sensortec reference implementation
    - BH1750 and MFRC522 logic adapted from open community C/C++ projects
    - SPI and I²C interfaces based on Linux sysfs and `/dev` IO models
 
    ---
 
    ### Usage examples:
    \code
    #include "SPI_Interface.h"
    #include "MFRC522_RFID.h"
 
    SPI_Interface spi("/dev/spidev0.0");
    MFRC522_RFID rfid(spi);
 
    if (rfid.init()) {
       if (rfid.cardPresent()) {
           auto id = rfid.readUID();
           std::cout << "RFID Tag: " << id << std::endl;
          }
       }
   \endcode
 
   ---
 
   \author Volker Hillmann (adecc Systemhaus GmbH)
   \date 16.06.2025
   \copyright
   Copyright © 2020–2025 adecc Systemhaus GmbH  
   This program is free software: you can redistribute it and/or modify it  
   under the terms of the GNU General Public License, version 3.  
   See <https://www.gnu.org/licenses/>.

   \note This documentation is part of the adecc Scholar project —  
         Free educational materials for distributed systems in modern C++.
 */