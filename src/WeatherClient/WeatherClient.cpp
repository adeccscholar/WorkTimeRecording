/**
  \file 
  \brief Holt aktuelle Tageswetterdaten von Open-Meteo via Boost.Beast und parsed sie in eine strukturierte Darstellung.
 */

#include "WeatherData.h"
#include "WeatherReader.h"
#include "WeatherPrint.h"

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
      HttpRequest server(WeatherAPI::GetServer());
      auto json = server.perform_get(GetUrl(WeatherAPI::WeatherResolution::TimeCheck, latitude, longitude, 1));
      const auto meta = WeatherAPI::parse<WeatherAPI::WeatherMeta>(json);
      const auto check = WeatherAPI::parse<WeatherAPI::WeatherTime>(json, "current");
      std::println("WeatherAPI CLient: {:%d.%m.%Y %X}", check.timestamp);
      WeatherAPI::print(meta);

      json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Current_Extended, latitude, longitude, 1));
      const auto cur_extended_data = WeatherAPI::parse<WeatherAPI::WeatherCurrentExtended>(json, "current");
      WeatherAPI::print(cur_extended_data);
      
      std::println("\n\ndaily weather:");
      json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Daily, latitude, longitude, 14));
      const auto daily_data = WeatherAPI::parse_series<WeatherAPI::WeatherDay>(json, "daily", WeatherAPI::weather_day_fields);
      WeatherAPI::print(daily_data);
     
      std::println("\n\nshort summation for current weather:");
      const auto current_data = WeatherAPI::parse<WeatherAPI::WeatherCurrent>(server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Current, latitude, longitude, 1)), "current_weather");
      WeatherAPI::print(current_data);

      std::println("\n\nhourly weather:");
      json = server.perform_get(WeatherAPI::GetUrl(WeatherAPI::WeatherResolution::Hourly, latitude, longitude, 14));
      const auto hourly_data = WeatherAPI::parse_series<WeatherAPI::WeatherHour>(json, "hourly", WeatherAPI::weather_hour_fields);
      WeatherAPI::print(hourly_data);
      }
   catch (std::exception const& e) {
      std::println(stderr, "Fehler: {}", e.what());
      }
   return 0;
   }


