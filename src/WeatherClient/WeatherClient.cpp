/**
  \file 
  \brief Holt aktuelle Tageswetterdaten von Open-Meteo via Boost.Beast und parsed sie in eine strukturierte Darstellung.
 */

#include "WeatherData.h"
#include "WeatherReader.h"

#include <print>


#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std::string_literals;


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


int main() {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   std::println("Testprogram to use the open-meteo.com Rest API");
   try {
      const double latitude = 52.5366923, longitude = 13.2027663;
      HttpRequest server("api.open-meteo.com");

      auto json = server.perform_get(GetUrl(WeatherResolution::Current_Extended, latitude, longitude, 1));
      const auto meta              = parse<WeatherMeta>(json);
      print(meta);

      const auto cur_extended_data = parse<WeatherCurrentExtended>(json, "current");
      print(meta);
      print(cur_extended_data);
      
      std::println("\n\ndaily weather:");
      json = server.perform_get(GetUrl(WeatherResolution::Daily, latitude, longitude, 14));
      const auto daily_data = parse_series<WeatherDay>(json, "daily", weather_day_fields);
      print(daily_data);
     
      std::println("\n\nshort summation for current weather:");
      const auto current_data = parse<WeatherCurrent>(server.perform_get(GetUrl(WeatherResolution::Current, latitude, longitude, 1)), "current_weather");
      print(current_data);

      std::println("\n\nhourly weather:");
      json = server.perform_get(GetUrl(WeatherResolution::Hourly, latitude, longitude, 7));
      const auto hourly_data = parse_series<WeatherHour>(json, "hourly", weather_hour_fields);
      print(hourly_data);
      }
   catch (std::exception const& e) {
      std::println(stderr, "Fehler: {}", e.what());
      }
   return 0;
   }


