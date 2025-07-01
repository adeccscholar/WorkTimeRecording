
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/json.hpp>

#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <format>
#include <print>


#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std::string_literals;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace json = boost::json;

using timepoint_ty = std::chrono::system_clock::time_point;
using date_ty = std::chrono::year_month_day;
using time_ty = std::chrono::hh_mm_ss<std::chrono::seconds>;

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

std::string describe_weather_code(int code) {
   if (auto it = weather_code_descriptions.find(code); it != weather_code_descriptions.end()) {
      return std::format("{}", it->second.first); // alternativ englisch
   }
   else {
      return std::format("unbekannter Code {}", code);
   }
}

// https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&timezone=auto&current=temperature_2m,relative_humidity_2m,dew_point_2m,weather_code,cloud_cover,wind_speed_10m,wind_gusts_10m"

enum class WeatherResolution : uint32_t { Current, Current_Extended, Daily, Hourly };

struct WeatherData {
   std::string timezone;
   std::string timezone_abbreviation;
   int         utc_offset_seconds;
   double      elevation;

   };


struct WeatherCurrentExtended {
   timepoint_ty       timestamp;
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
   std::optional<bool>   is_day;
   };

std::string fetch_weather_json(WeatherResolution resolution, double latitude, double longitude, int forcast_days = 1) {
   boost::asio::io_context ioc;
   tcp::resolver resolver(ioc);
   tcp::socket socket(ioc);

   const std::string host = "api.open-meteo.com"s;
   const std::string port = "80"s;

   std::string endpoint = std::format("/v1/forecast?latitude={}&longitude={}&timezone=auto", latitude, longitude);
   
   switch (resolution) {
      case WeatherResolution::Current_Extended:
         endpoint += "&current=temperature_2m,relative_humidity_2m,dew_point_2m,precipitation,rain,snowfall,"
                     "weathercode,pressure_msl,surface_pressure,cloudcover,windspeed_10m,windgusts_10m,"
                     "winddirection_10m,uv_index,is_day";
         break;

      default: std::runtime_error("not implemented.");
      }

   const auto results = resolver.resolve(host, port);
   boost::asio::connect(socket, results.begin(), results.end());

   http::request<http::string_body> req{ http::verb::get, endpoint, 11 };
   req.set(http::field::host, host);
   req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

   http::write(socket, req);
   
   boost::beast::flat_buffer buffer;
   http::response<http::string_body> res;
   http::read(socket, buffer, res);
   socket.shutdown(tcp::socket::shutdown_both);

   return res.body();
   }


timepoint_ty parse_timepoint(std::string const& datetime) {
   std::chrono::sys_seconds tp;
   std::istringstream ss(datetime);
   ss >> std::chrono::parse("%Y-%m-%dT%H:%M", tp);
   return tp;
}

WeatherCurrentExtended parse_weather_current_extended(std::string const& json_str) {
   boost::json::value jv = boost::json::parse(json_str);
   const auto& root = jv.as_object();
   const auto& current = root.at("current").as_object();

   WeatherCurrentExtended wcx;
   wcx.timestamp = parse_timepoint(current.at("time").as_string().c_str());

   auto get_opt = [&current](const char* key) -> std::optional<double> {
      if (!current.contains(key) || current.at(key).is_null()) return std::nullopt;
      const auto& v = current.at(key);
      if (v.is_double()) return v.as_double();
      if (v.is_int64()) return static_cast<double>(v.as_int64());
      return std::nullopt;
      };

   auto get_opt_int = [&current](const char* key) -> std::optional<int> {
      if (!current.contains(key) || current.at(key).is_null()) return std::nullopt;
      return static_cast<int>(current.at(key).as_int64());
      };

   auto get_opt_bool = [&current](const char* key) -> std::optional<bool> {
      if (!current.contains(key) || current.at(key).is_null()) return std::nullopt;
      return current.at(key).as_int64() != 0;
      };

   wcx.temperature_2m = get_opt("temperature_2m");
   wcx.relative_humidity_2m = get_opt("relative_humidity_2m");
   wcx.dew_point_2m = get_opt("dew_point_2m");
   wcx.precipitation = get_opt("precipitation");
   wcx.rain = get_opt("rain");
   wcx.snowfall = get_opt("snowfall");
   wcx.weather_code = get_opt_int("weathercode");
   wcx.pressure_msl = get_opt("pressure_msl");
   wcx.surface_pressure = get_opt("surface_pressure");
   wcx.cloudcover = get_opt("cloudcover");
   wcx.windspeed_10m = get_opt("windspeed_10m");
   wcx.windgusts_10m = get_opt("windgusts_10m");
   wcx.winddirection_10m = get_opt("winddirection_10m");
   wcx.uv_index = get_opt("uv_index");
   wcx.is_day = get_opt_bool("is_day");

   return wcx;
}

std::string describe_uv_index(double uv_index, bool German = true) {
   if (uv_index < 0.0)  return German ? "Ungültig" : "undefined";
   if (uv_index <= 2.0)  return German ? "Gering" : "Low";
   if (uv_index <= 5.0)  return German ? "Mäßig" : "Moderate";
   if (uv_index <= 7.0)  return German ? "Hoch" : "High";
   if (uv_index <= 10.0) return German ? "Sehr hoch" : "Very high";
   return                        German ? "Extrem" : "Extreme";
}


void print_weather_current_extended(WeatherCurrentExtended const& w) {
   std::println("Wetter aktuell ({:%d.%m.%Y %X})", w.timestamp);
   std::println("  Temperatur:  {} °C", w.temperature_2m ? std::format("{:.1f}", *w.temperature_2m) : "n/a");
   std::println("  Luftfeuchte: {} %", w.relative_humidity_2m ? std::format("{:.0f}", *w.relative_humidity_2m) : "n/a");
   std::println("  Luftdruck:   {} hPa", w.surface_pressure ? std::format("{:.0f}", *w.surface_pressure) : "n/a");
   std::println("  Taupunkt: {} °C", w.dew_point_2m ? std::format("{:.1f}", *w.dew_point_2m) : "n/a");
   std::println("  Niederschlag: {} mm", w.precipitation ? std::format("{:.1f}", *w.precipitation) : "n/a");
   std::println("  Wind: {} km/h aus {}°",
      w.windspeed_10m ? std::format("{:.1f}", *w.windspeed_10m) : "n/a",
      w.winddirection_10m ? std::format("{:.0f}", *w.winddirection_10m) : "n/a");
   std::println("  UV-Index: {} ({})",
      w.uv_index ? std::format("{:.1f}", *w.uv_index) : "n/a",
      w.uv_index ? describe_uv_index(*w.uv_index) : "n/a");
   std::println("  Wolken: {} %, Tag: {}",
      w.cloudcover ? std::format("{:.0f}", *w.cloudcover) : "n/a",
      w.is_day ? (*w.is_day ? "Ja" : "Nein") : "n/a");
   if (w.weather_code)
      std::println("  Wettercode: {} ({})", *w.weather_code, describe_weather_code(*w.weather_code));

}


int main(int argc, char* argv[]) {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   std::string json = fetch_weather_json(WeatherResolution::Current_Extended, 52.52, 13.41);
   WeatherCurrentExtended weather = parse_weather_current_extended(json);
   print_weather_current_extended(weather);
   //std::println("{}", json);

}

