#include "WeatherPrint.h"

#include <formatter_optional.h>  // now in Tools

#include <print>

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

void print(WeatherCurrent const& wc) {
   std::println("Aktuell {:%d.%m.%Y %X}: {:4.1f} °C, Wind {:4.1f} km/h aus {:3.0f}° Code {:2}, Tag: {}",
      wc.timestamp, wc.temperature, wc.windspeed, wc.winddirection, wc.weathercode, wc.is_day);
}

void print(std::vector<WeatherDay> const& data) {
   for (const auto& d : data) {
      std::println("{}: {} {} ({:4.1f}°C / {:4.1f}°C), Sonne: {}, Regen:{:6.1f} mm, UV: {:4.1f} ({:<9}), Code: {:>2} ({})",
         d.date, d.sunrise, d.sunset, d.temp_min, d.temp_max, d.sunshine_duration, d.precipitation_mm,
         d.uv_index, describe_uv_index(d.uv_index), d.weather_code, describe_weather_code(d.weather_code));
   }
}

void print(std::vector<WeatherHour> const& data) {
   for (const auto& wh : data) {
      std::println("{:%d.%m.%Y %X}: Temp {:4.1f} °C, Taupunkt {:4.1f} °C, Wolken: {:3.0f} %, UV: {:3.1f} ({:<9}), Druck: {:4.0f} hPA, Tag: {:<5},  WCode: {:2} ({})",
         wh.timestamp, wh.temperature_2m, wh.dew_point_2m, wh.cloudcover, wh.uv_index, describe_uv_index(wh.uv_index), wh.surface_pressure,
         wh.is_day, wh.weather_code, describe_weather_code(wh.weather_code)
      );
   }
}
