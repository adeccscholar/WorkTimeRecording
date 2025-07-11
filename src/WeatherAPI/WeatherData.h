// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Core data structures and helpers for weather data exchange with Open-Meteo in the WeatherAPI library.

\details
This header defines the main value types, enumerations, and utility templates for the WeatherAPI shared library.
It forms the foundation for deserializing, processing, and reporting weather data received from the Open-Meteo API, leveraging robust JSON conversion and validation mechanisms provided by the BoostTools utilities.

**Structure overview:**
- **Data section**: Contains all core weather value types and enumerations used throughout the API, including `WeatherMeta`, `WeatherCurrent`, `WeatherCurrentExtended`, `WeatherDay`, `WeatherHour`, and `WeatherTime`.
- **Rule and text helpers**: Provides generic rule-based description utilities (e.g. for Beaufort scale, wind directions, UV-Index) and formatting logic.
- **Interface declarations**: Declares all exported formatting and description functions for weather values; implementations are in the corresponding `.cpp` file.

This design separates clean data representation, logic for value extraction/validation, and human-readable formatting, and is intended to be extended as Open-Meteo or domain requirements evolve.

**Integration with BoostTools:**
All types and helpers in this header are designed to work seamlessly with the traits- and policy-based JSON handling patterns from the BoostTools suite.

\warning
All exported functions and classes must use the `WEATHERAPI_API` macro for correct symbol visibility across platforms.
Do not add application logic or heavy dependencies to this header.

\todo Extend support for further weather phenomena as Open-Meteo expands.
\todo Integrate additional localization features as required.

\see WeatherData.cpp (implementations), src/BoostTools (utility layer)

  \version 1.0
  \date    30.07.2025
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

#include "WeatherAPI.h"

#include <optional>
#include <string>
#include <utility>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <format>
#include <chrono>
#include <print>
#include <type_traits>

namespace WeatherAPI {
// Eigene Zeittypen 

using timepoint_ty = std::chrono::local_time<std::chrono::seconds>;

using date_ty = std::chrono::year_month_day;
using time_ty = std::chrono::hh_mm_ss<std::chrono::seconds>;

/**
\brief Defines the type and resolution of weather data as provided by Open-Meteo.

\details
This enumeration indicates for which purpose or granularity a weather data structure is intended:
- \c TimeCheck: Data for time synchronization or verification.
- \c Current: Current weather values (simple).
- \c Current_Extended: Extended set of current weather fields.
- \c Daily: Daily aggregated weather values.
- \c Hourly: Hourly weather details.

\warning
The selection affects which data fields are present in corresponding structures.
*/
enum class WeatherResolution { TimeCheck, Current, Current_Extended, Daily, Hourly };

/**
\brief Represents a timestamped weather value.

\details
A minimal structure for associating weather data with a specific local timestamp.

\var WeatherAPI::WeatherTime::timestamp
Local timestamp for this weather entry (\c std::chrono::local_time<std::chrono::seconds>).
*/
struct WeatherTime {
   timepoint_ty          timestamp;
   };


/**
\brief Metadata describing the location and context of a weather request.

\details
Contains site and time zone information as returned by Open-Meteo, useful for localizing all other data.
*/
struct WeatherMeta {
   std::string timezone;                ///< Canonical time zone string (e.g., "Europe/Berlin").
   std::string timezone_abbreviation;   ///< Time zone abbreviation (e.g., "CEST", "GMT+2").
   int utc_offset_seconds;              ///< Offset to UTC in seconds.
   double elevation;                    ///< Elevation of the site in meters.
};


/**
\brief Holds a simple current weather report as returned by Open-Meteo.

\details
Covers the most commonly needed instantaneous weather fields. All values are optional and may be missing in API responses.

\warning
All fields except timestamp are optional and may be \c std::nullopt if not reported.
*/
struct WeatherCurrent {
   timepoint_ty          timestamp;     ///< Timestamp for the measurement.
   std::optional<double> temperature;   ///< Current temperature in degrees Celsius.
   std::optional<double> windspeed;     ///< Wind speed in km/h.
   std::optional<double> winddirection; ///< Wind direction in degrees.
   std::optional<int>    weathercode;   ///< Weather code as per Open-Meteo's WMO mapping.
   std::optional<bool>   is_day;        ///< Boolean flag indicating if the value is during daytime.
};

/**
\struct WeatherAPI::WeatherCurrentExtended
\brief Extended set of current weather fields, as returned with \c current=... requests.

\details
Adds more meteorological detail to the current weather snapshot, such as pressure, humidity, precipitation, radiation, and wind characteristics.
All fields except timestamp are optional.

\warning
All fields except timestamp are optional.
*/
struct WeatherCurrentExtended {
   timepoint_ty          timestamp;            ///< Timestamp for the measurement.
   std::optional<double> temperature_2m;       ///< Temperature at 2m height in degrees Celsius.
   std::optional<double> relative_humidity_2m; ///< Relative humidity at 2m height in percent.
   std::optional<double> dew_point_2m;         ///< Dew point temperature at 2m height in degrees Celsius.
   std::optional<double> precipitation;        ///< Total precipitation in mm.
   std::optional<double> rain;                 ///< Rain amount in mm.
   std::optional<double> snowfall;             ///< Snowfall in mm.
   std::optional<int>    weather_code;         ///< Weather code as per Open-Meteo's WMO mapping.
   std::optional<double> pressure_msl;         ///< Air pressure at mean sea level in hPa.
   std::optional<double> surface_pressure;     ///< Surface air pressure in hPa.
   std::optional<double> cloudcover;           ///< Total cloud cover in percent.
   std::optional<double> windspeed_10m;        ///< Wind speed at 10m in km/h.
   std::optional<double> windgusts_10m;        ///< Maximum wind gusts at 10m in km/h.
   std::optional<double> winddirection_10m;    ///< Wind direction at 10m in degrees.
   std::optional<double> uv_index;             ///< UV index (unitless).
   std::optional<double> shortwave_radiation;  ///< Shortwave radiation in W/m².
   std::optional<double> cape;                 ///< cape
   std::optional<bool>   is_day;               ///< Boolean flag indicating if the value is during daytime.
};

/**
\struct WeatherAPI::WeatherDay
\brief Aggregated weather data for a single day.

\details
Contains all daily fields as returned by Open-Meteo for the requested location, including temperatures, 
precipitation, wind, solar radiation, and sunrise/sunset times.

*/
struct WeatherDay {
   date_ty date;                ///< Calendar date of the report.(JJJJ-MM-TT)
   time_ty sunrise;             ///< Local sunrise time.
   time_ty sunset;              ///< Local sunrise time.
   double  temp_max;            ///< Maximum temperature of the day (°C).
   double  temp_min;            ///< Minimum temperature of the day (°C).
   double  precipitation_mm;    ///< Total daily precipitation (mm).
   int     weather_code;        ///< Weather code (WMO).
   double  windspeed_max;       ///< Maximum wind speed (km/h).
   double  uv_index;            ///< Maximum UV index.
   double  temp_mean;           ///< Mean daily temperature (°C).
   double  apparent_temp_max;   ///< Maximum apparent temperature (°C).
   double  apparent_temp_min;   ///< Minimum apparent temperature (°C).
   time_ty sunshine_duration;   ///< Total sunshine duration (hh:mm:ss).
   double  precipitation_hours; ///< Total hours with precipitation.
   double  windgusts_max;       ///< Maximum wind gusts (km/h).
   double  radiation_sum;       ///< Total solar radiation (W/m²).
   double  evapotranspiration;  ///< Total evapotranspiration (mm).
   double  rain_sum;            ///< Total rainfall (mm).
   double  snowfall_sum;        ///< Total snowfall (mm).
   int     wind_direction_deg;  ///< Mean wind direction (degrees).
};

/**
\struct WeatherAPI::WeatherHour
\brief Weather details for a single hour.

\details
All typical fields for high-resolution meteorological data are included.  
All values except timestamp are optional, as Open-Meteo may omit fields.

\warning
All fields except timestamp are optional.
*/
struct WeatherHour {
   timepoint_ty          timestamp;            ///< Timestamp for the measurement.
   std::optional<double> temperature_2m;       ///< Temperature at 2m height (°C).
   std::optional<double> relative_humidity_2m; ///< Relative humidity at 2m (%).
   std::optional<double> dew_point_2m;         ///< Dew point at 2m (°C).
   std::optional<double> apparent_temperature; ///< Apparent temperature (°C).
   std::optional<double> precipitation;        ///< Precipitation (mm).
   std::optional<double> rain;                 ///< Rain (mm).
   std::optional<double> showers;              ///< Showers (mm).
   std::optional<double> snowfall;             ///< Snowfall (mm).
   std::optional<int>    weather_code;         ///< Weather code.
   std::optional<double> pressure_msl;         ///< Pressure at mean sea level (hPa).
   std::optional<double> surface_pressure;     ///< Surface pressure (hPa).
   std::optional<double> cloudcover;           ///< Total cloud cover (%).
   std::optional<double> cloudcover_low;       ///< Low cloud cover (%).
   std::optional<double> cloudcover_mid;       ///< Mid cloud cover (%).
   std::optional<double> cloudcover_high;      ///< High cloud cover (%).
   std::optional<double> shortwave_radiation;  ///< Shortwave radiation (W/m²).
   std::optional<double> direct_radiation;     ///< Direct solar radiation (W/m²).
   std::optional<double> diffuse_radiation;    ///< Diffuse solar radiation (W/m²).
   std::optional<double> windspeed_10m;        ///< Wind speed at 10m (km/h).
   std::optional<double> windgusts_10m;        ///< Wind gusts at 10m (km/h).
   std::optional<double> winddirection_10m;    ///< Wind direction at 10m (degrees).
   std::optional<double> uv_index;             ///< UV index.
   std::optional<double> cape;                 ///< cape value.
   std::optional<bool>   is_day;               ///< Boolean flag indicating if value is during daytime.
};

/**
\typedef WeatherAPI::RuleSet
\brief Typedef für Regeln zur Beschreibung von Wertebereichen.

\details
A generic rule-based description table.  
Each rule is a tuple: lower bound, upper bound, and a pair of text descriptions (German, English).

\tparam ty Type of value to which the rules apply (e.g. double for UV index, wind speed).
\see apply_rules
*/
template <typename ty>
using RuleSet = std::vector<std::tuple<ty, ty, std::pair<std::string, std::string>>>;

/**
\brief Applies rule-based textual descriptions to a value.

\details
Iterates over the given rules and outputs the corresponding message if the value falls within the rule’s interval.
Supports multi-language descriptions (German/English).

\tparam ty The value type.

\param out Output stream to write the description to.
\param value Optional value to describe.
\param rules Vector of rules as (from, to, (German, English)).
\param use_german Whether to use German (\c true) or English (\c false) description.

\warning
If no rule matches, nothing is output.

\code
RuleSet<double> uv_rules = { {0, 3, {"niedrig", "low"}}, {3, 6, {"moderat", "moderate"}}, ... };
std::ostringstream oss;
apply_rules(oss, uv_index, uv_rules, true); // Outputs German description
\endcode
*/
template <typename ty>
void apply_rules(std::ostringstream& out, std::optional<ty> const& value, RuleSet<ty> const& rules, bool use_german = true) {
   if (!value) return;
   for (const auto& [from, to, msg_pair] : rules) {
      if (*value >= from && *value < to) {
         out << (use_german ? msg_pair.first : msg_pair.second) << ' ';
         break;
         }
      }
   }

WEATHERAPI_API std::string wind_direction_text(std::optional<double> deg);
WEATHERAPI_API std::pair<std::string, std::string> wind_beaufort_text(double speed_kmh);
WEATHERAPI_API std::string describe_uv_index(std::optional<double> uv_index, bool German = true);
WEATHERAPI_API std::string describe_weather_code(std::optional<int> code);
WEATHERAPI_API std::string generate_weather_summary(WeatherCurrentExtended const& wh, bool german = true);

} // end of namespace WeatherAPI