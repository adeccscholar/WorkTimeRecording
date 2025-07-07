/**
\file
\brief High-level output functions for WeatherAPI weather data types.

\details
This header declares all exportable print and reporting routines for WeatherAPI types.
It enables applications and services to easily output weather metadata, current conditions, and time series in a structured, human-readable format.
All functions use the core data structures from \c WeatherData.h and are designed for extensibility and localization.

**Key areas:**
- Output of site and timezone metadata.
- Reporting for single (current) and series (daily/hourly) weather datasets.
- Consistent formatting conventions for tabular and detail output.

All exported functions are marked with \c WEATHERAPI_API for correct symbol visibility in shared library use.

\warning
These output routines are intended for diagnostic and reporting purposes; for persistent data export, consider structured formats or dedicated exporters.

\todo Add support for localized output and further formatting customization.
\todo Integrate error reporting if input data is incomplete or inconsistent.

\see WeatherPrint.cpp (implementations), WeatherData.h (data types)

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

#include <vector>

namespace WeatherAPI {

/**
\name Output and Reporting Functions
\brief Functions for formatted output of weather data types.
\details
This group contains all high-level reporting and print functions for WeatherAPI types.
All output routines format their data for human-readable console or log output, supporting both metadata and time series.
*/
// \{

/**
\brief Prints site and timezone metadata to standard output.
\param meta The metadata structure to print.
\see WeatherMeta
*/
WEATHERAPI_API void print(WeatherMeta const& meta);

/**
\brief Prints all fields of an extended current weather data record.
\param w The extended weather data to print.
\see WeatherCurrentExtended
*/
WEATHERAPI_API void print(WeatherCurrentExtended const& w);

/**
\brief Prints a compact summary of a simple current weather record.
\param wc The current weather data to print.
\see WeatherCurrent
*/
WEATHERAPI_API void print(WeatherCurrent const& wc);

/**
\brief Prints a table or list of daily weather summaries.
\param data The vector of daily weather data.
\see WeatherDay
*/
WEATHERAPI_API void print(std::vector<WeatherDay> const& data);

/**
\brief Prints a table or list of hourly weather values.
\param data The vector of hourly weather data.
\see WeatherHour
*/
WEATHERAPI_API void print(std::vector<WeatherHour> const& data);

/// \}

} // end of namespace WeatherAPI
