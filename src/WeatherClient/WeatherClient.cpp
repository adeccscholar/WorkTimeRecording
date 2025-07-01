/**
  \file 
  \brief Holt aktuelle Tageswetterdaten von Open-Meteo via Boost.Beast und parsed sie in eine strukturierte Darstellung.
 */

#include "WeatherData.h"
#include <formatter_optional.h>  // now in Tools
#include <BoostJsonTools.h>      // now in BoostTools
#include <BoostBeastTools.h>     // now in BoostTools


#include <boost/system/system_error.hpp>  // falls noch nicht inkludiert

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <format>
#include <print>
#include <type_traits>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std::string_literals;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;



/**
  \brief Holt Wetterdaten im JSON-Format (täglich oder stündlich) von Open-Meteo.
 
  \param latitude Breitengrad
  \param longitude Längengrad
  \param forecast_days Anzahl der Vorhersagetage (1-16)
  \param resolution Auflösung (täglich oder stündlich)
  \return JSON-Antwort als String
 */
std::string GetUrl(WeatherResolution resolution, double latitude, double longitude, int forecast_days) {
   if (latitude < -90.0 || latitude > 90.0) {
      throw std::range_error("Latitude must be between -90 and 90.");
      }
   if (longitude < -180.0 || longitude > 180.0) {
      throw std::range_error("Longitude must be between -180 and 180.");
      }
   if (forecast_days < 1 || forecast_days > 16) {
      throw std::range_error("Forecast days must be between 1 and 16.");
      }

   std::string endpoint = std::format("/v1/forecast?latitude={}&longitude={}&timezone=auto", latitude, longitude);

 
   switch (resolution) {
      case WeatherResolution::Current_Extended:
         endpoint += "&current=temperature_2m,relative_humidity_2m,dew_point_2m,precipitation,rain,snowfall,"
                     "weathercode,pressure_msl,surface_pressure,cloudcover,windspeed_10m,windgusts_10m,"
                     "winddirection_10m,uv_index,shortwave_radiation,is_day";
         break;
      case WeatherResolution::Daily:
         endpoint += std::format("&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_sum,"
                                 "weathercode,windspeed_10m_max,uv_index_max,temperature_2m_mean,"
                                 "apparent_temperature_max,apparent_temperature_min,sunshine_duration,"
                                 "precipitation_hours,windgusts_10m_max,shortwave_radiation_sum,"
                                 "et0_fao_evapotranspiration,rain_sum,snowfall_sum,winddirection_10m_dominant&"
                                 "forecast_days={}", forecast_days);
         break;
      case WeatherResolution::Hourly:
         endpoint += std::format("&hourly=temperature_2m,relative_humidity_2m,dew_point_2m,apparent_temperature,"
                                 "precipitation,rain,showers,snowfall,weathercode,pressure_msl,surface_pressure,"
                                 "cloudcover,cloudcover_low,cloudcover_mid,cloudcover_high,shortwave_radiation,"
                                 "direct_radiation,diffuse_radiation,windspeed_10m,windgusts_10m,"
                                 "winddirection_10m,uv_index,is_day&forecast_days={}", forecast_days);
         break;
      case WeatherResolution::Current:
         endpoint += "&current_weather=true";
         break;
      default:
         std::unreachable();
      }
   return endpoint;
   }


//&daily = temperature_2m_max, temperature_2m_min, sunrise, sunset, precipitation_sum,weathercode,windspeed_10m_max,uv_index_max,temperature_2m_mean,apparent_temperature_max,apparent_temperature_min,sunshine_duration,precipitation_hours,windgusts_10m_max,shortwave_radiation_sum,et0_fao_evapotranspiration,rain_sum,snowfall_sum,winddirection_10m_dominant&forecast_days=20

void check_for_api_error(boost::json::object const& json_response) {
   if (json_response.contains("error") && json_response.at("error").as_bool()) {
      std::string reason = json_response.contains("reason") ? json_response.at("reason").as_string().c_str() : "Unbekannter Fehler.";
      throw std::runtime_error(std::format("Open-Meteo API Error: {}", reason));
      }
   }

/*
std::string fetch_weather_json(WeatherResolution resolution, double latitude, double longitude, int forecast_days) {
   HttpRequest server("api.open-meteo.com");
   return server.perform_get(GetUrl(resolution, latitude, longitude, forecast_days));
   }
*/

WeatherMeta parse_weather_meta(std::string const& json_str) {
   const auto current = extract_json_object(std::string_view{ json_str.data(), json_str.size() });
   check_for_api_error(current);
   return WeatherMeta {
      .timezone              = get_value<std::string>(current, "timezone"),
      .timezone_abbreviation = get_value<std::string>(current, "timezone_abbreviation"),
      .utc_offset_seconds    = get_value<int>(current, "utc_offset_seconds"),
      .elevation             = get_value<double>(current, "elevation")
      };
   }

WeatherCurrentExtended parse_weather_current_extended(std::string const& json_str) {
   const auto current = extract_json_object(std::string_view { json_str.data(), json_str.size() } , "current");
   check_for_api_error(current);
   return WeatherCurrentExtended {
          .timestamp               = get_value<timepoint_ty, false>(current, "time"),
          .temperature_2m          = get_value<double,       true>(current, "temperature_2m"),
          .relative_humidity_2m    = get_value<double,       true>(current, "relative_humidity_2m"),
          .dew_point_2m            = get_value<double,       true>(current, "dew_point_2m"),
          .precipitation           = get_value<double,       true>(current, "precipitation"),
          .rain                    = get_value<double,       true>(current, "rain"),
          .snowfall                = get_value<double,       true>(current, "snowfall"),
          .weather_code            = get_value<int,          true>(current, "weathercode"),
          .pressure_msl            = get_value<double,       true>(current, "pressure_msl"),
          .surface_pressure        = get_value<double,       true>(current, "surface_pressure"),
          .cloudcover              = get_value<double,       true>(current, "cloudcover"),
          .windspeed_10m           = get_value<double,       true>(current, "windspeed_10m"),
          .windgusts_10m           = get_value<double,       true>(current, "windgusts_10m"),
          .winddirection_10m       = get_value<double,       true>(current, "winddirection_10m"),
          .uv_index                = get_value<double,       true>(current, "uv_index"),
          .shortwave_radiation     = get_value<double,       true>(current, "shortwave_radiation"),
          .is_day                  = get_value<bool,         true>(current, "is_day")
         };
   }


WeatherCurrent parse_weather_current(const std::string& json_str) {
   const auto current = extract_json_object(std::string_view{ json_str.data(), json_str.size() }, "current_weather");
   check_for_api_error(current);
   return WeatherCurrent {
        .timestamp     = get_value<timepoint_ty>(current, "time"),
        .temperature   = get_value<double, true>(current, "temperature"),
        .windspeed     = get_value<double, true>(current, "windspeed"),
        .winddirection = get_value<double, true>(current, "winddirection"),
        .weathercode   = get_value<int, true>(current, "weathercode"),
        .is_day        = get_value<bool, true>(current, "is_day")
        };
   }



std::vector<WeatherDay> parse_weather_days(std::string const& json_str) {
   const auto daily = extract_json_object(std::string_view{ json_str.data(), json_str.size() }, "daily");
   check_for_api_error(daily);

   boost::json::array const& time_vec = daily.at("time").as_array();
   std::vector<WeatherDay> result;
   result.reserve(time_vec.size());

   for (auto const& t : time_vec) result.emplace_back(std::move(WeatherDay { .date = get_value<date_ty>(t) }));

   static const std::vector<std::tuple<std::string_view, std::function<void(WeatherDay&, boost::json::array const&, std::size_t)>>> field_map{
      { "temperature_2m_max",         [](auto& d, auto const& a, auto i) { d.temp_max            = get_value<double, false>(a, i); } },
      { "temperature_2m_min",         [](auto& d, auto const& a, auto i) { d.temp_min            = get_value<double, false>(a, i); } },
      { "sunrise",                    [](auto& d, auto const& a, auto i) { d.sunrise             = get_value<time_ty, false>(a, i); } },
      { "sunset",                     [](auto& d, auto const& a, auto i) { d.sunset              = get_value<time_ty, false>(a, i); } },
      { "precipitation_sum",          [](auto& d, auto const& a, auto i) { d.precipitation_mm    = get_value<double, false>(a, i); } },
      { "weathercode",                [](auto& d, auto const& a, auto i) { d.weather_code        = get_value<int, false>(a, i); } },
      { "windspeed_10m_max",          [](auto& d, auto const& a, auto i) { d.windspeed_max       = get_value<double, false>(a, i); } },
      { "uv_index_max",               [](auto& d, auto const& a, auto i) { d.uv_index            = get_value<double, false>(a, i); } },
      { "temperature_2m_mean",        [](auto& d, auto const& a, auto i) { d.temp_mean           = get_value<double, false>(a, i); } },
      { "apparent_temperature_max",   [](auto& d, auto const& a, auto i) { d.apparent_temp_max   = get_value<double, false>(a, i); } },
      { "apparent_temperature_min",   [](auto& d, auto const& a, auto i) { d.apparent_temp_min   = get_value<double, false>(a, i); } },
      { "sunshine_duration",          [](auto& d, auto const& a, auto i) { d.sunshine_duration   = get_value<time_ty, false>(a, i); } },
      { "precipitation_hours",        [](auto& d, auto const& a, auto i) { d.precipitation_hours = get_value<double, false>(a, i); } },
      { "windgusts_10m_max",          [](auto& d, auto const& a, auto i) { d.windgusts_max       = get_value<double, false>(a, i); } },
      { "shortwave_radiation_sum",    [](auto& d, auto const& a, auto i) { d.radiation_sum       = get_value<double, false>(a, i); } },
      { "et0_fao_evapotranspiration", [](auto& d, auto const& a, auto i) { d.evapotranspiration  = get_value<double, false>(a, i); } },
      { "rain_sum",                   [](auto& d, auto const& a, auto i) { d.rain_sum            = get_value<double, false>(a, i); } },
      { "snowfall_sum",               [](auto& d, auto const& a, auto i) { d.snowfall_sum        = get_value<double, false>(a, i); } },
      { "winddirection_10m_dominant", [](auto& d, auto const& a, auto i) { d.wind_direction_deg  = get_value<int, false>(a, i); } }
   };
   for (auto const& [field, apply] : field_map) {
      auto it = daily.find(field);
      if (it == daily.end()) continue;

      boost::json::array const& arr = it->value().as_array();
      for (std::size_t i = 0; i < std::min(result.size(), arr.size()); ++i) {
         try {
            apply(result[i], arr, i);  // ← ACHTUNG: angepasst: übergebe das Array + Index
            }
         catch (std::exception const& ex) {
            throw std::runtime_error(std::format("Error in field '{}', index {}: {}", field, i, ex.what()));
         }
      }
   }
   return result;
}



/**
 * \brief Parst stündliche Wetterdaten aus JSON.
 * \details Entspricht im Aufbau parse_weather_days, nutzt Mapping Feldname → Setter-Funktion.
 * \param json_str Das JSON mit den stündlichen Wetterdaten (erwartet: "hourly" Objekt).
 * \returns Vektor mit Wetterstunden.
 * \throw std::runtime_error bei JSON-Fehlern oder Parsing-Problemen.
 */
std::vector<WeatherHour> parse_weather_hours(std::string const& json_str) {
   // Extrahiere das "hourly"-Objekt analog zu den Tagen
   auto const hourly = extract_json_object(std::string_view{ json_str.data(), json_str.size() }, "hourly");

   // Schritt 1: Zeit-Array lesen und Struct initialisieren
   boost::json::array const& times = hourly.at("time").as_array();
   std::vector<WeatherHour> result;
   result.reserve(times.size());

   for (auto const& t : times) {
      WeatherHour wh{ .timestamp = get_value<timepoint_ty>(t) };
      result.emplace_back(std::move(wh));
      }


   using setter_fn = std::function<void(WeatherHour&, boost::json::array const&, std::size_t)>;
   std::vector<std::tuple<std::string_view, setter_fn>> const field_map{
      { "temperature_2m",        [](auto& wh, auto const& a, auto i) { wh.temperature_2m = get_value<double, true>(a, i); } },
      { "relative_humidity_2m",  [](auto& wh, auto const& a, auto i) { wh.relative_humidity_2m = get_value<double, true>(a, i); } },
      { "dew_point_2m",          [](auto& wh, auto const& a, auto i) { wh.dew_point_2m = get_value<double, true>(a, i); } },
      { "apparent_temperature",  [](auto& wh, auto const& a, auto i) { wh.apparent_temperature = get_value<double, true>(a, i); } },
      { "precipitation",         [](auto& wh, auto const& a, auto i) { wh.precipitation = get_value<double, true>(a, i); } },
      { "rain",                  [](auto& wh, auto const& a, auto i) { wh.rain = get_value<double, true>(a, i); } },
      { "showers",               [](auto& wh, auto const& a, auto i) { wh.showers = get_value<double, true>(a, i); } },
      { "snowfall",              [](auto& wh, auto const& a, auto i) { wh.snowfall = get_value<double, true>(a, i); } },
      { "weathercode",           [](auto& wh, auto const& a, auto i) { wh.weather_code = get_value<int, true>(a, i); } },
      { "pressure_msl",          [](auto& wh, auto const& a, auto i) { wh.pressure_msl = get_value<double, true>(a, i); } },
      { "surface_pressure",      [](auto& wh, auto const& a, auto i) { wh.surface_pressure = get_value<double, true>(a, i); } },
      { "cloudcover",            [](auto& wh, auto const& a, auto i) { wh.cloudcover = get_value<double, true>(a, i); } },
      { "cloudcover_low",        [](auto& wh, auto const& a, auto i) { wh.cloudcover_low = get_value<double, true>(a, i); } },
      { "cloudcover_mid",        [](auto& wh, auto const& a, auto i) { wh.cloudcover_mid = get_value<double, true>(a, i); } },
      { "cloudcover_high",       [](auto& wh, auto const& a, auto i) { wh.cloudcover_high = get_value<double, true>(a, i); } },
      { "shortwave_radiation",   [](auto& wh, auto const& a, auto i) { wh.shortwave_radiation = get_value<double, true>(a, i); } },
      { "direct_radiation",      [](auto& wh, auto const& a, auto i) { wh.direct_radiation = get_value<double, true>(a, i); } },
      { "diffuse_radiation",     [](auto& wh, auto const& a, auto i) { wh.diffuse_radiation = get_value<double, true>(a, i); } },
      { "windspeed_10m",         [](auto& wh, auto const& a, auto i) { wh.windspeed_10m = get_value<double, true>(a, i); } },
      { "windgusts_10m",         [](auto& wh, auto const& a, auto i) { wh.windgusts_10m = get_value<double, true>(a, i); } },
      { "winddirection_10m",     [](auto& wh, auto const& a, auto i) { wh.winddirection_10m = get_value<double, true>(a, i); } },
      { "uv_index",              [](auto& wh, auto const& a, auto i) { wh.uv_index = get_value<double, true>(a, i); } },
      { "is_day",                [](auto& wh, auto const& a, auto i) { wh.is_day = get_value<bool, true>(a, i); } }
   };

   for (auto const& [field, apply] : field_map) {
      auto it = hourly.find(field);
      if (it == hourly.end()) continue;

      boost::json::array const& arr = it->value().as_array();
      for (std::size_t i = 0; i < std::min(result.size(), arr.size()); ++i) {
         try {
            apply(result[i], arr, i); // Analog zum Tagesparser
            }
         catch (std::exception const& ex) {
            throw std::runtime_error(std::format("Error in field '{}', index {}: {}", field, i, ex.what()));
            }
         }
      }
   return result;
   }





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


void print(const WeatherCurrent& wc) {
      std::println("Aktuell {:%d.%m.%Y %X}: {:4.1f} °C, Wind {:4.1f} km/h aus {:3.0f}°, Code {:2}, Tag: {}",
         wc.timestamp, wc.temperature, wc.windspeed, wc.winddirection, wc.weathercode, wc.is_day);
   }

void print(std::vector<WeatherDay> const& data) {
   for (const auto& d : data) {
      std::println("{}: {} {} ({:4.1f}°C / {:4.1f}°C), Sonne: {}, Regen:{:6.1f} mm, UV: {:4.1f} ({:<9}), Code: {:>2} ({})",
             d.date, d.sunrise, d.sunset, d.temp_min, d.temp_max, d.sunshine_duration, d.precipitation_mm, 
             d.uv_index, describe_uv_index(d.uv_index), d.weather_code, describe_weather_code(d.weather_code));
             }
   }


/**
 * @brief Gibt stündliche Wetterdaten in die Konsole aus.
 */
void print(const std::vector<WeatherHour>& data) {
   for (const auto& wh : data) {
       std::println("{:%d.%m.%Y %X}: Temp {:4.1f} °C, Taupunkt {:4.1f} °C, Wolken: {:3.0f} %, UV: {:3.1f} ({:<9}), Druck: {:4.0f} hPA, Tag: {:<5},  WCode: {:2} ({})",
         wh.timestamp, wh.temperature_2m, wh.dew_point_2m, wh.cloudcover, wh.uv_index, describe_uv_index(wh.uv_index), wh.surface_pressure,
         wh.is_day,wh.weather_code, describe_weather_code(wh.weather_code)
         );
   }
}

int main() {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   try {
      const double latitude = 52.5366923, longitude = 13.2027663;
      HttpRequest server("api.open-meteo.com");

      auto json = server.perform_get(GetUrl(WeatherResolution::Current_Extended, latitude, longitude, 1));
      auto meta = parse_weather_meta(json);
      //const auto cur_extended_data = parse_weather_current_extended(json);
      print(meta);
      //print(cur_extended_data);
      print(parse_weather_current_extended(json));

      std::println("\n\ndaily weather:");
      const auto daily_data = parse_weather_days(server.perform_get(GetUrl(WeatherResolution::Daily, latitude, longitude, 14)));
      print(daily_data);
     
      std::println("\n\nshort summation for current weather:");
      const auto current_data = parse_weather_current(server.perform_get(GetUrl(WeatherResolution::Current, latitude, longitude, 1)));
      print(current_data);

      std::println("\n\nhourly weather:");
      const auto hourly_data = parse_weather_hours(server.perform_get(GetUrl(WeatherResolution::Hourly, latitude, longitude, 7)));
      print(hourly_data);
      }
   catch (std::exception const& e) {
      std::println(stderr, "Fehler: {}", e.what());
      }
   return 0;
}


