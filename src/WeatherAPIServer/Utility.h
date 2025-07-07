// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file 
 \brief Common utility functions and time handling tools for scheduling and timepoint alignment.

 \details
 This header provides reusable time-related and general-purpose helper functions used throughout
 the weather-fetching infrastructure and scheduling framework. The utilities aim to simplify
 operations related to chrono arithmetic, event planning, and clean code reuse.

 Key components include:
 - \ref NextStep: Computes the next aligned timepoint for a given step size
 - Potential date/time conversion helpers
 - Miscellaneous strongly typed, constexpr-based utilities
 - Small convenience wrappers to improve clarity and correctness in time-domain logic

 All functions are implemented using modern C++23 features such as:
 - chrono time zone support (`std::chrono::current_zone`)
 - strong type safety via concepts and static_assert
 - constexpr and compile-time validation where applicable

 \note This file avoids opening namespaces in the global scope and uses named clocks and types explicitly.
 \note Intended to be used throughout time-based coordination systems like schedulers, weather state machines, etc.

  \version 1.0
  \date    05.07.2025
  \author  Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see https://www.gnu.org/licenses/.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.

*/

#pragma once

#include <string>
#include <chrono>
#include <format>
#include <cassert> 

/**
 \brief Utility function to compute the next aligned timepoint for a given duration step.
 \details This function computes the next timepoint that aligns with a specified duration interval.
          It rounds the given timepoint up to the next multiple of the step (e.g., the next 15-minute mark).
          Local time zone is respected for correct alignment with human-readable schedules.

 \tparam timepoint_ty The type of time_point to compute (e.g., std::chrono::system_clock::time_point)
 \tparam duration_ty The type of duration step (e.g., std::chrono::minutes, std::chrono::hours)

 \param step The interval duration to align to (must be a positive integral duration)
 \param now  (Optional) The current timepoint to start from. Defaults to std::chrono::system_clock::now()

 \returns The next timepoint of type \c timepoint_ty aligned to the specified duration step.

 \pre \c step.count() > 0
 \note Only integral duration types are supported.
 \warning Will assert if a non-positive step is provided at runtime.

 \code{.cpp}
 // Example usage with 15-minute step and system_clock timepoint:
 auto next = NextStep<std::chrono::system_clock::time_point>(std::chrono::minutes{15});
 \endcode

 \code{.cpp}
 // Former implementation for fixed 15-minute steps:
 timepoint_ty NextQuarter(timepoint_ty const& tp) {
    auto t = std::chrono::floor<std::chrono::minutes>(tp);
    auto min = std::chrono::duration_cast<std::chrono::minutes>(t.time_since_epoch()).count();
    auto next = ((min / 15) + 1) * 15;
    return timepoint_ty {std::chrono::minutes{ next }};
 }
 \endcode
*/
template<typename timepoint_ty, typename duration_ty>
[[nodiscard]] timepoint_ty NextStep(duration_ty const& step, std::chrono::system_clock::time_point const& now = std::chrono::system_clock::now()) {
   static_assert(!std::chrono::treat_as_floating_point_v<typename duration_ty::rep>,
      "Only integral durations are supported.");
   assert(step.count() > 0 && "Step duration must be positive.");

   // Convert system time to local time for alignment
   auto local = std::chrono::current_zone()->to_local(now);
   auto tp = std::chrono::time_point_cast<std::chrono::seconds>(local);

   // Round down to last full step
   auto t = std::chrono::floor<duration_ty>(tp);
   auto since_epoch = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
   auto step_sec = std::chrono::duration_cast<std::chrono::seconds>(step).count();
   auto next = ((since_epoch / step_sec) + 1) * step_sec;

   return timepoint_ty(std::chrono::seconds{ next });
   }

/**
 \brief Converts any time_point to a local time_point in seconds precision.
 \details
 This function converts a time_point from any clock (e.g., std::chrono::system_clock) to a
 `std::chrono::local_time<std::chrono::seconds>` (or any user-defined type `timepoint_ty`)
 representing the corresponding wall clock time truncated to seconds.

 \tparam tp_ty         The clock type from which to convert (e.g., std::chrono::system_clock)
 \tparam timepoint_ty  The resulting type 
 \param now            The current time_point in UTC from the given clock
 \returns              A timepoint in seconds, converted to local time
 \note Uses std::chrono::current_zone()->to_local internally.
 \warning Only clocks compatible with std::chrono are allowed. Use only integral precision time.
*/
template<typename tp_ty, typename timepoint_ty> requires std::chrono::is_clock_v<typename tp_ty::clock>
[[nodiscard]] timepoint_ty convert_to_seconds(typename tp_ty::time_point const& now) {
   auto tp = std::chrono::current_zone()->to_local(now);
   return timepoint_ty{ std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()) };
   }

/**
 \brief Computes the duration in seconds until a future local timepoint.
 \details
 This function computes how many seconds remain until a scheduled event timepoint in local time.
 It uses the current local wall clock time and compares it to the target timepoint to calculate
 the remaining seconds.

 \tparam timepoint_ty The target timepoint type (e.g., systimes or local_time<seconds>, ...)
 \param next          The target future timepoint
 \returns             A std::chrono::seconds value representing the time remaining
 \note                If the result is negative, the timepoint lies in the past.
*/
template<typename timepoint_ty>
[[nodiscard]] std::chrono::seconds SecondsTo(timepoint_ty const& next) {
   auto local_now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
   auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(local_now);
   return next - now_sec;
   }

/**
   \brief Formats a time point into a human-readable timestamp string.
   \details Converts a given \c std::chrono::system_clock::time_point into a string formatted as \c DD.MM.YYYY HH:MM:SS,mmm,
            where \c mmm represents the millisecond component. The formatting is done using \c std::format and uses the system-local time zone.

   \param tp The time point to be formatted. The type must be \c std::chrono::system_clock::time_point.
   \returns A string representation of the time point in the format "dd.mm.yyyy HH:MM:SS,mmm".
   \note The millisecond precision is extracted separately, as the chrono formatting does not include milliseconds directly.
   \pre The time point \c tp must be a valid (non-default) \c system_clock::time_point.

   \example
   \code
   auto now = std::chrono::system_clock::now();
   std::string str = FormatTime(now);
   std::println("Current time: {}", str);
   \endcode
*/
[[nodiscard]] inline std::string FormatTime(std::chrono::system_clock::time_point const& tp) {
   //auto local = std::chrono::current_zone()->to_local(tp);
   auto const millis = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000);
   return std::format("{:%d.%m.%Y %X},{:03}", tp, millis);
   }
