// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Implementation of formatted output routines for WeatherAPI weather data.

\details
This file contains all logic for formatting and printing weather data types, as declared in WeatherOutput.h.
Output conventions follow structured, locale-independent patterns to aid diagnostics and reporting.
Each function handles its input robustly and provides useful output even for partially filled or incomplete data.

\warning
Do not mix business logic or non-output processing with these routines; keep formatting and data handling separate.

\see WeatherOutput.h (interface and documentation)

  \version 1.0
  \date    30.06.2025
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
#include "WeatherPrint.h"

#include <formatter_optional.h>  // now in Tools

#include <print>

namespace WeatherAPI {

void print(WeatherMeta const& meta) {
   std::println("Zeitzone: {} ({}), Offset: {}s, Höhe: {}m",
      meta.timezone, meta.timezone_abbreviation, meta.utc_offset_seconds, meta.elevation);
}

void print(WeatherCurrentExtended const& w) {
   std::println("Wetter aktuell ({:%d.%m.%Y %X})", w.timestamp);
   std::println("  Temperatur:   {:10.1f} °C", w.temperature_2m);
   std::println("  Luftfeuchte:  {:10.0f} %", w.relative_humidity_2m);
   std::println("  Luftdruck:    {:10.0f} hPa", w.surface_pressure);
   std::println("  Taupunkt:     {:10.1f} °C", w.dew_point_2m);
   std::println("  Niederschlag: {:10.1f} mm", w.precipitation);
   std::println("  Wind:         {:10.1f} km/h aus {}", w.windspeed_10m, wind_direction_text(w.winddirection_10m));
   std::println("  Böen:         {:10.1f} km/h", w.windgusts_10m);
   std::println("  UV-Index:     {:10.1f}  ({})", w.uv_index, describe_uv_index(w.uv_index));
   std::println("  Wolken:       {:10.0f} %, Tag: {}", w.cloudcover, w.is_day);
   std::println("  Wettercode:   {:>10}  ({:})", w.weather_code, describe_weather_code(w.weather_code));
   std::println("\n Zusammenfassung:\n{}", generate_weather_summary(w));
}

void print(WeatherCurrent const& wc) {
   std::println("Aktuell {:%d.%m.%Y %X}: {:4.1f} °C, Wind {:4.1f} km/h aus {:3.0f}° Code {:2}, Tag: {}",
      wc.timestamp, wc.temperature, wc.windspeed, wc.winddirection, wc.weathercode, wc.is_day);
}

void print(std::vector<WeatherDay> const& data) {
   for (const auto& d : data) {
      std::println("{}: {} {} ({:4.1f}°C / {:4.1f}°C), Sonne: {}, Regen:{:6.1f} mm, UV: {:4.1f} ({:<9}), Code: {:>2} ({})",
         d.date, d.sunrise, d.sunset, d.temp_min, d.temp_max, d.sunshine_duration, d.precipitation_mm,
         d.uv_index, describe_uv_index(d.uv_index), d.weather_code, describe_weather_code(d.weather_code));
   }
}

void print(std::vector<WeatherHour> const& data) {
   for (const auto& wh : data) {
      std::println("{:%d.%m.%Y %X}: Temp {:4.1f} °C, Taupunkt {:4.1f} °C, Wolken: {:3.0f} %, UV: {:3.1f} ({:<9}), Druck: {:4.0f} hPA, Tag: {:<5},  WCode: {:2} ({})",
         wh.timestamp, wh.temperature_2m, wh.dew_point_2m, wh.cloudcover, wh.uv_index, describe_uv_index(wh.uv_index), wh.surface_pressure,
         wh.is_day, wh.weather_code, describe_weather_code(wh.weather_code)
      );
   }
}

} // end of namespace WeatehrAPI