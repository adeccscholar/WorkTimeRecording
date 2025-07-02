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


int main() {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   std::println("Testprogram to use the open-meteo.com Rest API");
   try {
      const double latitude = 52.5366923, longitude = 13.2027663;
      HttpRequest server(GetServer());
      auto json = server.perform_get(GetUrl(WeatherResolution::Current_Extended, latitude, longitude, 1));
      const auto meta              = parse<WeatherMeta>(json);
      print(meta);

      const auto cur_extended_data = parse<WeatherCurrentExtended>(json, "current");
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


