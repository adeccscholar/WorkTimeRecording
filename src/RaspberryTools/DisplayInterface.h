// SPDX-FileCopyrightText: 2023 Michael Fuhs
// SPDX-FileCopyrightText: 2025 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT

/**
 \file

 \brief CRTP-based display interface and validation concepts for HD44780-compatible devices.

 \details
 This header defines a reusable, generic interface for character-based displays that support
 formatted text output (such as HD44780-compatible LCDs over I2C). It uses the
 Curiously Recurring Template Pattern (CRTP) to allow compile-time polymorphism
 without virtual overhead, providing a clean, type-safe and modern C++ interface.

 The file includes:
 - `DisplayInterface`: a CRTP base class offering a `print()` method built on `std::format`.
 - `DisplayDevice`: a C++23 concept that enforces that a class implements the required
   `printString(uint8_t, uint8_t, std::string_view)` method with the correct signature.
 - `HasDisplayPrint`: a secondary concept to detect if a class supports the generic `print()` API.

 These constructs make it easier to build consistent and safe abstractions around display
 devices in embedded systems and improve error reporting during template instantiation.

 \note `DisplayDevice` is used directly to constrain `DisplayInterface` to valid implementations.

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

#include <cstdint>
#include <string_view>
#include <format>
#include <concepts>
#include <utility>


/**
  \brief Concept to verify a class implements a compatible print() method.
  \tparam ty Class to check for the concept.
  \details Checks for the presence of a `print(uint8_t, uint8_t, std::format_string<Args...>, Args&&...)` method.
 */
template<typename ty>
concept DisplayDevice = requires(ty d, uint8_t line, uint8_t pos, std::string_view text) {
   { d.printString(line, pos, text) } -> std::same_as<void>;
};


/**
  \brief CRTP interface for LCD display types.
  \tparam Derived The actual implementation class.
  \details Provides a formatted output interface to derived classes.
 */
   template<typename Derived>
   class DisplayInterface {
   public:
      /**
        \brief Write a formatted string to a display line and position.
        \tparam Args Format arguments.
        \param line Display line (0–3).
        \param position Start column (0–19).
        \param fmt Format string.
        \param args Format arguments.
       */
      template<typename... Args>
      void print(uint8_t line, uint8_t position, std::format_string<Args...> fmt, Args&&... args) {
         static_cast<Derived*>(this)->printString(line, position, std::format(fmt, std::forward<Args>(args)...));
         }
   }   ;   
   




/**
   \brief Concept to verify a class derives from DisplayInterface.
   \details Ensures that a type is derived from DisplayInterface, regardless of implementation details.
 */
template<typename ty>
concept ImplementsDisplayInterface = std::derived_from<ty, DisplayInterface<ty>>;
