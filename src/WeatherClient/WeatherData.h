#pragma once

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

// Eigene Zeittypen 
using timepoint_ty = std::chrono::sys_seconds;
using date_ty = std::chrono::year_month_day;
using time_ty = std::chrono::hh_mm_ss<std::chrono::seconds>;

/// @brief Art der Wetterdaten (täglich oder stündlich)
enum class WeatherResolution { Current, Current_Extended, Daily, Hourly };

/**
 * @brief Struktur zur Darstellung von Metadaten der Wetteranfrage.
 */
struct WeatherMeta {
   std::string timezone;                ///< Vollständiger Zeitzonenname (z. B. Europe/Berlin)
   std::string timezone_abbreviation;   ///< Kurzform der Zeitzone (z. B. CEST, GMT+2)
   int utc_offset_seconds;              ///< UTC-Offset in Sekunden
   double elevation;                    ///< Höhenlage des Standorts in Metern
};

/// \brief Struktur für aktuellen Wetterwert
struct WeatherCurrent {
   timepoint_ty          timestamp;
   std::optional<double> temperature;
   std::optional<double> windspeed;
   std::optional<double> winddirection;
   std::optional<int>    weathercode;
   std::optional<bool>   is_day;
};

/// @brief Struktur für detaillierte aktuelle Wetterdaten via `current=...`
struct WeatherCurrentExtended {
   timepoint_ty          timestamp;
   std::optional<double> temperature_2m;
   std::optional<double> relative_humidity_2m;
   std::optional<double> dew_point_2m;
   std::optional<double> precipitation;
   std::optional<double> rain;
   std::optional<double> snowfall;
   std::optional<int>    weather_code;
   std::optional<double> pressure_msl;
   std::optional<double> surface_pressure;
   std::optional<double> cloudcover;
   std::optional<double> windspeed_10m;
   std::optional<double> windgusts_10m;
   std::optional<double> winddirection_10m;
   std::optional<double> uv_index;
   std::optional<double> shortwave_radiation;
   std::optional<bool>   is_day;
};

/**
  \brief Struktur zur Darstellung eines Tages mit Wetterdaten.
 */
struct WeatherDay {
   date_ty date; ///< Datum des Wetters (JJJJ-MM-TT)
   time_ty sunrise; ///< Sonnenaufgang (lokale Uhrzeit)
   time_ty sunset;  ///< Sonnenuntergang (lokale Uhrzeit)
   double  temp_max; ///< Tageshöchsttemperatur in °C
   double  temp_min; ///< Tagestiefsttemperatur in °C
   double  precipitation_mm; ///< Gesamtniederschlag in mm
   int     weather_code; ///< Wettercode nach WMO
   double  windspeed_max; ///< Maximale Windgeschwindigkeit in km/h
   double  uv_index; ///< Maximaler UV-Index
   double  temp_mean;
   double  apparent_temp_max;
   double  apparent_temp_min;
   time_ty sunshine_duration;
   double  precipitation_hours;
   double  windgusts_max;
   double  radiation_sum;
   double  evapotranspiration;
   double  rain_sum;
   double  snowfall_sum;
   int     wind_direction_deg;
};

/// @brief Struktur zur Darstellung eines stündlichen Wetterwerts
struct WeatherHour {
   timepoint_ty          timestamp;
   std::optional<double> temperature_2m;
   std::optional<double> relative_humidity_2m;
   std::optional<double> dew_point_2m;
   std::optional<double> apparent_temperature;
   std::optional<double> precipitation;
   std::optional<double> rain;
   std::optional<double> showers;
   std::optional<double> snowfall;
   std::optional<int>    weather_code;
   std::optional<double> pressure_msl;
   std::optional<double> surface_pressure;
   std::optional<double> cloudcover;
   std::optional<double> cloudcover_low;
   std::optional<double> cloudcover_mid;
   std::optional<double> cloudcover_high;
   std::optional<double> shortwave_radiation;
   std::optional<double> direct_radiation;
   std::optional<double> diffuse_radiation;
   std::optional<double> windspeed_10m;
   std::optional<double> windgusts_10m;
   std::optional<double> winddirection_10m;
   std::optional<double> uv_index;
   std::optional<bool>   is_day;
};

template <typename ty>
using RuleSet = std::vector<std::tuple<ty, ty, std::pair<std::string, std::string>>>;

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

inline std::string wind_direction_text(std::optional<double> deg) {
   static constexpr const char* directions[] = {
       "N", "NNO", "NO", "ONO", "O", "OSO", "SO", "SSO",
       "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
      };
   return deg ? directions[static_cast<int>((*deg + 11.25) / 22.5) % 16] : "n/a";
   }

inline std::pair<std::string, std::string> wind_beaufort_text(double speed_kmh) {
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

std::string describe_uv_index(std::optional<double> uv_index, bool German = true) {
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



inline const std::unordered_map<int, std::pair<std::string, std::string>> weather_code_descriptions = {
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
 * @brief Gibt zu einem Wettercode die deutsch/englische Beschreibung aus.
 */
inline std::string describe_weather_code(std::optional<int> code) {
   if (!code) return "n/a";
   if (auto it = weather_code_descriptions.find(*code); it != weather_code_descriptions.end()) {
      return std::format("{}", it->second.first); // alternativ englisch
   }
   else {
      return std::format("unbekannt");
   }
}

inline std::string generate_weather_summary(WeatherCurrentExtended const& wh, bool german = true) {
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
       {8.0, 12.0, {"Sehr hoher UV-Index! Unbedingt Schutz!", "Very high UV – strong protection needed."}}
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
        {    0.0,   10.0, { "Leichter Regen möglich",                "Light rain possible"             } },
        {   10.0,   30.0, { "Mäßiger Regen wahrscheinlich",          "Moderate rain likely"            } },
        {   30.0,   50.0, { "Starker Regen wahrscheinlich",          "Heavy rain likely"               } },
        {   50.0,  100.0, { "Sehr starker Regen (Unwettergefahr)",   "Very heavy rain (severe weather possible)" } },
        {  100.0,  300.0, { "Extremer Regen (z. B. Monsun)",          "Extreme rain (e.g., monsoon)"    } },
        {  300.0, 1800.0, { "Regen im Bereich historischer Rekorde", "Rain within historic world record range" } },
        { 1800.0, std::numeric_limits<double>::infinity(), { "Noch nie gemessene Regenmenge",  "Rain amount never recorded"      } }
      };
   apply_rules(out, wh.precipitation, rain_rules, german);

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

   /*
   // Blitzhäufigkeit
   RuleSet<double> lightning_rules = {
       {0.1, 5.0, {"Einzelne Blitze. Achtung bei Aufenthalt im Freien.", "Some lightning – caution outdoors."}},
       {5.0, 100.0, {"Häufige Blitze! Gefahr durch Gewitter!", "Frequent lightning! Thunderstorm hazard!"}}
   };
   apply_rules(out, wh.lightning, lightning_rules, german);
   */

   // Strahlung
   RuleSet<double> radiation_rules = {
       {500.0, 800.0, {"Starke Sonneneinstrahlung.", "Strong sunlight."}},
       {800.0, 2000.0, {"Sehr starke Sonnenstrahlung. Schutzmaßnahmen nötig!", "Very intense radiation – protection needed!"}}
   };
   apply_rules(out, wh.shortwave_radiation, radiation_rules, german);

   return out.str().empty() ? (german ? "Keine besonderen Wettererscheinungen." : "No special weather conditions.") : out.str();
}
