
/*
std::string fetch_weather_json(WeatherResolution resolution, double latitude, double longitude, int forecast_days) {
   HttpRequest server("api.open-meteo.com");
   return server.perform_get(GetUrl(resolution, latitude, longitude, forecast_days));
   }
*/

/*
WeatherMeta parse_weather_meta(std::string const& json_str) {
   const auto current = boost_tools::extract_json_object(std::string_view{ json_str.data(), json_str.size() });
   check_for_api_error(current);
   return WeatherMeta {
      .timezone              = boost_tools::get_value<std::string>(current, "timezone"),
      .timezone_abbreviation = boost_tools::get_value<std::string>(current, "timezone_abbreviation"),
      .utc_offset_seconds    = boost_tools::get_value<int>(current, "utc_offset_seconds"),
      .elevation             = boost_tools::get_value<double>(current, "elevation")
      };
   }

WeatherCurrentExtended parse_weather_current_extended(std::string const& json_str) {
   const auto current = boost_tools::extract_json_object(std::string_view { json_str.data(), json_str.size() } , "current");
   check_for_api_error(current);
   return WeatherCurrentExtended {
          .timestamp               = boost_tools::get_value<boost_tools::timepoint_ty, false>(current, "time"),
          .temperature_2m          = boost_tools::get_value<double,       true>(current, "temperature_2m"),
          .relative_humidity_2m    = boost_tools::get_value<double,       true>(current, "relative_humidity_2m"),
          .dew_point_2m            = boost_tools::get_value<double,       true>(current, "dew_point_2m"),
          .precipitation           = boost_tools::get_value<double,       true>(current, "precipitation"),
          .rain                    = boost_tools::get_value<double,       true>(current, "rain"),
          .snowfall                = boost_tools::get_value<double,       true>(current, "snowfall"),
          .weather_code            = boost_tools::get_value<int,          true>(current, "weathercode"),
          .pressure_msl            = boost_tools::get_value<double,       true>(current, "pressure_msl"),
          .surface_pressure        = boost_tools::get_value<double,       true>(current, "surface_pressure"),
          .cloudcover              = boost_tools::get_value<double,       true>(current, "cloudcover"),
          .windspeed_10m           = boost_tools::get_value<double,       true>(current, "windspeed_10m"),
          .windgusts_10m           = boost_tools::get_value<double,       true>(current, "windgusts_10m"),
          .winddirection_10m       = boost_tools::get_value<double,       true>(current, "winddirection_10m"),
          .uv_index                = boost_tools::get_value<double,       true>(current, "uv_index"),
          .shortwave_radiation     = boost_tools::get_value<double,       true>(current, "shortwave_radiation"),
          .is_day                  = boost_tools::get_value<bool,         true>(current, "is_day")
         };
   }


WeatherCurrent parse_weather_current(const std::string& json_str) {
   const auto current = boost_tools::extract_json_object(std::string_view{ json_str.data(), json_str.size() }, "current_weather");
   check_for_api_error(current);
   return WeatherCurrent {
        .timestamp     = boost_tools::get_value<boost_tools::timepoint_ty>(current, "time"),
        .temperature   = boost_tools::get_value<double, true>(current, "temperature"),
        .windspeed     = boost_tools::get_value<double, true>(current, "windspeed"),
        .winddirection = boost_tools::get_value<double, true>(current, "winddirection"),
        .weathercode   = boost_tools::get_value<int, true>(current, "weathercode"),
        .is_day        = boost_tools::get_value<bool, true>(current, "is_day")
        };
   }

   */
   /*/
   std::vector<WeatherDay> parse_weather_days(std::string const& json_str) {
      const auto daily = boost_tools::extract_json_object(std::string_view{ json_str.data(), json_str.size() }, "daily");
      check_for_api_error(daily);

      boost::json::array const& time_vec = daily.at("time").as_array();
      std::vector<WeatherDay> result;
      result.reserve(time_vec.size());

      for (auto const& t : time_vec) result.emplace_back(std::move(WeatherDay { .date = boost_tools::get_value<boost_tools::date_ty>(t) }));

      static const std::vector<std::tuple<std::string_view, std::function<void(WeatherDay&, boost::json::array const&, std::size_t)>>> field_map{
         { "temperature_2m_max",         [](auto& d, auto const& a, auto i) { d.temp_max            = boost_tools::get_value<double, false>(a, i); } },
         { "temperature_2m_min",         [](auto& d, auto const& a, auto i) { d.temp_min            = boost_tools::get_value<double, false>(a, i); } },
         { "sunrise",                    [](auto& d, auto const& a, auto i) { d.sunrise             = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
         { "sunset",                     [](auto& d, auto const& a, auto i) { d.sunset              = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
         { "precipitation_sum",          [](auto& d, auto const& a, auto i) { d.precipitation_mm    = boost_tools::get_value<double, false>(a, i); } },
         { "weathercode",                [](auto& d, auto const& a, auto i) { d.weather_code        = boost_tools::get_value<int, false>(a, i); } },
         { "windspeed_10m_max",          [](auto& d, auto const& a, auto i) { d.windspeed_max       = boost_tools::get_value<double, false>(a, i); } },
         { "uv_index_max",               [](auto& d, auto const& a, auto i) { d.uv_index            = boost_tools::get_value<double, false>(a, i); } },
         { "temperature_2m_mean",        [](auto& d, auto const& a, auto i) { d.temp_mean           = boost_tools::get_value<double, false>(a, i); } },
         { "apparent_temperature_max",   [](auto& d, auto const& a, auto i) { d.apparent_temp_max   = boost_tools::get_value<double, false>(a, i); } },
         { "apparent_temperature_min",   [](auto& d, auto const& a, auto i) { d.apparent_temp_min   = boost_tools::get_value<double, false>(a, i); } },
         { "sunshine_duration",          [](auto& d, auto const& a, auto i) { d.sunshine_duration   = boost_tools::get_value<boost_tools::time_ty, false>(a, i); } },
         { "precipitation_hours",        [](auto& d, auto const& a, auto i) { d.precipitation_hours = boost_tools::get_value<double, false>(a, i); } },
         { "windgusts_10m_max",          [](auto& d, auto const& a, auto i) { d.windgusts_max       = boost_tools::get_value<double, false>(a, i); } },
         { "shortwave_radiation_sum",    [](auto& d, auto const& a, auto i) { d.radiation_sum       = boost_tools::get_value<double, false>(a, i); } },
         { "et0_fao_evapotranspiration", [](auto& d, auto const& a, auto i) { d.evapotranspiration  = boost_tools::get_value<double, false>(a, i); } },
         { "rain_sum",                   [](auto& d, auto const& a, auto i) { d.rain_sum            = boost_tools::get_value<double, false>(a, i); } },
         { "snowfall_sum",               [](auto& d, auto const& a, auto i) { d.snowfall_sum        = boost_tools::get_value<double, false>(a, i); } },
         { "winddirection_10m_dominant", [](auto& d, auto const& a, auto i) { d.wind_direction_deg  = boost_tools::get_value<int, false>(a, i); } }
      };
      for (auto const& [field, apply] : field_map) {
         auto it = daily.find(field);
         if (it == daily.end()) continue;

         boost::json::array const& arr = it->value().as_array();
         for (std::size_t i = 0; i < std::min(result.size(), arr.size()); ++i) {
            try {
               apply(result[i], arr, i);  //
               }
            catch (std::exception const& ex) {
               throw std::runtime_error(std::format("Error in field '{}', index {}: {}", field, i, ex.what()));
            }
         }
      }
      return result;
   }

   */



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

std::string fetch_weather_json_old(WeatherResolution resolution, double latitude, double longitude, int forecast_days) {

   if (latitude < -90.0 || latitude > 90.0) {
      throw std::range_error("Latitude must be between -90 and 90.");
   }
   if (longitude < -180.0 || longitude > 180.0) {
      throw std::range_error("Longitude must be between -180 and 180.");
   }
   if (forecast_days < 1 || forecast_days > 16) {
      throw std::range_error("Forecast days must be between 1 and 16.");
   }

   boost::asio::io_context ioc;
   tcp::resolver resolver(ioc);
   tcp::socket socket(ioc);

   const std::string host = "api.open-meteo.com";
   const std::string port = "80";


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


   auto const results = resolver.resolve(host, port);
   boost::asio::connect(socket, results.begin(), results.end());

   http::request<http::string_body> req{ http::verb::get, endpoint, 11 }; // 1 * 10 + 1 = 11 für HTTP/1.1
   req.set(http::field::host, host);
   req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

   http::write(socket, req);
   boost::beast::flat_buffer buffer;
   http::response<http::string_body> res;
   http::read(socket, buffer, res);
   socket.shutdown(tcp::socket::shutdown_both);

   return res.body();
}


std::string timepoint_to_string(timepoint_ty const& tp) {
   return std::format("{:%d.%m.%Y %X}", tp);
   }

date_ty parse_date(std::string const& date_str) {
   std::chrono::sys_days tp;
   std::istringstream ss(date_str);
   ss >> std::chrono::parse("%Y-%m-%d", tp);
   return std::chrono::year_month_day{ tp };
   }

time_ty parse_time(std::string const& datetime) {
   std::chrono::local_time<std::chrono::seconds> lt;
   std::istringstream ss(datetime);
   ss >> std::chrono::parse("%Y-%m-%dT%H:%M", lt);
   return std::chrono::hh_mm_ss{ lt.time_since_epoch() % std::chrono::days{1} };
   }

timepoint_ty parse_timepoint(std::string const& datetime) {
   std::chrono::sys_seconds tp;
   std::istringstream ss(datetime);
   ss >> std::chrono::parse("%Y-%m-%dT%H:%M", tp);
   return tp;
   }


// ------------------------------------------------------------------------------------------------
// Parsen aus einem Value
// ------------------------------------------------------------------------------------------------


std::optional<double> get_optional_double(boost::json::value const& val) {
   if (val.is_null()) return std::nullopt;
   if (val.is_double()) return val.as_double();
   if (val.is_int64()) return static_cast<double>(val.as_int64());
   if (val.is_uint64()) return static_cast<double>(val.as_uint64());
   return std::nullopt;
   }


// ------------------------------------------------------------------------------------------------
// Parsen aus einem Array mit Werten
// ------------------------------------------------------------------------------------------------

/**
 * @brief Sicheres Parsen eines optionalen double-Wertes aus einem JSON-Array.
 */
std::optional<double> get_optional_double(boost::json::array const& arr, size_t index) {
   if (index >= arr.size() || arr[index].is_null()) return std::nullopt;

   const auto& val = arr[index];
   if (val.is_double()) return val.as_double();
   if (val.is_int64()) return static_cast<double>(val.as_int64());
   if (val.is_uint64()) return static_cast<double>(val.as_uint64());
   return std::nullopt; // falls Typ unerwartet
   }

/**
 * @brief Sicheres Parsen eines optionalen int-Wertes aus einem JSON-Array.
 */
std::optional<int> get_optional_int(boost::json::array const& arr, size_t index) {
   if (index >= arr.size() || arr[index].is_null()) return std::nullopt;
   return static_cast<int>(arr[index].as_int64());
   }

/**
 * @brief Sicheres Parsen eines optionalen bool-Wertes aus einem JSON-Array.
 */
std::optional<bool> get_optional_bool(boost::json::array const& arr, size_t index) {
   if (index >= arr.size() || arr[index].is_null()) return std::nullopt;
   return arr[index].as_int64() != 0;
   }

WeatherMeta parse_weather_meta_old(std::string const& jstr) {
   boost::json::value jv = boost::json::parse(jstr);
   const auto& obj = jv.as_object();
   check_for_api_error(obj);
   return WeatherMeta{
       std::string(obj.at("timezone").as_string()),
       std::string(obj.at("timezone_abbreviation").as_string()),
       static_cast<int>(obj.at("utc_offset_seconds").as_int64()),
       obj.at("elevation").as_double()
      };
   }

WeatherCurrent parse_weather_current_old(std::string const& json_str) {
   boost::json::value jv = boost::json::parse(json_str);
   auto const& root = jv.as_object();

   if (!root.contains("current_weather") || !root.at("current_weather").is_object())
      throw std::runtime_error("current_weather fehlt oder ist ungültig");

   auto const& current = root.at("current_weather").as_object();

   WeatherCurrent wc;

   // Sicheres Parsen jedes Feldes
   if (current.contains("time"))
      wc.timestamp = parse_timepoint(current.at("time").as_string().c_str());

   if (current.contains("temperature"))
      wc.temperature = get_optional_double(current.at("temperature"));

   if (current.contains("windspeed"))
      wc.windspeed = get_optional_double(current.at("windspeed"));

   if (current.contains("winddirection"))
      wc.winddirection = get_optional_double(current.at("winddirection"));

   if (current.contains("weathercode") && current.at("weathercode").is_int64())
      wc.weathercode = static_cast<int>(current.at("weathercode").as_int64());

   if (current.contains("is_day") && current.at("is_day").is_int64())
      wc.is_day = current.at("is_day").as_int64() != 0;

   return wc;
}



/// @brief Parsen der aktuellen Wetterdaten bei Verwendung von `current=...`
WeatherCurrentExtended parse_weather_current_extended_old(std::string const& json_str) {
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

std::vector<WeatherDay> parse_weather_days_old(std::string const& jstr) {
   boost::json::value jv = boost::json::parse(jstr);
   auto daily = jv.as_object()["daily"].as_object();

   auto time = daily["time"].as_array();
   auto temp_max = daily["temperature_2m_max"].as_array();
   auto temp_min = daily["temperature_2m_min"].as_array();
   auto sunrise = daily["sunrise"].as_array();
   auto sunset = daily["sunset"].as_array();
   auto precip = daily["precipitation_sum"].as_array();
   auto weathercode = daily["weathercode"].as_array();
   auto windspeed = daily["windspeed_10m_max"].as_array();
   auto uvindex = daily["uv_index_max"].as_array();
   auto temp_mean = daily["temperature_2m_mean"].as_array();
   auto app_max = daily["apparent_temperature_max"].as_array();
   auto app_min = daily["apparent_temperature_min"].as_array();
   auto sunshine = daily["sunshine_duration"].as_array();
   auto precip_hours = daily["precipitation_hours"].as_array();
   auto windgusts = daily["windgusts_10m_max"].as_array();
   auto radiation = daily["shortwave_radiation_sum"].as_array();
   auto evap = daily["et0_fao_evapotranspiration"].as_array();
   auto rain_sum = daily["rain_sum"].as_array();
   auto snowfall_sum = daily["snowfall_sum"].as_array();
   auto winddir = daily["winddirection_10m_dominant"].as_array();

   std::vector<WeatherDay> days;
   for (std::size_t i = 0; i < time.size(); ++i) {
      WeatherDay d;
      d.date = parse_date(std::string(time[i].as_string()));
      d.sunrise = parse_time(std::string(sunrise[i].as_string()));
      d.sunset = parse_time(std::string(sunset[i].as_string()));
      d.temp_max = temp_max[i].as_double();
      d.temp_min = temp_min[i].as_double();
      d.precipitation_mm = precip[i].as_double();
      d.weather_code = weathercode[i].as_int64();
      d.windspeed_max = windspeed[i].as_double();
      d.uv_index = uvindex[i].as_double();
      d.temp_mean = temp_mean[i].as_double();
      d.apparent_temp_max = app_max[i].as_double();
      d.apparent_temp_min = app_min[i].as_double();
      d.sunshine_duration = time_ty(std::chrono::seconds(static_cast<int>(sunshine[i].as_double())));
      d.precipitation_hours = precip_hours[i].as_double();
      d.windgusts_max = windgusts[i].as_double();
      d.radiation_sum = radiation[i].as_double();
      d.evapotranspiration = evap[i].as_double();
      d.rain_sum = rain_sum[i].as_double();
      d.snowfall_sum = snowfall_sum[i].as_double();
      d.wind_direction_deg = winddir[i].as_int64();
      days.emplace_back(d);
   }
   return days;
}

std::vector<WeatherHour> parse_weather_hours_old(std::string const& json_str) {
   boost::json::value jv = boost::json::parse(json_str);
   const auto& root = jv.as_object();
   const auto& hourly = root.at("hourly").as_object();

   const auto& times = hourly.at("time").as_array();
   const auto& temperature_2m = hourly.at("temperature_2m").as_array();
   const auto& relative_humidity_2m = hourly.at("relative_humidity_2m").as_array();
   const auto& dew_point_2m = hourly.at("dew_point_2m").as_array();
   const auto& apparent_temperature = hourly.at("apparent_temperature").as_array();
   const auto& precipitation = hourly.at("precipitation").as_array();
   const auto& rain = hourly.at("rain").as_array();
   const auto& showers = hourly.at("showers").as_array();
   const auto& snowfall = hourly.at("snowfall").as_array();
   const auto& weathercode = hourly.at("weathercode").as_array();
   const auto& pressure_msl = hourly.at("pressure_msl").as_array();
   const auto& surface_pressure = hourly.at("surface_pressure").as_array();
   const auto& cloudcover = hourly.at("cloudcover").as_array();
   const auto& cloudcover_low = hourly.at("cloudcover_low").as_array();
   const auto& cloudcover_mid = hourly.at("cloudcover_mid").as_array();
   const auto& cloudcover_high = hourly.at("cloudcover_high").as_array();
   const auto& shortwave_radiation = hourly.at("shortwave_radiation").as_array();
   const auto& direct_radiation = hourly.at("direct_radiation").as_array();
   const auto& diffuse_radiation = hourly.at("diffuse_radiation").as_array();
   const auto& windspeed_10m = hourly.at("windspeed_10m").as_array();
   const auto& windgusts_10m = hourly.at("windgusts_10m").as_array();
   const auto& winddirection_10m = hourly.at("winddirection_10m").as_array();
   const auto& uv_index = hourly.at("uv_index").as_array();
   const auto& is_day = hourly.at("is_day").as_array();

   size_t count = times.size();
   std::vector<WeatherHour> result;
   result.reserve(count);

   for (size_t i = 0; i < count; ++i) {
      WeatherHour wh{
          parse_timepoint(times[i].as_string().c_str()),
          get_optional_double(temperature_2m, i),
          get_optional_double(relative_humidity_2m, i),
          get_optional_double(dew_point_2m, i),
          get_optional_double(apparent_temperature, i),
          get_optional_double(precipitation, i),
          get_optional_double(rain, i),
          get_optional_double(showers, i),
          get_optional_double(snowfall, i),
          get_optional_int(weathercode, i),
          get_optional_double(pressure_msl, i),
          get_optional_double(surface_pressure, i),
          get_optional_double(cloudcover, i),
          get_optional_double(cloudcover_low, i),
          get_optional_double(cloudcover_mid, i),
          get_optional_double(cloudcover_high, i),
          get_optional_double(shortwave_radiation, i),
          get_optional_double(direct_radiation, i),
          get_optional_double(diffuse_radiation, i),
          get_optional_double(windspeed_10m, i),
          get_optional_double(windgusts_10m, i),
          get_optional_double(winddirection_10m, i),
          get_optional_double(uv_index, i),
          get_optional_bool(is_day, i)
      };
      result.emplace_back(std::move(wh));
   }
   return result;
}



std::string describe_uv_index_old(double uv_index, bool German = true) {
   if (uv_index < 0.0)  return German ? "Ungültig" : "undefined";
   if (uv_index <= 2.0)  return German ? "Gering" : "Low";
   if (uv_index <= 5.0)  return German ? "Mäßig" : "Moderate";
   if (uv_index <= 7.0)  return German ? "Hoch" : "High";
   if (uv_index <= 10.0) return German ? "Sehr hoch" : "Very high";
   return                        German ? "Extrem" : "Extreme";
}




std::string generate_weather_summary_old(WeatherCurrentExtended const& wh) {
   std::ostringstream summary;

   // Temperaturbewertung
   if (wh.temperature_2m && wh.temperature_2m.value() >= 30.0)
      summary << "Sehr heiß. ";
   else if (wh.temperature_2m && wh.temperature_2m.value() >= 25.0)
      summary << "Warm. ";
   else if (wh.temperature_2m && wh.temperature_2m.value() < 5.0)
      summary << "Kalt. ";

   // Luftfeuchtigkeit / Taupunkt (wenn beide vorhanden)
   if (wh.dew_point_2m && wh.temperature_2m) {
      double diff = *wh.temperature_2m - *wh.dew_point_2m;
      if (diff < 2.5)
         summary << "Sehr feuchte, schwüle Luft. ";
      else if (diff < 5.0)
         summary << "Hohe Luftfeuchtigkeit. ";
      else if (diff > 10.0)
         summary << "Trockene Luft. ";
      }

   // UV-Index
   if (wh.uv_index && *wh.uv_index >= 6.0)
      summary << "Hoher UV-Index – Sonnenschutz empfohlen. ";

   // Wind
   if (wh.windgusts_10m) {
      if (*wh.windgusts_10m > 50.0)
         summary << "Sturmböen möglich! ";
      else if (*wh.windgusts_10m > 30.0)
         summary << "Frischer bis starker Wind. ";
      }

   // Niederschlag
   if (wh.precipitation) {
      if (*wh.precipitation >= 5.0)
         summary << "Starker Regen möglich. ";
      else if (*wh.precipitation >= 1.0)
         summary << "Leichter Regen. ";
      }

   // Wettercode-Warnungen (vereinfachtes Beispiel)
   if (wh.weather_code) {
      switch (*wh.weather_code) {
      case 95:
      case 96:
      case 99:
         summary << "Gewittergefahr! ";
         break;
      case 61:
      case 63:
      case 65:
         summary << "Regenwetter. ";
         break;
      case 71:
      case 73:
      case 75:
         summary << "Schneefall möglich. ";
         break;
      case 85:
      case 86:
         summary << "Starker Schneefall! ";
         break;
      case 45:
      case 48:
         summary << "Nebel kann die Sicht beeinträchtigen. ";
         break;
      }
   }

   return summary.str().empty() ? "Keine besonderen Wettererscheinungen." : summary.str();
}


void print_weather_current(const WeatherCurrent& wc) {
   std::string temp_str = wc.temperature
      ? std::format("{:.1f} °C", *wc.temperature)
      : "n/a";

   std::string wind_str = (wc.windspeed && wc.winddirection)
      ? std::format("{:.1f} km/h aus {:.0f}°", *wc.windspeed, *wc.winddirection)
      : "n/a";

   std::string code_str = wc.weathercode
      ? std::format("{}", *wc.weathercode)
      : "n/a";

   std::string is_day_str = wc.is_day
      ? (*wc.is_day ? "Ja" : "Nein")
      : "n/a";

   std::println("Aktuell {:%d.%m.%Y %X}: {}, Wind {}, Code {}, Tag: {}",
      wc.timestamp, temp_str, wind_str, code_str, is_day_str);
}


void print_weather_current_extended_old(WeatherCurrentExtended const& w) {
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
   std::println("\n Zusammenfassung:\n{}", generate_weather_summary(w));
   }

void print_weather_hours(const std::vector<WeatherHour>& data) {
   for (const auto& wh : data) {
      std::println("{:%d.%m.%Y %X}: Temp {} °C, Taupunkt {} °C, WCode: {} ({}), Wolken: {} %, UV: {} ({}), Druck: {} hPA, Tag: {}",
         wh.timestamp,
         wh.temperature_2m ? std::format("{:.1f}", *wh.temperature_2m) : "n/a",
         wh.dew_point_2m ? std::format("{:.1f}", *wh.dew_point_2m) : "n/a",
         wh.weather_code ? std::to_string(*wh.weather_code) : "n/a",
         wh.weather_code ? describe_weather_code(*wh.weather_code) : "n/a",
         wh.cloudcover ? std::format("{:.0f}", *wh.cloudcover) : "n/a",
         wh.uv_index ? std::format("{:.1f}", *wh.uv_index) : "n/a",
         wh.uv_index ? describe_uv_index(*wh.uv_index) : "n/a",
         wh.surface_pressure ? std::format("{:.0f}", *wh.surface_pressure) : "n/a",
         wh.is_day ? (*wh.is_day ? "Ja" : "Nein") : "n/a"
      );
   }
}


int main() {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   try {
      // const double latitude = 52.52, longitude = 13.405;
      const double latitude = 52.5366923, longitude = 13.2027663;
      auto json = fetch_weather_json(WeatherResolution::Current_Extended, latitude, longitude, 1);
      auto meta = parse_weather_meta(json);
      const auto cur_extended_data = parse_weather_current_extended(json);
      print_weather_meta(meta);
      print_weather_current_extended(cur_extended_data);

      json = fetch_weather_json(WeatherResolution::Daily, latitude, longitude, 14);
      const auto daily_data = parse_weather_days(json);
      std::println("\n\ndaily weather:");
      print_weather_daily(daily_data);

      json = fetch_weather_json(WeatherResolution::Current, latitude, longitude, 1);
      const auto current_data = parse_weather_current(json);
      std::println("\n\nshort summation for current weather:");
      print_weather_current(current_data);

      json = fetch_weather_json(WeatherResolution::Hourly, latitude, longitude, 7);
      const auto hourly_data = parse_weather_hours(json);
      std::println("\n\nhourly weather:");
      print_weather_hours(hourly_data);
   }
   catch (std::exception const& e) {
      std::println(stderr, "Fehler: {}", e.what());
   }
   return 0;
}
