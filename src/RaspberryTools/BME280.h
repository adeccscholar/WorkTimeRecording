// SPDX-FileCopyrightText: 2024 Bosch Sensortec GmbH
// SPDX-FileCopyrightText: 2024 Michael Fuhs
// SPDX-FileCopyrightText: 2025 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: BSD-3-Clause AND MIT

/**
 \file

 \brief High-level C++ driver for Bosch BME280 environmental sensor using CRTP interfaces.
 \details This is a fully modernized wrapper around the Bosch BME280 sensor, adapted 
          for I2C access via the `I2CDevice` abstraction layer and structured using modern 
          C++ features (C++20+).

 \details Core design features:
 - Uses CRTP-based `EnvironmentalSensorInterface` to provide `read()`, `temperature()`, `humidity()`, `pressure()` methods.
 - Incorporates calibration logic and compensation equations directly from Bosch’s official BSD-licensed reference code.
 - Supports safe communication with the sensor via I2C with system_error-based exception handling.
 - Designed for single-threaded use.

 \details Architecture highlights:
 - Compile-time polymorphism via CRTP (no virtuals, no dynamic dispatch).
 - Portable across Linux-capable systems with `/dev/i2c-*` support.
 - Easily mockable and testable through interface concepts.

 \note Part of the adecc Scholar project – modern embedded C++ learning suite.

 \author Michael Fuhs
 \author Volker Hillmann (adecc Systemhaus GmbH)
 \details
 This code is originally based on the official C implemented Bosch BME280 driver, 
 which was published by Bosch Sensortec GmbH under the BSD-3-Clause License:
 <https://github.com/BoschSensortec/BME280_SensorAPI>

 This modified version has been:
 - adapted by Michael Fuhs in 2024 for I2C abstraction and reimplemented with C++ and classes
 - integrated and refactored by Volker Hillmann (adecc Systemhaus GmbH) in 2024, 2025
   as part of the **adecc Scholar** project under the MIT License. 

 \note
 The original BSD-3-Clause License and Copyright notice of Bosch
 remains applicable to all code derived from their implementation.
 You find a copy of the BSD-3-Clause license in the License Folder of the project.

 \copyright 
 Bosch code: Copyright © Bosch Sensortec GmbH – BSD-3-Clause  
 Additions: Copyright © 2024–2025 adecc Systemhaus GmbH – MIT License
 */

#pragma once

#include "I2CDevice.h"
#include "SensorInterfaces.h"

#include <array>
#include <string>
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <print>

using namespace std::string_literals;

 /**
  \class BME280Device
  \brief High-level interface for Bosch BME280 environmental sensor (I2C-based).

  \details
  This class derives from `I2CDevice` for communication and implements both:
  - `EnvironmentalSensorInterface<BME280Device>`
  - `GenericSensorInterface<BME280Device>`

  Sensor data can be accessed via:
  - `temperature()` – Temperature in °C
  - `humidity()` – Relative humidity in %
  - `pressure()` – Pressure in hPa
  - `sensorValues()` – JSON-compatible key/value output

  Call `read()` to update values before access.
 */
class BME280Device : public I2CDevice,
                     public EnvironmentalSensorInterface<BME280Device>,
                     public GenericSensorInterface<BME280Device> {
public:
   /**
     \brief Constructs a BME280 sensor interface.
     \param i2cAddress I2C 7-bit address (usually 0x76 or 0x77).
   */
   explicit BME280Device(uint16_t i2cAddress) : I2CDevice("BME280"s, i2cAddress) {
      BME_ReadID();
      ReadCalibration();
      InitControl();
      }

   /**
     \brief Default destructor.
     \note Does not power down the sensor.
   */
   virtual ~BME280Device() override = default;

   /**
    \brief Updates all sensor readings (temperature, pressure, humidity).
    \details Must be called before accessing any values.
   */
   void readEnvImpl() {
      std::array<uint8_t, 2> control = { 0xF4, ((2 << 5) | (5 << 2) | (1 << 0)) };
      Write(control);

      std::array<uint8_t, 1> cmd = { 0xF7 };
      std::array<uint8_t, 8> response;
      WriteRead(cmd, response);

      uint32_t data_msb, data_lsb, data_xlsb;

      // Pressure
      data_msb = static_cast<uint32_t>(response[0]) << 12;
      data_lsb = static_cast<uint32_t>(response[1]) << 4;
      data_xlsb = static_cast<uint32_t>(response[2]) >> 4;
      uint32_t up = data_msb | data_lsb | data_xlsb;
      fPressure = compensate_pressure(up) / 100.0;

      // Temperature
      data_msb = static_cast<uint32_t>(response[3]) << 12;
      data_lsb = static_cast<uint32_t>(response[4]) << 4;
      data_xlsb = static_cast<uint32_t>(response[5]) >> 4;
      uint32_t ut = data_msb | data_lsb | data_xlsb;
      fTemperature = compensate_temperature(ut);

      // Humidity
      data_msb = static_cast<uint32_t>(response[6]) << 8;
      data_lsb = static_cast<uint32_t>(response[7]);
      uint32_t uh = data_msb | data_lsb;
      fHumidity = compensate_humidity(uh);
   }

   /**
    \brief Returns last measured temperature in °C.
    \return Temperature as double (typically -40°C to +85°C).
    */
   [[nodiscard]] double getTemperatureImpl() const { return fTemperature; }

   /**
    \brief Returns last measured pressure in hPa.
    \return Barometric pressure (typically 300–1100 hPa).
    */
   [[nodiscard]] double getPressureImpl() const { return fPressure; }

   /**
    \brief Returns last measured humidity in %RH.
    \return Relative humidity (0–100%).
    */
   [[nodiscard]] double getHumidityImpl() const { return fHumidity; }

   /**
    \brief Returns a structured view of sensor values.
    \details Each value includes a name, unit and value for display or JSON serialization.
    \return A vector of SensorValue triples.
    */
   [[nodiscard]] std::vector<SensorValue> sensorValuesImpl() const {
      return { { "Temperature", "°C", fTemperature },
               { "Humidity", "%", fHumidity },
               { "Pressure", "hPa", fPressure } };
      }

private:
   /**
    \brief Reads the sensor ID register (0xD0) and prints it.
    \details Used to identify the BME280 variant (production vs. sample).
    */
   void BME_ReadID() {
      std::array<uint8_t, 1> cmd = { 0xD0 };
      WriteRead(cmd, cmd);
      switch (cmd[0]) {
      default: std::println("BME ID=0x{:02X} (unknown)", cmd[0]); break;
      case 0x58: std::println("BME ID=0x{:02X} (mass production)", cmd[0]); break;
      case 0x56: case 0x57: std::println("BME ID=0x{:02X} (samples)", cmd[0]); break;
      }
   }

   /**
    \brief Initializes sensor configuration registers.
    \details Sets filter, oversampling and measurement control parameters.
   */
   void InitControl() {
      Write(std::span<const uint8_t, 2>{ { 0xF5, static_cast<uint8_t>((4 << 5) | (4 << 2)) } }); // config
      Write(std::span<const uint8_t, 2>{ { 0xF2, static_cast<uint8_t>(1 << 0) } });              // ctrl_hum
      Write(std::span<const uint8_t, 2>{ { 0xF4, static_cast<uint8_t>((2 << 5) | (5 << 2) | (1 << 0)) } }); // ctrl_meas
      }

   /**
    \brief Reads all sensor calibration values from internal NVM.
    \details Required once at initialization. Used by compensation formulas.
    */
   void ReadCalibration() {
      std::array<uint8_t, 1> in = { 0x88 };
      std::array<uint8_t, 24> out;
      WriteRead(in, out);

      dig_T1 = (out[1] << 8) | out[0];
      dig_T2 = (out[3] << 8) | out[2];
      dig_T3 = (out[5] << 8) | out[4];

      dig_P1 = (out[7] << 8) | out[6];
      dig_P2 = (out[9] << 8) | out[8];
      dig_P3 = (out[11] << 8) | out[10];
      dig_P4 = (out[13] << 8) | out[12];
      dig_P5 = (out[15] << 8) | out[14];
      dig_P6 = (out[17] << 8) | out[16];
      dig_P7 = (out[19] << 8) | out[18];
      dig_P8 = (out[21] << 8) | out[20];
      dig_P9 = (out[23] << 8) | out[22];

      in[0] = 0xA1;
      std::array<uint8_t, 1> h1;
      WriteRead(in, h1);
      dig_H1 = h1[0];

      in[0] = 0xE1;
      std::array<uint8_t, 8> h2;
      WriteRead(in, h2);

      dig_H2 = (h2[1] << 8) | h2[0];
      dig_H3 = h2[2];
      dig_H4 = (h2[3] << 4) | (h2[4] & 0x0F);
      dig_H5 = (h2[5] << 4) | (h2[4] >> 4);
      dig_H6 = h2[6];
   }

   /**
    \brief Applies temperature compensation to raw ADC value.
    \param adc_T Raw 20-bit temperature value.
    \return Temperature in °C, clamped to sensor spec (-40...85).
    */
   double compensate_temperature(uint32_t adc_T) {
      double var1 = (adc_T / 16384.0 - dig_T1 / 1024.0) * dig_T2;
      double var2 = ((adc_T / 131072.0 - dig_T1 / 8192.0) *
         (adc_T / 131072.0 - dig_T1 / 8192.0)) * dig_T3;
      t_fine = static_cast<int32_t>(var1 + var2);
      double T = (var1 + var2) / 5120.0;
      return std::clamp(T, -40.0, 85.0);
   }

   /**
    \brief Applies pressure compensation to raw ADC value.
    \param adc_P Raw 20-bit pressure value.
    \return Pressure in Pascal, clamped to physical limits.
    */
   double compensate_pressure(uint32_t adc_P) {
      double var1 = t_fine / 2.0 - 64000.0;
      double var2 = var1 * var1 * dig_P6 / 32768.0;
      var2 += var1 * dig_P5 * 2.0;
      var2 = var2 / 4.0 + dig_P4 * 65536.0;
      var1 = (dig_P3 * var1 * var1 / 524288.0 + dig_P2 * var1) / 524288.0;
      var1 = (1.0 + var1 / 32768.0) * dig_P1;

      if (var1 == 0.0) return 30000.0; // avoid division by zero

      double p = 1048576.0 - adc_P;
      p = (p - var2 / 4096.0) * 6250.0 / var1;
      var1 = dig_P9 * p * p / 2147483648.0;
      var2 = p * dig_P8 / 32768.0;
      p += (var1 + var2 + dig_P7) / 16.0;

      return std::clamp(p, 30000.0, 110000.0);
   }

   /**
 \brief Applies humidity compensation to raw ADC value.
 \param adc_H Raw 16-bit humidity value.
 \return Relative humidity in percent [%RH], clamped to 0–100%.
 */
   double compensate_humidity(uint32_t adc_H) {
      double var1 = t_fine - 76800.0;
      double var2 = (dig_H4 * 64.0 + dig_H5 / 16384.0 * var1);
      double var3 = adc_H - var2;
      double var4 = dig_H2 / 65536.0;
      double var5 = 1.0 + dig_H3 / 67108864.0 * var1;
      double var6 = 1.0 + dig_H6 / 67108864.0 * var1 * var5;
      double h = var3 * var4 * var5 * var6;
      h = h * (1.0 - dig_H1 * h / 524288.0);
      return std::clamp(h, 0.0, 100.0);
   }

private:
   /// \name Temperature Calibration Coefficients
   /// \brief Factory calibration constants for temperature readings.
   /// \details These values are factory-calibrated temperature coefficients 
   ///          read from registers 0x88 to 0x8D of the BME280 sensor. They  
   ///          are used to compensate raw temperature measurements.  
   /// \{
   uint16_t dig_T1;  ///< Unsigned temperature coefficient T1 from registers 0x88–0x89.
   int16_t  dig_T2;  ///< Signed temperature coefficient T2 from registers 0x8A–0x8B.
   int16_t  dig_T3;  ///< Signed temperature coefficient T3 from registers 0x8C–0x8D.
   /// \}
  
   /// \name Pressure Calibration Coefficients
   /// \brief Factory calibration constants for barometric pressure readings
   /// \details These coefficients are read from registers 0x8E to 0x9F 
   ///          and used in the BME280 pressure compensation formula.
   /// \{
   uint16_t dig_P1;  ///< Unsigned pressure coefficient P1 from registers 0x8E–0x8F.
   int16_t  dig_P2;  ///< Signed pressure coefficient P3 from registers 0x92–0x93.
   int16_t  dig_P3;  ///< Signed pressure coefficient P3 from registers 0x92–0x93.
   int16_t  dig_P4;  ///< Signed pressure coefficient P4 from registers 0x94–0x95.
   int16_t  dig_P5;  ///< Signed pressure coefficient P5 from registers 0x96–0x97.
   int16_t  dig_P6;  ///< Signed pressure coefficient P6 from registers 0x98–0x99.
   int16_t  dig_P7;  ///< Signed pressure coefficient P7 from registers 0x9A–0x9B.
   int16_t  dig_P8;  ///< Signed pressure coefficient P8 from registers 0x9C–0x9D.
   int16_t  dig_P9;  ///< Signed pressure coefficient P9 from registers 0x9E–0x9F.   
   /// \}
  
   /// \name Humidity Calibration Coefficients
   /// \brief   Calibration coefficient for humidity (H1), read from sensor register 0xA1.
   /// \details These coefficients are read from registers 0xA1 and 0xE1–0xE7. 
   ///          They are used to compensate raw humidity readings.
   /// \details Used as part of the compensation formula for relative humidity.
   /// \{
   uint8_t dig_H1;  ///< Unsigned humidity coefficient H1 from register 0xA1.
   int16_t dig_H2;  ///< Signed humidity coefficient H2 from registers 0xE1–0xE2.
   uint8_t dig_H3;  ///< Unsigned humidity coefficient H3 from register 0xE3.
   int16_t dig_H4;  ///< Signed humidity coefficient H4 from register bits 0xE4[7:0] and 0xE5[3:0].
   int16_t dig_H5;  /// Signed humidity coefficient H5 from register bits 0xE6[7:0] and 0xE5[7:4].
   int8_t  dig_H6;  /// Signed humidity coefficient H6 from register 0xE7.
   /// \}
   
   /// \name Compensation and Output Values
   /// \brief Intermediate computation and final result storage for compensated sensor data
   /// \details Internal compensation and result registers used for storing
   ///          final sensor values and the intermediate fine resolution value.
   /// \{

   /// Internal fine-resolution temperature value used in pressure and humidity compensation.
   int32_t t_fine; 
   double fTemperature = 0.0; ///< Compensated and scaled temperature value in degrees Celsius.
   double fPressure = 0.0;    ///< Compensated and scaled pressure value in Pascal.
   double fHumidity = 0.0;    ///< Compensated and scaled relative humidity value in %RH.
   /// \}
};

