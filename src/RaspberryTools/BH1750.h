// SPDX-FileCopyrightText: 2023 Michael Fuhs
// SPDX-FileCopyrightText: 2024 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT
/**
 \file 
 \brief Concrete class to interface with the BH1750 light sensor over I2C.
 \details This class inherits from I2CDevice and provides a high-level method
          to read ambient light levels using continuous high-resolution mode.
          The sensor must be connected to a Linux-based I2C interface, e.g. on a Raspberry Pi.
 \author Michael Fuhs (original)
 \author Volker Hillmann (adecc Systemhaus GmbH)

 \copyright © 2023–2025 adecc Systemhaus GmbH
 \licenseblock{MIT}
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 \endlicenseblock

 \note This file is part of the adecc Scholar project – Free educational materials for modern C++.
 \version 2.0
 \date    25.06.2025
*/

#pragma once

#include "I2CDevice.h"
#include "SensorInterfaces.h"

#include <array>
#include <chrono>
#include <thread>
#include <cstdint>

using namespace std::string_literals;

/**
 \brief Driver class for the BH1750 ambient light sensor.

 \details This class uses the I2CDevice abstraction to send measurement 
          commands and read lux values from the sensor. It supports reading 
          both raw and  calibrated values and conforms to 
          LightSensorInterface<BH1750Device>.

 \details Uses continuous high-resolution mode (0x10) by default.
*/
class BH1750Device : public I2CDevice, 
                     public LightSensorInterface<BH1750Device>,
                     public GenericSensorInterface<BH1750Device> {
public:
   /**
    \brief Constructs the sensor object and initializes the I2C base class.
    \param address The I2C address of the BH1750 sensor (typically 0x23 or 0x5C).
   */
   explicit BH1750Device(uint16_t address) : I2CDevice("BH1750"s, address) {  }

   /**
    \brief Defaulted virtual destructor to ensure proper cleanup in inheritance.
    \note Marked virtual to support polymorphic use.
   */
   virtual ~BH1750Device() override = default;

   /**
    \brief Starts a high-resolution measurement and stores the result.
    \details Reads the ambient light level from the BH1750 sensor. 
    \note Applies the conversion factor of 1.2 to compute calibrated lux.
    \throws std::system_error if I2C communication fails.
   */
   void readLuxImpl() {
      using namespace std::chrono_literals;
      constexpr std::array<uint8_t, 1> cmd = { 0x10 }; // Continuous H-Resolution Mode
      Write(cmd);
      std::this_thread::sleep_for(180ms);

      std::array<uint8_t, 2> response{};
      Read(response);

      rawLux_ = static_cast<uint16_t>((response[0] << 8) | response[1]);
      calibratedLux_ = rawLux_ / 1.2;
      }

   /**
    \brief Retrieves the raw lux reading from the sensor.
    \details     This method returns the last raw 16-bit lux value as received 
                 from the BH1750 sensor. The raw value is sensor-specific and 
                 not scaled to human-readable physical units. It represents the 
                 direct digital output of the sensor after the last `read()` 
                 operation.
    \return Unsigned 16-bit raw lux value (0–65535), depending on light level.
   */
   [[nodiscard]] uint16_t getRawLuxImpl() const { return rawLux_; }

   /**
    \brief Retrieves the calibrated lux value.
    \details This method returns the calibrated lux value, computed from the 
             last raw reading by applying the BH1750 datasheet’s scaling factor 
             of 1.2. The result is a human-readable illuminance in lux (lx).
    \note This value is only valid after a successful call to `read()`.
    \return Illuminance in lux as a double-precision floating point value.
   */
   [[nodiscard]] double getCalibratedLuxImpl() const { return calibratedLux_; }

   /**
    \brief Structured access to sensor values for JSON/export.
    \details Provides name/unit/value triplets compatible with GenericSensorInterface.
    \return Vector with lux value entry.
   */
   [[nodiscard]] std::vector<SensorValue> sensorValuesImpl() const {
      return { { "Lux", "lx", calibratedLux_ } };
      }

private:
   int rawLux_ = 0;             ///< Last raw measurement value (digital output from sensor)
   double calibratedLux_ = 0.0; ///< Last computed lux value after applying calibration factor
};

