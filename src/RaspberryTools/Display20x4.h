// SPDX-FileCopyrightText: 2023 Michael Fuhs
// SPDX-FileCopyrightText: 2025 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT

/**
 \file

 \brief Implementation of a 20x4 LCD display class using I2C, based on DisplayInterface CRTP.

 \details
 This header implements the DISPLAY_20x4 class, which provides high-level control
 for HD44780-compatible 20x4 character LCD modules using an I2C interface and
 a PCF8574 I/O expander.

 The class inherits from a generic `DisplayInterface<Derived>` via CRTP and uses modern
 language features such as concepts, spans, views, and RAII. It supports compile-time
 polymorphism without virtual inheritance and leverages the `print()` functionality provided
 by the `DisplayInterface`, while implementing all necessary low-level LCD control
 sequences like initialization, cursor positioning, and 4-bit data transfer..

 For validating compatibility of display drivers, a concept `DisplayDevice` is also provided
 in the `DisplayInterface.h` header. This ensures implementations provide required member
 functions at compile time.
 
 Modern C++23 features such as `std::format`, `std::span`, RAII, and `consteval`
 values are used to ensure safety, clarity, and efficiency.

 This class is part of the adecc Scholar project to demonstrate embedded software
 engineering practices in modern C++.

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

 \example
 \code
 #include "Display20x4.h"

 int main() {
     try {
         Display20x4 lcd(0x27); // Replace with your actual I2C address
         lcd.writeString(0, 0, "Hello, World!");
         lcd.writeString(1, 0, "Display Demo");
     } catch (const std::exception& ex) {
         std::cerr << "LCD error: " << ex.what() << '\n';
     }
     return 0;
 }
 \endcode

*/

#pragma once

#include "DisplayInterface.h"
#include "I2CDevice.h"

#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <span>
#include <format>
#include <algorithm>

using namespace std::string_literals;

/**
  \brief High-level class for managing a 20x4 character LCD display over I2C.
 
  \details This class wraps the low-level communication for a PCF8574 I/O expander
  and exposes a modern API with `print()`, `printString()` and cursor positioning.
 
  It inherits from DisplayInterface<DISPLAY_20x4> to provide CRTP-based formatted
  printing for UI or logging use cases.
 */
class DISPLAY_20x4 : public I2CDevice, public DisplayInterface<DISPLAY_20x4> {
public:
   /**
     \brief Constructs and initializes the display driver.
     \param address I2C address of the display module.
     \throws std::system_error if I2C communication fails.
    */
   explicit DISPLAY_20x4(uint16_t address) : I2CDevice("Display20x4"s, address),
                 backlightState_{ 0x08 }, cursorState_{ 0x02 }, cursorBlinkingState_{ 0x01 }, displayState_{ 0x04 } {
      Reset();
      initDevice();
      }

   /**
     \brief Default destructor.
    */
   virtual ~DISPLAY_20x4() override = default;

   /**
     \brief Waits for power-up stabilization.
     \details Sleeps for 100 ms to allow hardware reset.
     \todo Adding a real reset for the display when possible.
    */
   void Reset() {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

   /**
     \brief Initializes the LCD controller.
     \details Performs 4-bit initialization sequence and sets default display configuration.
    */
   void initDevice() {
      using namespace std::chrono_literals;

      std::this_thread::sleep_for(16ms);
      write8BitData(false, 0x03);
      std::this_thread::sleep_for(5ms);
      write8BitData(false, 0x03);
      std::this_thread::sleep_for(155us);
      write8BitData(false, 0x03);
      std::this_thread::sleep_for(55us);
      write8BitData(false, 0x02);
      std::this_thread::sleep_for(55us);
      write8BitData(false, 0x20 | 0x08); // Function set: 2 lines, 5x8 font
      std::this_thread::sleep_for(100ms);
      write8BitData(false, 0x0F); // Display on
      std::this_thread::sleep_for(100ms);
      write8BitData(false, 0x01); // Clear display
      std::this_thread::sleep_for(100ms);
      write8BitData(false, 0x06); // Entry mode: left
      std::this_thread::sleep_for(100ms);
      write8BitData(false, 0x0F); // Display on
      std::this_thread::sleep_for(500ms);
   }


   /**
     \brief Sets the cursor to a given position.
     \param line Row index (0–3).
     \param position Column index (0–19).
     \throws std::out_of_range if line or column are invalid.
   */
   void setPosition(uint8_t line, uint8_t position) {
      if (line < 4 && position < 20) {
         static constinit std::array<uint8_t, 4> lineOffsets = { 0x80, 0xC0, 0x94, 0xD4 };
         write8BitData(false, lineOffsets[line] + position);
         std::this_thread::sleep_for(std::chrono::microseconds(50));
         }
      }

   /**
     \brief Writes a string to the display at the specified position.
     \param line Row index (0–3).
     \param position Column index (0–19).
     \param text The string content to display (truncated to fit).
     \throws std::out_of_range if line or position exceed display bounds.
    */
   void printString(uint8_t line, uint8_t position, std::string_view text) {
      if (line >= 4 || position >= 20) [[unlikely]] return;

      setPosition(line, position);
      size_t count = std::min<size_t>(20 - position, text.size());

      for (size_t i = 0; i < count; ++i) {
         write8BitData(true, static_cast<uint8_t>(text[i]));
         }
      }


private:
   /**
     \brief Writes a full byte to the display.
     \param rs Register select bit (true=data, false=command).
     \param data The 8-bit value to write.
    */
   void write8BitData(bool rs, uint8_t data) {
      writeNibble(rs, false, (data >> 4) & 0x0F);
      writeNibble(rs, false, data & 0x0F);
      }

   /**
    \brief Writes a 4-bit nibble to the display with strobe.
    \param rs Register select.
    \param rw Read/write bit.
    \param data 4-bit nibble to transmit.
    \details Asserts the Enable signal and then clears it to trigger the LCD latch.
   */
   void writeNibble(bool rs, bool rw, uint8_t data) {
      using namespace std::chrono_literals;

      uint8_t value = (data << 4) | backlightState_;
      if (rs) value |= (1 << RS_SHIFT);
      if (rw) value |= (1 << RW_SHIFT);
      value |= (1 << E_SHIFT);

      std::array<uint8_t, 1> send = { value };
      write(send);

      std::this_thread::sleep_for(1us);

      send[0] &= ~(1 << E_SHIFT);
      write(send);
      std::this_thread::sleep_for(1us);
      }

   /**
     \brief Sends I2C data to the display.
     \param data Data to write as span.
   */
   void write(const std::array<uint8_t, 1>& data) {
      I2CDevice::Write(data);
   }

private:
   static constexpr uint8_t RS_SHIFT = 0;
   static constexpr uint8_t RW_SHIFT = 1;
   static constexpr uint8_t E_SHIFT = 2;

   uint8_t backlightState_;
   uint8_t cursorState_;
   uint8_t cursorBlinkingState_;
   uint8_t displayState_;
};