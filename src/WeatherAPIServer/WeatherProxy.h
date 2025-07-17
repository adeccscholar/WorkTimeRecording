// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
   \file 
   \brief Weather data interface with internal caching and thread-safe access
   \details
   This header defines the \c WeatherServer class, which encapsulates the logic to
   fetch and cache weather information for a fixed geographical location. It provides
   methods to access the most recent weather values in a thread-safe manner.

   The class performs two types of data retrieval:
   - Daily weather data (e.g., sunrise/sunset), fetched once per day at local midnight
   - Current weather data (e.g., temperature, humidity, wind), fetched periodically
     or on demand when a timestamp change is detected

   The class is optimized for safe concurrent access:
   - Writers (fetch methods) acquire a \c std::unique_lock on a \c std::shared_timed_mutex
   - Readers (e.g., \c GetWeatherData) acquire a \c std::shared_lock, allowing concurrent access
   - All locks are time-constrained (\c try_lock_for) to avoid blocking logic in the main control flow

   This component acts as a bridge between external APIs and internal state-based logic,
   such as a finite state machine that coordinates weather retrieval and scheduling.

   \note This module assumes a local time zone context for day transitions and sunrise/sunset data.
   \warning All API communication is assumed to succeed unless a parse or connection exception occurs.
   \todo Extend support for dynamic location if needed in future scenarios

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
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.

*/

#pragma once

#include <WeatherData.h>
#include <WeatherReader.h>
#include <WeatherPrint.h>

#include <chrono>
#include <string>
#include <optional>
#include <shared_mutex>
#include <print>
#include <format>
#include <system_error>

class WeatherProxy {
public:
   using timepoint_ty = std::chrono::local_time<std::chrono::seconds>;

   struct WeatherData {
      std::optional<WeatherAPI::time_ty> sunrise;
      std::optional<WeatherAPI::time_ty> sunset;
      std::optional<double> temperature;
      std::optional<double> pressure;
      std::optional<double> humidity;
      std::optional<double> precipitation;
      std::optional<double> windspeed;
      std::optional<double> winddirection;
      std::optional<double> cloudcover;
      std::optional<double> uv_index;
      std::optional<int>    weathercode;
      std::optional<std::string> summary;

      };
private:
   const double latitude  = 52.5366923;  ///< Fixed latitude for weather data
   const double longitude = 13.2027663;  ///< Fixed longitude for weather data

   HttpRequest server;

   std::optional<WeatherAPI::date_ty>       last_day{};     ///< Last fetched calendar day
   std::optional<WeatherAPI::timepoint_ty>  last_weather{}; ///< Last fetched weather timestamp

   std::shared_timed_mutex  mutex;
   WeatherData weather_data;
   
public:
   /**
     \brief Constructor
     \details Initializes internal server using WeatherAPI helper.
   */
   WeatherProxy() : server(WeatherAPI::GetServer()), last_day{}, last_weather{} { }

   /**
     \brief Fetch daily summary values (sunrise, sunset, etc.)
     \returns true if fetch and update succeeded
     \note Data is only updated if the calendar day has changed.
   */
   bool FetchDailyData() {
      std::println("[WeatherProxy] Fetching daily data...");
      // Lokales Datum (z. B. für Mitternacht im lokalen Zeitbereich)
      auto now_local = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
      auto today_days = std::chrono::floor<std::chrono::days>(now_local);
      auto today = std::chrono::year_month_day{ today_days };
      if (!last_day || *last_day < today) {
         auto json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Daily, latitude, longitude, 1));
         const auto daily_data = WeatherAPI::parse_series<WeatherAPI::WeatherDay>(json, "daily", WeatherAPI::weather_day_fields);
         if (daily_data.size() == 0) return false; // later exception
         if (daily_data[0].date == today) {
            try {
               std::unique_lock lock(mutex, std::defer_lock);
               if (lock.try_lock_for(std::chrono::milliseconds(100))) {
                  weather_data.sunrise = daily_data[0].sunrise;
                  weather_data.sunset  = daily_data[0].sunset;
                  std::println("[WeatherProxy] Successfully fetched daily data. {:%d.%m.%Y} {:%X}", today, *weather_data.sunrise);
                  last_day = today;
                  return true;
                  }
               else {
                  std::println("[WeatherProxy] Timeout: Failed to acquire lock for fetching daily data.");
                  return false;
                  }
               }
            catch (std::system_error const& ex) {
               std::println("[WeatherProxy] Failed to acquire lock for fetching daily data:\n{}", ex.what());
               return false;
               }
            }
         else {
            std::println("[WeatherProxy] Daily data unchanged, no fetch required.");
            return false;
            }
         }
      
      return false;
      }

   /**
     \brief Fetch current weather readings (temperature, pressure, etc.)
     \returns true if update was successful and new data was stored
     \note Skips update if timestamp is unchanged.
   */
   bool FetchCurrentData() {
      try {
         std::println("[WeatherProxy] Fetching current data...");
         auto json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::TimeCheck, latitude, longitude, 1));
         const auto check = WeatherAPI::parse<WeatherAPI::WeatherTime>(json, "current");
         if (!last_weather || *last_weather < check.timestamp) {
            json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Current_Extended, latitude, longitude, 1));
            const auto cur_extended_data = WeatherAPI::parse<WeatherAPI::WeatherCurrentExtended>(json, "current");
            try {
               std::unique_lock lock(mutex, std::defer_lock);
               if(lock.try_lock_for(std::chrono::milliseconds(100))) {
                  last_weather               = cur_extended_data.timestamp;
                  weather_data.temperature   = cur_extended_data.temperature_2m;
                  weather_data.pressure      = cur_extended_data.surface_pressure;
                  weather_data.humidity      = cur_extended_data.relative_humidity_2m;
                  weather_data.precipitation = cur_extended_data.precipitation;
                  weather_data.windspeed     = cur_extended_data.windspeed_10m;
                  weather_data.winddirection = cur_extended_data.winddirection_10m;
                  weather_data.cloudcover    = cur_extended_data.cloudcover;
                  weather_data.uv_index      = cur_extended_data.uv_index;
                  weather_data.weathercode   = cur_extended_data.weather_code;
                  weather_data.summary       = WeatherAPI::generate_weather_summary(cur_extended_data);
                  std::println("[WeatherProxy] Successfully fetched current data. {:%d.%m.%Y %X} {} °C", *last_weather, *weather_data.temperature);
                  return true;
                  }
               else {
                  std::println("[WeatherProxy] Timeout: Failed to acquire lock for fetching current data.");
                  return false;
                  }
               }
            catch (std::system_error const& ex) {
               std::println("[WeatherProxy] Failed to acquire lock for fetching current data:\n{}", ex.what());
               return false;
               }
            }
         else {
            std::println("[WeatherProxy] Current data unchanged, no fetch required.");
            return false;
            }
         }
      catch (const std::exception& ex) {
         std::println("[WeatherProxy] HTTP/Parse error: {}", ex.what());
         return false;
         }
      }


   /**
     \brief Thread-safe read access to current weather data
     \returns WeatherData copy if lock could be acquired, otherwise std::nullopt
   */
   std::optional<WeatherData> GetWeatherData() {
      //if (mutex.try_lock_shared_for(std::chrono::milliseconds(100))) {
      //   std::shared_lock lock(mutex, std::adopt_lock);
      std::shared_lock lock(mutex, std::defer_lock);
      if (lock.try_lock_for(std::chrono::milliseconds(100))) {
         return weather_data;
         }
      else {
         return std::nullopt;
         }
      }
   };

