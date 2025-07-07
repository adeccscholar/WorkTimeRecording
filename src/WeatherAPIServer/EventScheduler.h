// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
   \file 
   \brief Declaration of the Scheduler class for timed event triggering.
   \details
   The Scheduler manages a queue of timed events and provides two interfaces:
   - \ref waitNextEvent: blocking wait with condition variable
   - \ref pollEvent: non-blocking polling mechanism

   This allows decoupling of state machine event triggering and time-based scheduling.
   
   \note Thread-safe for concurrent scheduling and wait/polling access.
   \note Optimized for minimal locking and wake-up notification control.
   
   \note
   - waitNextEvent uses a std::unique_lock to allow timed waits and integration with std::condition_variable.
   - Schedule uses std::lock_guard to perform short, thread-safe inserts into the queue.
   - Both use the same mutex and can operate safely in parallel from different threads.
   - The mutex must never be acquired twice from the same thread (std::mutex is non-reentrant).

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

  \note This file is part of the adecc Scholar project - Free educational materials for modern C++.

*/

#pragma once

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <functional>

#include <print>

/// namespace to separate the Scheduler and used types 
namespace TimedEvents {

/// internal time type as local time with seconds precision for the events
using timepoint_ty = std::chrono::local_time<std::chrono::seconds>;

/**
\class Scheduler
\brief Manages time-based execution of scheduled events
\details Events are processed when their time is reached. Wakeup callback can be registered for early triggers.
*/
class Scheduler {
public:

   /**
     \brief Represents a scheduled trigger with execution time and callback
   */
   struct Event {
      timepoint_ty when; //!< local timestamp with seconds precision
      std::function<void()> trigger;  //!< function to be called when event fires

      /**
        \brief Comparator for priority queue
        \details Earlier timepoints have higher priority
        \param rhs Other event to compare
        \returns Strong ordering result between timepoints
      */
      auto operator <=> (Event const& rhs) const {
         return when <=> rhs.when;
         }

      };

private:
   std::mutex mutex;                    //!< protects queue and wakeup handler
   std::condition_variable condition;   //!< used for blocking wait
   std::priority_queue<Event, std::vector<Event>, std::greater<>> queue; //!< min-heap for scheduled events
   std::function<void()> wakeup; //!< optional wakeup callback when front changes

public:
   /**
    \brief Adds a new event to the scheduler
    \param ev Event with time and trigger
    \details If the event becomes the new earliest entry, the wakeup handler is triggered
    */
   void addEvent(Event&& ev) {
      bool notify = false;
         {
         std::lock_guard lock(mutex);
         notify = queue.empty() || ev.when < queue.top().when;
         queue.emplace(std::move(ev));
         }

      // Notify outside the lock to prevent deadlock
      if (notify && wakeup) {
         std::thread([fn = wakeup]() {
               fn(); // call wakeup in independent thread
               }).detach();
         }
      condition.notify_one();
      }

   void clearEvents() {
      bool notify = false;
         {
         std::lock_guard lock(mutex);
         notify = !queue.empty();
         while (!queue.empty()) {
            queue.pop();
            }
         }
         // Notify outside the lock to prevent deadlock
         if (notify && wakeup) {
            std::thread([fn = wakeup]() {
               fn(); // call wakeup in independent thread
               }).detach();
            }
         condition.notify_one();
      }

   /**
   \brief Waits for the next event if due
   \param running external stop condition
   \returns due event or std::nullopt if stopped
   */
   std::optional<Event> waitNextEvent(std::atomic_bool& running) {
      std::unique_lock lock(mutex);
      if(running) {
         if (!queue.empty()) {
            auto local_now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
            auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(local_now);
            auto const& next = queue.top();
            if (now_sec >= next.when) {
               auto ev = std::move(queue.top());
               queue.pop();
               return ev;
               }

            // Compute precise wait time until next event
            auto wait_until_tp = std::chrono::system_clock::now() + (next.when.time_since_epoch() - now_sec.time_since_epoch());
            condition.wait_until(lock, wait_until_tp, [&] { return !running; });
            }
         else {
            condition.wait(lock, [&] {  return !running; }); // Infinite wait, interrupted by running and notify_one()
            }
         }
      return std::nullopt;
      }

   /**
     \brief Returns the timestamp of the next scheduled event, if any.
     \returns std::optional<timepoint_ty> with the next event time, or std::nullopt if queue is empty.
   */
   std::optional<timepoint_ty> peekNextEventTime() {
      std::lock_guard lock(mutex);
      if (!queue.empty()) {
         return queue.top().when;
         }
      return std::nullopt;
      }

   /**
     \brief Non-blocking check for due events
     \param running external stop condition
     \returns due event or std::nullopt
     \note Can be used when waiting is handled externally
   */
   std::optional<Event> pollEvent(std::atomic_bool& running) {
      using namespace std::chrono;
      std::lock_guard  lock(mutex);
      if (!queue.empty()) [[likely]] {
         auto local_now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
         auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(local_now);

         auto const& next = queue.top();
         if (now_sec >= next.when) [[unlikely]] {
            auto ev = std::move(queue.top());
            queue.pop();
            return ev;
            }
         else return std::nullopt;
         }
      return std::nullopt;
      }

   /*!
   \brief Sets the optional wakeup trigger when a new earliest event is scheduled
   \param fn callable to be executed
   */
   void setWakeup(std::function<void()>&& fn) {
      std::lock_guard lock(mutex);
      wakeup = std::move(fn);
   }
};

} // end of namespace TimedEvents
