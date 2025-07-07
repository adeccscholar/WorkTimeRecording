// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief High-level weather data reading and Open-Meteo parsing for the WeatherAPI library.

\details
This header defines the main import/export interface, parsing routines, and JSON-mapping functions for reading weather data from the Open-Meteo API.
It builds upon the data structures defined in `WeatherData.h` and the traits/policy-based JSON and HTTP tools from BoostTools.

**Key areas:**
- **API construction**: Helpers for constructing URLs and retrieving the Open-Meteo endpoint.
- **JSON to struct mapping**: Functions for robust, type-safe mapping of JSON objects to weather data types using the from_json tag dispatch pattern.
- **Error handling**: Dedicated function to detect and handle API errors in JSON responses.
- **Generic parsing**: Templated routines to parse arbitrary weather data types or series from JSON using reusable control data.
- **Field control**: Generic field descriptors and control data for flexible mapping and transformation of series data.

The design allows new data fields or weather types to be integrated by extending the control structures and from_json mappings, with all parsing and validation routed through robust, reusable infrastructure.

\warning
Always ensure that all new exported symbols are marked with `WEATHERAPI_API` and that all parsing routines are validated against current Open-Meteo API responses.

\todo Extend the API to support additional Open-Meteo endpoints (e.g. historical, air quality).
\todo Integrate localization for descriptions and errors.
\todo Consider advanced error recovery for partial or malformed responses.

\see WeatherReader.cpp (implementations), WeatherData.h (data structures), BoostTools (utility layer)

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
#pragma once
#include "WeatherAPI.h"

#include "WeatherData.h"

#include <BoostJsonTools.h>      // now in BoostTools
#include <BoostBeastTools.h>     // now in BoostTools
#include <BoostJsonFrom.h>

#include <boost/system/system_error.hpp>  // falls noch nicht inkludiert

#include <string>
#include <vector>
#include <format>
#include <tuple>
#include <stdexcept>
#include <span>
#include <ranges>

using namespace std::string_literals;

// https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&timezone=auto&current=temperature_2m
// https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&timezone=auto&current=temperature_2m,relative_humidity_2m,dew_point_2m,precipitation,rain,snowfall,weathercode,pressure_msl,surface_pressure,cloudcover,windspeed_10m,windgusts_10m,winddirection_10m,uv_index,shortwave_radiation,is_day

// visibility (m) ,shower (mm), precipitation_probability (%) shortwave_radiation (W/m²)
// apparent_temperature (°C), cloud_cover (%), cape (J/kg), sunshine_duration (sec)

namespace WeatherAPI {

/**
\name WeatherAPI: High-level Parsing and Mapping
\brief All main API, mapping, and control functions for Open-Meteo weather data.
\details
This group contains the primary interface for reading, parsing, and mapping Open-Meteo weather data in the WeatherAPI module.
*/
// \{

/**
\brief Returns the Open-Meteo API endpoint URL.
\return The base URL as string.
*/
WEATHERAPI_API std::string GetServer();

/**
\brief Constructs the query URL for Open-Meteo API for the given resolution, location, and forecast days.
\param resolution Type of weather data (see WeatherResolution).
\param latitude Latitude of the location (degrees).
\param longitude Longitude of the location (degrees).
\param forecast_days Number of forecast days to request.
\return The complete Open-Meteo API URL as string.
\warning
Make sure latitude/longitude are within valid ranges, and forecast_days does not exceed API limits.

\todo Support for additional Open-Meteo query parameters (e.g. air quality, hourly variables).
*/
WEATHERAPI_API std::string GetUrl(WeatherResolution resolution, double latitude, double longitude, int forecast_days);

/**
\name from_json Mapping Functions
\brief Tag-dispatch based pattern for mapping JSON to WeatherAPI data types.
\details
These functions implement a robust, extensible mapping scheme using tag dispatching and SFINAE (see BoostTools).
Each supported type gets its own `from_json` overload, enabling type-safe, decoupled, and easily extendable deserialization logic.
This pattern separates mapping logic from the data structures, minimizes coupling, and allows new types to be integrated with minimal friction.

**Pattern Note:**  
The `from_json` pattern is inspired by modern C++ serialization libraries (e.g. nlohmann::json) but uses a tag type and ADL for extensibility without macros or inheritance.

\see BoostTools::from_json_tag
*/
/// \{

/**
\brief Maps a JSON object to a \c WeatherTime structure using tag dispatch.

\details
This function implements the tag-dispatch-based `from_json` pattern for deserializing a JSON object into a \c WeatherTime struct.
It is automatically discovered and invoked by the generic mapping routines in BoostTools (via ADL and \c from_json_tag).
All required fields are extracted using robust, type-safe utilities.

\param[out] tc  Reference to the \c WeatherTime struct to fill.
\param[in]  obj The Boost.JSON object representing the weather time data.
\param[in]  tag Tag type to enable tag-dispatch and ADL (see BoostTools::from_json_tag).
\throws std::runtime_error if required fields are missing or invalid.
\see BoostTools::from_json, WeatherTime
*/
WEATHERAPI_API void from_json(WeatherTime& tc, boost::json::object const& obj, boost_tools::from_json_tag);

/**
\brief Maps a JSON object to a \c WeatherMeta structure using tag dispatch.

\details
Implements the tag-dispatch-based `from_json` pattern for deserializing JSON metadata into a \c WeatherMeta struct.
Fields such as timezone, offset, and elevation are mapped directly using type-safe extraction helpers.

\param[out] meta Reference to the \c WeatherMeta struct to fill.
\param[in]  obj  The Boost.JSON object representing the weather metadata.
\param[in]  tag  Tag type for dispatching (see BoostTools::from_json_tag).
\throws std::runtime_error if required fields are missing or invalid.
\see BoostTools::from_json, WeatherMeta
*/
WEATHERAPI_API void from_json(WeatherMeta& meta, boost::json::object const& obj, boost_tools::from_json_tag);

/**
\brief Maps a JSON object to a \c WeatherCurrent structure using tag dispatch.

\details
Implements the tag-dispatch-based `from_json` pattern for converting a JSON object to a \c WeatherCurrent struct,
extracting timestamp, temperature, wind, weather code, and daytime flag.

\param[out] wc   Reference to the \c WeatherCurrent struct to fill.
\param[in]  obj  The Boost.JSON object representing the current weather data.
\param[in]  tag  Tag type for pattern-based dispatch (see BoostTools::from_json_tag).
\throws std::runtime_error if required fields are missing or invalid.
\see BoostTools::from_json, WeatherCurrent
*/
WEATHERAPI_API void from_json(WeatherCurrent& wc, boost::json::object const& obj, boost_tools::from_json_tag);

/**
\brief Maps a JSON object to a \c WeatherCurrentExtended structure using tag dispatch.

\details
Implements the tag-dispatch-based `from_json` pattern for deserializing detailed current weather data into a \c WeatherCurrentExtended struct.
All available fields, such as temperatures, humidity, precipitation, radiation, and wind, are mapped using robust, type-safe field extraction.

\param[out] wce  Reference to the \c WeatherCurrentExtended struct to fill.
\param[in]  obj  The Boost.JSON object containing the extended current weather data.
\param[in]  tag  Tag type for enabling tag dispatch (see BoostTools::from_json_tag).
\throws std::runtime_error if required fields are missing or invalid.
\see BoostTools::from_json, WeatherCurrentExtended
*/
WEATHERAPI_API void from_json(WeatherCurrentExtended& wce, boost::json::object const& obj, boost_tools::from_json_tag);

/// \}

/**
\brief Throws a runtime error if the JSON response contains an API error object.
\param json_response The parsed JSON response.
\throw std::runtime_error on API-reported error.
*/
WEATHERAPI_API void check_for_api_error(boost::json::object const& json_response);

/**
\brief Parses a single weather data element from a JSON string.

\details
If an element is specified, extracts the corresponding sub-object before parsing.

\tparam ty Target type (must be mappable via from_json).
\param json_str Raw JSON string.
\param element Name of the sub-element (optional).

\return Parsed value of type ty.

\throw std::runtime_error on error, missing field, or invalid JSON.

\code
WeatherCurrent current = parse<WeatherCurrent>(json_string, "current");
\endcode
*/
template <typename ty>
ty parse(std::string const& json_str, std::string const& element = ""s) {
   const auto json_obj = [&]() {
      if (element.size() > 0)
         return boost_tools::extract_json_object(std::string_view{ json_str.data(), json_str.size() }, element);
      else
         return boost_tools::extract_json_object(std::string_view{ json_str.data(), json_str.size() });
      }();

   check_for_api_error(json_obj);
   return boost_tools::from_json<ty>(json_obj);
}

/**
\brief Function type for mapping a field array to a struct field.
\tparam ty The struct type being filled.
*/
template <typename ty>
using control_func = std::function<void(ty&, boost::json::array const&, std::size_t)>;

/**
\brief Span of field mapping functions for series parsing.
\tparam ty The struct type being filled.
*/
template <typename ty>
using control_data = std::span<std::tuple<std::string_view, control_func<ty>>>;

/// List of field mappings for WeatherDay series parsing.
WEATHERAPI_API extern std::vector<std::tuple<std::string_view, control_func<WeatherDay>>> weather_day_fields;

/// List of field mappings for WeatherHour series parsing.
WEATHERAPI_API extern std::vector<std::tuple<std::string_view, control_func<WeatherHour>>> weather_hour_fields;

/**
\brief Parses a series of weather data objects from JSON arrays using control data.

\details
Each entry in control_data links a field name to a function that fills the corresponding struct field.
The function iterates over all series arrays and builds a vector of structs, handling per-field and per-index errors robustly.

\tparam struct_ty The target struct type.
\param json_str Raw JSON string.
\param root_key Name of the root object key containing the series.
\param cntrl Array of control_data describing all fields and mapping logic.
\return Vector of parsed struct_ty entries.
\throw std::runtime_error on error, missing fields, or mismatched sizes.
\warning
Control data must contain at least the "time" field.
*/
template <typename struct_ty>
std::vector<struct_ty> parse_series(std::string const& json_str, std::string const& root_key, control_data<struct_ty> const& cntrl) {
   boost::json::object const& root = boost_tools::extract_json_object(json_str, root_key);
   if (cntrl.empty())
      throw std::runtime_error("control_data must contain at least the time field");

   auto const& [first_field, first_apply] = cntrl[0];
   boost::json::array const& first_arr = root.at(first_field).as_array();

   std::vector<struct_ty> result;
   result.reserve(first_arr.size());

   for (std::size_t i = 0; i < first_arr.size(); ++i) {
      struct_ty entry{}; // initial leer
      try {
         first_apply(entry, first_arr, i);
         }
      catch (std::exception const& ex) {
         throw std::runtime_error(std::format("Error in key field '{}', index {}: {}", first_field, i, ex.what()));
         }
      result.emplace_back(std::move(entry));
      }
   // ---------------------------------------------------------------------
   for (auto const& [field_name, apply] : cntrl | std::views::drop(1)) {
      auto it = root.find(field_name);
      if (it == root.end()) continue;   // potentiell exception

      boost::json::array const& arr = it->value().as_array();
      for (std::size_t i = 0; i < std::min(result.size(), arr.size()); ++i) {
         try {
            apply(result[i], arr, i);
            }
         catch (std::exception const& ex) {
            throw std::runtime_error(std::format("Error in field '{}', index {}: {}", field_name, i, ex.what()));
            }
         }
      }

   return result;
   }

/// \}

} // end of namespace WeatherAPI
