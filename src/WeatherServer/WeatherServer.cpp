
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/json.hpp>

#include <string>
#include <optional>
#include <vector>
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


int main(int argc, char* argv[]) {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   std::string json = fetch_weather_json(WeatherResolution::Current_Extended, 52.52, 13.41);

   std::println("{}", json);

}

