
#include "WeatherReader.h"

#include <BoostJsonTools.h>      // now in BoostTools
#include <BoostBeastTools.h>     // now in BoostTools
#include <BoostJsonFrom.h>

using namespace std::string_literals;

std::string GetServer() { 
   return "api.open-meteo.com"s; 
   }


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
      case WeatherResolution::TimeCheck:
         endpoint += "&current";
         break;
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

void check_for_api_error(boost::json::object const& json_response) {
   if (json_response.contains("error") && json_response.at("error").as_bool()) {
      std::string reason = json_response.contains("reason") ? json_response.at("reason").as_string().c_str() : "Unbekannter Fehler.";
      throw std::runtime_error(std::format("Open-Meteo API Error: {}", reason));
      }
   }


void from_json(WeatherTime& tc, boost::json::object const& obj, boost_tools::from_json_tag) {
   tc.timestamp = boost_tools::get_value<boost_tools::timepoint_ty>(obj, "time");
   }

void from_json(WeatherMeta& meta, boost::json::object const& obj, boost_tools::from_json_tag) {
   meta.timezone = boost_tools::get_value<std::string>(obj, "timezone");
   meta.timezone_abbreviation = boost_tools::get_value<std::string>(obj, "timezone_abbreviation");
   meta.utc_offset_seconds = boost_tools::get_value<int>(obj, "utc_offset_seconds");
   meta.elevation = boost_tools::get_value<double>(obj, "elevation");
   }

void from_json(WeatherCurrent& wc, boost::json::object const& obj, boost_tools::from_json_tag) {
   wc.timestamp = boost_tools::get_value<boost_tools::timepoint_ty>(obj, "time");
   wc.temperature = boost_tools::get_value<double, true>(obj, "temperature");
   wc.windspeed = boost_tools::get_value<double, true>(obj, "windspeed");
   wc.winddirection = boost_tools::get_value<double, true>(obj, "winddirection");
   wc.weathercode = boost_tools::get_value<int, true>(obj, "weathercode");
   wc.is_day = boost_tools::get_value<bool, true>(obj, "is_day");
   }

void from_json(WeatherCurrentExtended& wce, boost::json::object const& obj, boost_tools::from_json_tag) {
   wce.timestamp = boost_tools::get_value<boost_tools::timepoint_ty>(obj, "time");
   wce.temperature_2m = boost_tools::get_value<double, true>(obj, "temperature_2m");
   wce.relative_humidity_2m = boost_tools::get_value<double, true>(obj, "relative_humidity_2m");
   wce.dew_point_2m = boost_tools::get_value<double, true>(obj, "dew_point_2m");
   wce.precipitation = boost_tools::get_value<double, true>(obj, "precipitation");
   wce.rain = boost_tools::get_value<double, true>(obj, "rain");
   wce.snowfall = boost_tools::get_value<double, true>(obj, "snowfall");
   wce.weather_code = boost_tools::get_value<int, true>(obj, "weathercode");
   wce.pressure_msl = boost_tools::get_value<double, true>(obj, "pressure_msl");
   wce.surface_pressure = boost_tools::get_value<double, true>(obj, "surface_pressure");
   wce.cloudcover = boost_tools::get_value<double, true>(obj, "cloudcover");
   wce.windspeed_10m = boost_tools::get_value<double, true>(obj, "windspeed_10m");
   wce.windgusts_10m = boost_tools::get_value<double, true>(obj, "windgusts_10m");
   wce.winddirection_10m = boost_tools::get_value<double, true>(obj, "winddirection_10m");
   wce.uv_index = boost_tools::get_value<double, true>(obj, "uv_index");
   wce.is_day = boost_tools::get_value<bool, true>(obj, "is_day");
   }

WEATHERAPI_API std::vector<std::tuple<std::string_view, control_func<WeatherDay>>> weather_day_fields = {
  { "time",                       [](auto& d, auto const& a, auto i) { d.date = boost_tools::get_value<boost_tools::date_ty>(a, i); } },
  { "temperature_2m_max",         [](auto& d, auto const& a, auto i) { d.temp_max = boost_tools::get_value<double, false>(a, i); } },
  { "temperature_2m_min",         [](auto& d, auto const& a, auto i) { d.temp_min = boost_tools::get_value<double, false>(a, i); } },
  { "sunrise",                    [](auto& d, auto const& a, auto i) { d.sunrise = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
  { "sunset",                     [](auto& d, auto const& a, auto i) { d.sunset = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
  { "precipitation_sum",          [](auto& d, auto const& a, auto i) { d.precipitation_mm = boost_tools::get_value<double, false>(a, i); } },
  { "weathercode",                [](auto& d, auto const& a, auto i) { d.weather_code = boost_tools::get_value<int, false>(a, i); } },
  { "windspeed_10m_max",          [](auto& d, auto const& a, auto i) { d.windspeed_max = boost_tools::get_value<double, false>(a, i); } },
  { "uv_index_max",               [](auto& d, auto const& a, auto i) { d.uv_index = boost_tools::get_value<double, false>(a, i); } },
  { "temperature_2m_mean",        [](auto& d, auto const& a, auto i) { d.temp_mean = boost_tools::get_value<double, false>(a, i); } },
  { "apparent_temperature_max",   [](auto& d, auto const& a, auto i) { d.apparent_temp_max = boost_tools::get_value<double, false>(a, i); } },
  { "apparent_temperature_min",   [](auto& d, auto const& a, auto i) { d.apparent_temp_min = boost_tools::get_value<double, false>(a, i); } },
  { "sunshine_duration",          [](auto& d, auto const& a, auto i) { d.sunshine_duration = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
  { "precipitation_hours",        [](auto& d, auto const& a, auto i) { d.precipitation_hours = boost_tools::get_value<double, false>(a, i); } },
  { "windgusts_10m_max",          [](auto& d, auto const& a, auto i) { d.windgusts_max = boost_tools::get_value<double, false>(a, i); } },
  { "shortwave_radiation_sum",    [](auto& d, auto const& a, auto i) { d.radiation_sum = boost_tools::get_value<double, false>(a, i); } },
  { "et0_fao_evapotranspiration", [](auto& d, auto const& a, auto i) { d.evapotranspiration = boost_tools::get_value<double, false>(a, i); } },
  { "rain_sum",                   [](auto& d, auto const& a, auto i) { d.rain_sum = boost_tools::get_value<double, false>(a, i); } },
  { "snowfall_sum",               [](auto& d, auto const& a, auto i) { d.snowfall_sum = boost_tools::get_value<double, false>(a, i); } },
  { "winddirection_10m_dominant", [](auto& d, auto const& a, auto i) { d.wind_direction_deg = boost_tools::get_value<int, false>(a, i); } }
};

std::vector<std::tuple<std::string_view, control_func<WeatherHour>>> WEATHERAPI_API weather_hour_fields = {
     { "time",                 [](auto& wh, auto const& a, auto i) { wh.timestamp = boost_tools::get_value<boost_tools::timepoint_ty>(a, i); } },
     { "temperature_2m",       [](auto& wh, auto const& a, auto i) { wh.temperature_2m = boost_tools::get_value<double, true>(a, i); } },
     { "relative_humidity_2m", [](auto& wh, auto const& a, auto i) { wh.relative_humidity_2m = boost_tools::get_value<double, true>(a, i); } },
     { "dew_point_2m",         [](auto& wh, auto const& a, auto i) { wh.dew_point_2m = boost_tools::get_value<double, true>(a, i); } },
     { "apparent_temperature", [](auto& wh, auto const& a, auto i) { wh.apparent_temperature = boost_tools::get_value<double, true>(a, i); } },
     { "precipitation",        [](auto& wh, auto const& a, auto i) { wh.precipitation = boost_tools::get_value<double, true>(a, i); } },
     { "rain",                 [](auto& wh, auto const& a, auto i) { wh.rain = boost_tools::get_value<double, true>(a, i); } },
     { "showers",              [](auto& wh, auto const& a, auto i) { wh.showers = boost_tools::get_value<double, true>(a, i); } },
     { "snowfall",             [](auto& wh, auto const& a, auto i) { wh.snowfall = boost_tools::get_value<double, true>(a, i); } },
     { "weathercode",          [](auto& wh, auto const& a, auto i) { wh.weather_code = boost_tools::get_value<int, true>(a, i); } },
     { "pressure_msl",         [](auto& wh, auto const& a, auto i) { wh.pressure_msl = boost_tools::get_value<double, true>(a, i); } },
     { "surface_pressure",     [](auto& wh, auto const& a, auto i) { wh.surface_pressure = boost_tools::get_value<double, true>(a, i); } },
     { "cloudcover",           [](auto& wh, auto const& a, auto i) { wh.cloudcover = boost_tools::get_value<double, true>(a, i); } },
     { "cloudcover_low",       [](auto& wh, auto const& a, auto i) { wh.cloudcover_low = boost_tools::get_value<double, true>(a, i); } },
     { "cloudcover_mid",       [](auto& wh, auto const& a, auto i) { wh.cloudcover_mid = boost_tools::get_value<double, true>(a, i); } },
     { "cloudcover_high",      [](auto& wh, auto const& a, auto i) { wh.cloudcover_high = boost_tools::get_value<double, true>(a, i); } },
     { "shortwave_radiation",  [](auto& wh, auto const& a, auto i) { wh.shortwave_radiation = boost_tools::get_value<double, true>(a, i); } },
     { "direct_radiation",     [](auto& wh, auto const& a, auto i) { wh.direct_radiation = boost_tools::get_value<double, true>(a, i); } },
     { "diffuse_radiation",    [](auto& wh, auto const& a, auto i) { wh.diffuse_radiation = boost_tools::get_value<double, true>(a, i); } },
     { "windspeed_10m",        [](auto& wh, auto const& a, auto i) { wh.windspeed_10m = boost_tools::get_value<double, true>(a, i); } },
     { "windgusts_10m",        [](auto& wh, auto const& a, auto i) { wh.windgusts_10m = boost_tools::get_value<double, true>(a, i); } },
     { "winddirection_10m",    [](auto& wh, auto const& a, auto i) { wh.winddirection_10m = boost_tools::get_value<double, true>(a, i); } },
     { "uv_index",             [](auto& wh, auto const& a, auto i) { wh.uv_index = boost_tools::get_value<double, true>(a, i); } },
     { "is_day",               [](auto& wh, auto const& a, auto i) { wh.is_day = boost_tools::get_value<bool, true>(a, i); } }
};
