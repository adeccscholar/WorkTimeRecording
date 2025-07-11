// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Implementations for core weather data helpers and description functions in the WeatherAPI library.

\details
This source file provides all logic for formatting, description, and textual conversion of weather data as defined in `WeatherData.h`.
It implements functions for wind direction, Beaufort scale, weather code mapping, UV index classification, and rule-based summaries,
using the data types, templates, and interfaces declared in the header.

The file may make extensive use of C++ standard library features, as well as robust conversion and validation patterns from the BoostTools module,
to ensure all logic is type-safe, well-structured, and locale-independent.

**Design Note:**
- Implementation of functions is kept separate from the data structure definitions to allow for clean interface headers and shared library usability.
- Each function is documented in detail in this file, focusing on edge cases, internationalization, and integration with the overall API.

\warning
If you add new export functions, remember to update both header and file documentation and to use the correct `WEATHERAPI_API` macro.

\see WeatherData.h (declarations and type definitions)

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

#include "WeatherData.h"


#include <string>
#include <optional>
#include <memory>
#include <unordered_map>

namespace WeatherAPI {

/**
\brief Converts wind direction in degrees to a cardinal text description.
\param deg Wind direction in degrees (0-360).
\return Text description (e.g., "Nord", "North").
\see WeatherHour, WeatherDay
*/
std::string wind_direction_text(std::optional<double> deg) {
   static constexpr const char* directions[] = {
       "N", "NNO", "NO", "ONO", "O", "OSO", "SO", "SSO",
       "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
      };
   return deg ? directions[static_cast<int>((*deg + 11.25) / 22.5) % 16] : "n/a";
   }

/**
\brief Maps wind speed to Beaufort scale with textual description.
\param speed_kmh Wind speed in km/h.
\return Pair of German and English Beaufort descriptions.
*/
std::pair<std::string, std::string> wind_beaufort_text(double speed_kmh) {
   static const RuleSet<double> beaufort = {
       {  0.0,   1.0, {"Windstille", "Calm"}},
       {  1.0,   5.0, {"Leiser Zug", "Light air"}},
       {  5.0,  11.0, {"Leichte Brise", "Light breeze"}},
       { 11.0,  19.0, {"Schwache Brise", "Gentle breeze"}},
       { 19.0,  28.0, {"Mäßige Brise", "Moderate breeze"}},
       { 28.0,  38.0, {"Frische Brise", "Fresh breeze"}},
       { 38.0,  49.0, {"Starker Wind", "Strong breeze"}},
       { 49.0,  61.0, {"Steifer Wind", "Near gale"}},
       { 61.0,  74.0, {"Stürmischer Wind", "Gale"}},
       { 74.0,  88.0, {"Sturm", "Severe gale"}},
       { 88.0, 102.0, {"Schwerer Sturm", "Storm"}},
       {102.0, 117.0, {"Orkanartiger Sturm", "Violent storm"}},
       {117.0, std::numeric_limits<double>::max(), {"Orkan", "Hurricane"}},
      };

   for (const auto& [from, to, msg] : beaufort) {
      if (speed_kmh >= from && speed_kmh < to) {
         return msg;
         }
      }
   return { "Unbekannt", "Unknown" };
   }

/**
\brief Describes the UV index as a risk level.
\param uv_index UV index value.
\param German If true, outputs German text; otherwise English.
\return Description of UV index (e.g., "hoch", "high").
*/
std::string describe_uv_index(std::optional<double> uv_index, bool German) {
   static RuleSet<double> const rules = {
        { std::numeric_limits<double>::lowest(), 0.0,  { "Ungültig",  "undefined"   } },
        { 0.0,  2.1,  { "Gering",    "Low"         } },
        { 2.1,  5.1,  { "Mäßig",     "Moderate"    } },
        { 5.1,  7.1,  { "Hoch",      "High"        } },
        { 7.1, 10.1,  { "Sehr hoch", "Very high"   } },
        { 10.1, std::numeric_limits<double>::infinity(), { "Extrem", "Extreme" } }
      };

   /*
   static RuleSet<double> const detailed_rules = {
        { std::numeric_limits<double>::lowest(), 1.0,  { "Sehr gering",   "Very low"     } },
        { 1.0,  2.1,  { "Gering",        "Low"          } },
        { 2.1,  3.6,  { "Mäßig",         "Moderate"     } },
        { 3.6,  5.1,  { "Deutlich",      "Marked"       } },
        { 5.1,  6.6,  { "Hoch",          "High"         } },
        { 6.6,  7.6,  { "Sehr hoch",     "Very high"    } },
        { 7.6,  9.0,  { "Extrem",        "Extreme"      } },
        { 9.0, 11.0,  { "Gefährlich",    "Hazardous"    } },
        { 11.0, std::numeric_limits<double>::infinity(), { "Kritisch", "Critical" } }
      };
   */

   std::ostringstream oss;
   apply_rules(oss, uv_index, rules, German);
   return oss.str();
   }

/**
\brief Mapping table from WMO weather codes to human-readable descriptions (German/English).

\details
This constant unordered map provides the translation between WMO weather codes (as returned by Open-Meteo)
and their corresponding German and English descriptions.  
It is used by the function \c describe_weather_code to return user-friendly weather summaries.

If new weather codes are introduced by the API, they should be added here.

\see describe_weather_code

\warning
Descriptions are not localized at runtime; only German and English are supported as of now.

\todo Add support for additional languages if required.
\todo Synchronize with Open-Meteo documentation for updates.
*/
const std::unordered_map<int, std::pair<std::string, std::string>> weather_code_descriptions = {
    { 0, {"Klarer Himmel", "Clear sky"}},
    { 1, {"Überwiegend klar", "Mainly clear"}},
    { 2, {"Teilweise bewölkt", "Partly cloudy"}},
    { 3, {"Bewölkt", "Overcast"}},
    {45, {"Nebel", "Fog"}},
    {48, {"Reifnebel", "Depositing rime fog"}},
    {51, {"Leichter Nieselregen", "Light drizzle"}},
    {53, {"Mäßiger Nieselregen", "Moderate drizzle"}},
    {55, {"Starker Nieselregen", "Dense drizzle"}},
    {56, {"Leichter gefrierender Nieselregen", "Light freezing drizzle"}},
    {57, {"Starker gefrierender Nieselregen", "Dense freezing drizzle"}},
    {61, {"Leichter Regen", "Slight rain"}},
    {63, {"Mäßiger Regen", "Moderate rain"}},
    {65, {"Starker Regen", "Heavy rain"}},
    {66, {"Leichter gefrierender Regen", "Light freezing rain"}},
    {67, {"Starker gefrierender Regen", "Heavy freezing rain"}},
    {71, {"Leichter Schneefall", "Slight snow fall"}},
    {73, {"Mäßiger Schneefall", "Moderate snow fall"}},
    {75, {"Starker Schneefall", "Heavy snow fall"}},
    {77, {"Schneegriesel", "Snow grains"}},
    {80, {"Leichte Regenschauer", "Slight rain showers"}},
    {81, {"Mäßige Regenschauer", "Moderate rain showers"}},
    {82, {"Heftige Regenschauer", "Violent rain showers"}},
    {85, {"Leichte Schneeschauer", "Slight snow showers"}},
    {86, {"Starke Schneeschauer", "Heavy snow showers"}},
    {95, {"Gewitter", "Thunderstorm"}},
    {96, {"Gewitter mit leichtem Hagel", "Thunderstorm with slight hail"}},
    {99, {"Gewitter mit starkem Hagel", "Thunderstorm with heavy hail"}}
};


/**
\brief Converts a WMO weather code to a human-readable description.

\details
Looks up the code in \c weather_code_descriptions and returns the appropriate description in German or English.
If the code is not found, returns a default message.

\param code Weather code (WMO standard).
\return Description of the weather condition (localized).
\see weather_code_descriptions

\warning
Only codes present in the map are described; unknown codes are reported as "unbekannt"/"unknown".
*/
std::string describe_weather_code(std::optional<int> code) {
   if (!code) return "n/a";
   if (auto it = weather_code_descriptions.find(*code); it != weather_code_descriptions.end()) {
      return std::format("{}", it->second.first); // alternativ englisch
      }
   else {
      return std::format("unbekannt code {}", *code);
      }
   }

/**
\brief Generates a text summary of the extended weather report.
\param wh The extended current weather data.
\param german If true, outputs German; otherwise English.
\return Human-readable weather summary.
*/
std::string generate_weather_summary(WeatherCurrentExtended const& wh, bool german) {
   std::ostringstream out;

   // Temperatur
   RuleSet<double> temp_rules = {
       {30.0, 60.0, {"Sehr heiß.", "Very hot."}},
       {25.0, 30.0, {"Warm.", "Warm."}},
       {-50.0, 5.0, {"Kalt.", "Cold."}}
      };
   apply_rules(out, wh.temperature_2m, temp_rules, german);

   // Feuchte (Taupunktdifferenz)
   if (wh.temperature_2m && wh.dew_point_2m) {
      std::optional<double> diff = *wh.temperature_2m - *wh.dew_point_2m;
      RuleSet<double> humidity_rules = {
          {-100.0, 2.5, {"Sehr feuchte, schwüle Luft.", "Very humid."}},
          {2.5, 5.0, {"Hohe Luftfeuchtigkeit.", "Humid."}},
          {10.0, 100.0, {"Trockene Luft.", "Dry air."}}
         };
      apply_rules(out, diff, humidity_rules, german);
      }

   // UV-Index
   RuleSet<double> uv_rules = {
       {6.0, 8.0, {"Hoher UV-Index – Sonnenschutz empfohlen.", "High UV – use sun protection."}},
       {8.0, 14.0, {"Sehr hoher UV-Index! Unbedingt Schutz!", "Very high UV – strong protection needed."}}
      };
   apply_rules(out, wh.uv_index, uv_rules, german);

   // Windstärke (Beaufort)
   if (wh.windspeed_10m) {
      auto [de, en] = wind_beaufort_text(*wh.windspeed_10m);
      out << (german ? de : en) << ' ';
      }

   // Windrichtung
   if (wh.winddirection_10m) {
      out << (german ? "aus Richtung " : "from ") << wind_direction_text(*wh.winddirection_10m) << ". ";
      }

   // Niederschlag
   /*
   RuleSet<double> rain_rules = {
       {5.0, 1000.0, {"Starker Regen möglich.", "Heavy rain possible."}},
       {1.0, 5.0, {"Leichter Regen.", "Light rain."}}
   };
   */
   RuleSet<double> const rain_rules = {
        {    0.0,   10.0, { "Leichter Regen möglich.",                "Light rain possible."             } },
        {   10.0,   30.0, { "Mäßiger Regen wahrscheinlich.",          "Moderate rain likely."            } },
        {   30.0,   50.0, { "Starker Regen wahrscheinlich.",          "Heavy rain likely."               } },
        {   50.0,  100.0, { "Sehr starker Regen (Unwettergefahr).",   "Very heavy rain (severe weather possible)." } },
        {  100.0,  300.0, { "Extremer Regen (z. B. Monsun).",          "Extreme rain (e.g., monsoon)."    } },
        {  300.0, 1800.0, { "Regen im Bereich historischer Rekorde.", "Rain within historic world record range." } },
        { 1800.0, std::numeric_limits<double>::infinity(), { "Noch nie gemessene Regenmenge.",  "Rain amount never recorded."      } }
      };
   apply_rules(out, wh.precipitation, rain_rules, german);

   RuleSet<double> const cape_rules =  {
        {    0.0,   100.0, { "Keine Konvektion, stabile Atmosphäre.",         "No convection, stable atmosphere." } },
        {  100.0,   500.0, { "Geringe Instabilität, Schauer unwahrscheinlich.", "Low instability, showers unlikely." } },
        {  500.0,  1000.0, { "Mäßige Instabilität, einzelne Gewitter möglich.", "Moderate instability, isolated thunderstorms possible." } },
        { 1000.0,  2000.0, { "Deutliche Instabilität, erhöhte Gewittergefahr.", "Significant instability, increased thunderstorm risk." } },
        { 2000.0,  3500.0, { "Hohe Instabilität, Unwettergefahr.",              "High instability, severe weather possible." } },
        { 3500.0, 99999.0, { "Extreme Instabilität, schwere Unwetter möglich.", "Extreme instability, severe storms likely." } }
     };

   apply_rules(out, wh.cape, cape_rules, german);

   // Luftdruck
   RuleSet<double> pressure_rules = {
       {980.0, 1000.0, {"Tiefer Luftdruck. Möglicherweise instabil.", "Low pressure. Unstable weather possible."}},
       {1025.0, 1060.0, {"Hoher Luftdruck. Stabil und trocken.", "High pressure. Stable and dry."}}
      };
   apply_rules(out, wh.pressure_msl, pressure_rules, german);


   // Wettercodewarnung (z. B. Gewitter)
   if (wh.weather_code) {
      RuleSet<int> weathercode_alerts = {
          {95, 100, {"Gewittergefahr!", "Thunderstorm risk!"}},
          {61, 66, {"Regenwetter.", "Rainy weather."}},
          {71, 76, {"Schneefall möglich.", "Snow possible."}},
          {85, 87, {"Starker Schneefall!", "Heavy snowfall!"}},
          {45, 49, {"Nebel kann die Sicht beeinträchtigen.", "Fog may reduce visibility."}}
         };
      apply_rules(out, wh.weather_code, weathercode_alerts, german);
      }

   // Strahlung
   RuleSet<double> radiation_rules = {
       {500.0, 800.0, {"Starke Sonneneinstrahlung.", "Strong sunlight."}},
       {800.0, 2000.0, {"Sehr starke Sonnenstrahlung. Schutzmaßnahmen nötig!", "Very intense radiation – protection needed!"}}
      };
   apply_rules(out, wh.shortwave_radiation, radiation_rules, german);

   return out.str().empty() ? (german ? "Keine besonderen Wettererscheinungen." : "No special weather conditions.") : out.str();
   }

} // end of namespace WeatherAPI
