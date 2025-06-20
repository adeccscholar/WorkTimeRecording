﻿// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Simple logging helpers for traceable CORBA-based applications.

  \details
  This file defines generic formatting-based logging utilities for different
  log levels – such as state output, trace-level debugging and error reporting.
  Logging uses `std::println` and `std::format_string`, available since C++23.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.

  \version 1.0
  \date    31.05.2025
*/

#pragma once
#include <iostream>
#include <print>


/// \brief Controls verbosity of \ref log_trace output.
const constinit int TraceLevel = 5;

/**
  \brief Logs a message to `std::cout` using format syntax.

  \tparam Args Variadic template arguments for format placeholders.
  \param fmt Format string (must match number and type of arguments).
  \param args Arguments to be formatted and inserted into the output.

  \note Equivalent to an unconditional info/state log.
*/
template<typename... Args>
void log_state(std::format_string<Args...> fmt, Args&&... args) {
   std::println(std::cout, fmt, std::forward<Args>(args)...);
   }

/**
  \brief Logs a message to `std::cout` conditionally, based on a trace level.

  \tparam Level Integer trace level (compile-time constant).
  \tparam Args Format arguments.
  \param fmt Format string for the output message.
  \param args Arguments matching the format string.

  \note The message is only printed if `Level < TraceLevel`.
 */
template<int Level, typename... Args>
void log_trace(std::format_string<Args...> fmt, Args&&... args) {
   if constexpr (Level < TraceLevel) std::println(std::cout, fmt, std::forward<Args>(args)...);
   }

/**
  \brief Logs an error message to `std::cerr`.

  \tparam Args Format arguments.
  \param fmt Format string describing the error.
  \param args Arguments to be formatted.

  \note This is typically used for errors, exceptions, and critical failures.
 */
template<typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
   std::println(std::cerr, fmt, std::forward<Args>(args)...);
   }

/**
  \brief Logs a debug message to `std::cerr`, only active in Debug builds.

  \tparam Args Format arguments.
  \param fmt Format string for the debug message.
  \param args Arguments to be formatted.

  \note This function is disabled in release builds (`NDEBUG` is defined).
 */
template<typename... Args>
void log_debug(std::format_string<Args...> fmt, Args&&... args) {
#ifndef NDEBUG
   std::println(std::cerr, "[DEBUG] {}", std::vformat(fmt.get(), std::make_format_args(args...)));
#endif
   }