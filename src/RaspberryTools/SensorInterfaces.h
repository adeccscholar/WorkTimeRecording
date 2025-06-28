// SPDX-FileCopyrightText: 2024 Michael Fuhs
// SPDX-FileCopyrightText: 2025 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT

/**
 \file

 \brief Header providing modern C++ sensor interfaces using CRTP and concept constraints.

 \details
 This file defines flexible and compile-time efficient interfaces for environmental and light sensors
 using the Curiously Recurring Template Pattern (CRTP). It includes:

 - Concepts (`LightSensor`, `EnvironmentalSensor`) to validate sensor interfaces.
 - Interface classes (`LightSensorInterface`, `EnvironmentalSensorInterface`) to enforce consistent APIs.
 - A `GenericSensorInterface` for exposing structured sensor data using name/unit/value triples.
 - A `SensorValue` struct for representing typed sensor output.
 - Simulated implementations (`SimulatedLightSensor`, `SimulatedEnvironmentalSensor`) for testing,
   validation, or fallback operation when hardware is unavailable.

 The approach avoids virtual inheritance and runtime polymorphism by leveraging C++20/23 features,
 such as concepts, `std::variant`, `std::string_view`, and `std::vector`.

 These abstractions are intended to be used in embedded, industrial, or prototyping environments
 where flexibility, safety, and testability are important.

 \author Michael Fuhs (original CRTP and simulation interface)
 \author Volker Hillmann (concepts, integration, documentation, modernization)

 \copyright © 2024–2025 adecc Systemhaus GmbH

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

 \note This header is part of the adecc Scholar project — Free educational materials for modern C++.
 \version 2.1
 \date    25.06.2025

 \example
 \code
 SimulatedLightSensor light;
 SimulatedEnvironmentalSensor env;

 light.read();
 env.read();

 for (const auto& sv : light.sensorValues())
     std::println("{}: {} {}", sv.name, std::visit([](auto v) { return v; }, sv.value), sv.unit);

 for (const auto& sv : env.sensorValues())
     std::println("{}: {} {}", sv.name, std::visit([](auto v) { return v; }, sv.value), sv.unit);
 \endcode
 */

#pragma once

#include <concepts>
#include <vector>
#include <string>
#include <variant>
#include <string_view>
#include <cstdint>

/**
 \brief Concept for a light sensor interface with CRTP structure.
*/
template<typename ty>
concept LightSensor = requires(ty t) {
      { t.read() } -> std::same_as<void>;
      { t.getCalibratedLux() } -> std::convertible_to<double>;
   };

/**
 \brief Concept for an environmental sensor interface with CRTP structure.
*/
template<typename ty>
concept EnvironmentalSensor = requires(ty t) {
      { t.read() } -> std::same_as<void>;
      { t.temperature() } -> std::convertible_to<double>;
      { t.pressure() } -> std::convertible_to<double>;
      { t.humidity() } -> std::convertible_to<double>;
   };

/**
 \brief Interface for sensors that provide ambient light measurements (lux).
 \tparam Derived The concrete sensor class implementing the interface.
*/
template<typename Derived>
class LightSensorInterface {
public:
    void read() {
        static_cast<Derived*>(this)->readLuxImpl();
       }

    [[nodiscard]] uint16_t getRawLux() const {
        return static_cast<const Derived*>(this)->getRawLuxImpl();
       }

    [[nodiscard]] double getCalibratedLux() const {
        return static_cast<const Derived*>(this)->getCalibratedLuxImpl();
       }
};

/**
 \brief Interface for sensors that provide temperature, humidity, and pressure readings.
 \tparam Derived The concrete environmental sensor class.
*/
template<typename Derived>
class EnvironmentalSensorInterface {
public:
    void read() {
        static_cast<Derived*>(this)->readEnvImpl();
    }

    [[nodiscard]] double temperature() const {
        return static_cast<const Derived*>(this)->getTemperatureImpl();
    }

    [[nodiscard]] double pressure() const {
        return static_cast<const Derived*>(this)->getPressureImpl();
    }

    [[nodiscard]] double humidity() const {
        return static_cast<const Derived*>(this)->getHumidityImpl();
    }
};



/**
 \brief Optional generic interface for systems that wish to expose sensor data as named values.
*/
struct SensorValue {
   std::string_view name;
   std::string_view unit;
   std::variant<int, double> value;
};

template<typename Derived>
class GenericSensorInterface {
public:
   [[nodiscard]] std::vector<SensorValue> sensorValues() const {
      return static_cast<const Derived*>(this)->sensorValuesImpl();
   }
};

/**
 \brief Simulated light sensor for testing or demonstration purposes.

 \details This class implements the LightSensorInterface for testing purposes 
          without any hardware dependency. It mimics a BH1750-like sensor and 
          returns a fixed raw value and its calibrated equivalent. This allows 
          the interface to be exercised in unit tests, mock environments, or 
          simulation contexts.
*/
class SimulatedLightSensor : public LightSensorInterface<SimulatedLightSensor>,
                             public GenericSensorInterface<SimulatedLightSensor> {
public:
   /**
    \brief Simulates a lux measurement.
    \details Sets a fixed raw lux value and applies a standard conversion factor 
             of 1.2 to produce a calibrated lux reading.
   */
   void readLuxImpl() {
      luxRaw_ = 12345;
      luxCal_ = luxRaw_ / 1.2;
      }

   /**
    \brief Retrieves the raw lux value from the simulated sensor.
    \details Returns the internal integer lux value that would be received 
             from a real light sensor in raw 16-bit format.
    \return The raw lux reading as integer.
   */
   int getRawLuxImpl() const { return luxRaw_; }

   /**
    \brief Retrieves the calibrated lux value from the simulated sensor.

    \details Converts the raw lux reading by dividing it by 1.2, approximating 
             the BH1750 datasheet recommendation. This value reflects real-world 
             illuminance.
    \return Calibrated lux value as floating point.
   */
   double getCalibratedLuxImpl() const { return luxCal_; }

   /**
    * \brief Provides structured sensor data for external use (e.g. logging or JSON).
    *
    * \details This method exposes the current light sensor reading as a vector
    *          of name/unit/value entries. It is used by systems compatible with
    *          `GenericSensorInterface`, such as dashboards, exporters, or loggers.
    *
    * \return A list containing the calibrated lux value.
    */
   [[nodiscard]] std::vector<SensorValue> sensorValuesImpl() const {
      return {
          { "Illuminance", "lx", getCalibratedLux() }
      };
   }

private:
   int luxRaw_ = 0;             ///< Simulated raw sensor value.
   double luxCal_ = 0.0;        ///< Simulated calibrated lux value (raw / 1.2).
};


/**
 * \brief Simulated environmental sensor for testing temperature, humidity and pressure.
 *
 * \details This class mimics a real BME280-like sensor, implementing the
 * `EnvironmentalSensorInterface` and `GenericSensorInterface`. It provides deterministic
 * or randomized simulated values for use in tests, development environments,
 * or fallback logic when hardware is unavailable.
 */
class SimulatedEnvironmentalSensor
   : public EnvironmentalSensorInterface<SimulatedEnvironmentalSensor>,
     public GenericSensorInterface<SimulatedEnvironmentalSensor> {
public:
   /**
    * \brief Simulates a full measurement cycle.
    * \details Assigns fixed or randomized values for T/H/P. You may change the logic
    *          to make it deterministic, sinusoidal, or depend on time.
    */
   void readEnvImpl() {
      // Simulierte Werte oder z. B. Zufallswerte:
      temperature_ = 22.5;
      humidity_ = 45.0;
      pressure_ = 1013.25;
   }

   /**
    * \return Last simulated temperature in Celsius.
    */
   [[nodiscard]] double getTemperatureImpl() const { return temperature_; }

   /**
    * \return Last simulated pressure in hPa.
    */
   [[nodiscard]] double getPressureImpl() const { return pressure_; }

   /**
    * \return Last simulated humidity in percent.
    */
   [[nodiscard]] double getHumidityImpl() const { return humidity_; }

   /**
    * \brief Exposes sensor values in structured form.
    * \return A vector of SensorValue objects.
    */
   [[nodiscard]] std::vector<SensorValue> sensorValuesImpl() const {
      return {
          { "Temperature", "°C", temperature_ },
          { "Humidity",    "%",  humidity_    },
          { "Pressure",    "hPa", pressure_   }
      };
   }

private:
   double temperature_ = 0.0;
   double humidity_ = 0.0;
   double pressure_ = 0.0;
};
