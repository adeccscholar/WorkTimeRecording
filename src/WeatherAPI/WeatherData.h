#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef WEATHERAPI_BUILD_DLL
#    define WEATHERAPI_API __declspec(dllexport)
#  elif defined(WEATHERAPI_USE_DLL)
#    define WEATHERAPI_API __declspec(dllimport)
#  else
#    define WEATHERAPI_API
#  endif
#else
#  define WEATHERAPI_API __attribute__((visibility("default")))
#endif

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

WEATHERAPI_API std::string wind_direction_text(std::optional<double> deg);
WEATHERAPI_API std::pair<std::string, std::string> wind_beaufort_text(double speed_kmh);
WEATHERAPI_API std::string describe_uv_index(std::optional<double> uv_index, bool German = true);
WEATHERAPI_API std::string describe_weather_code(std::optional<int> code);
WEATHERAPI_API std::string generate_weather_summary(WeatherCurrentExtended const& wh, bool german = true);

WEATHERAPI_API void print(WeatherMeta const& meta);
WEATHERAPI_API void print(WeatherCurrentExtended const& w);
WEATHERAPI_API void print(WeatherCurrent const& wc);
WEATHERAPI_API void print(std::vector<WeatherDay> const& data);
WEATHERAPI_API void print(std::vector<WeatherHour> const& data);
