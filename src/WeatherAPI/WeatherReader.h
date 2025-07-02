#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef WEATHERAPI_BUILD_DLL
#    define WEATHERAPI_API __declspec(dllexport)
#  else
#    define WEATHERAPI_API __declspec(dllimport)
#  endif
#else
#  define WEATHERAPI_API __attribute__((visibility("default")))
#endif


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

WEATHERAPI_API std::string GetServer();
WEATHERAPI_API std::string GetUrl(WeatherResolution resolution, double latitude, double longitude, int forecast_days);

WEATHERAPI_API void from_json(WeatherMeta& meta, boost::json::object const& obj, boost_tools::from_json_tag);
WEATHERAPI_API void from_json(WeatherCurrent& wc, boost::json::object const& obj, boost_tools::from_json_tag);
WEATHERAPI_API void from_json(WeatherCurrentExtended& wce, boost::json::object const& obj, boost_tools::from_json_tag);
WEATHERAPI_API void check_for_api_error(boost::json::object const& json_response);

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


template <typename ty>
using control_func = std::function<void(ty&, boost::json::array const&, std::size_t)>;

template <typename ty>
using control_data = std::span<std::tuple<std::string_view, control_func<ty>>>;

WEATHERAPI_API extern std::vector<std::tuple<std::string_view, control_func<WeatherDay>>> weather_day_fields;
WEATHERAPI_API extern std::vector<std::tuple<std::string_view, control_func<WeatherHour>>> weather_hour_fields;


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

