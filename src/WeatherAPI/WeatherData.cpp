#include "WeatherData.h"

#include <formatter_optional.h>  // now in Tools

#include <string>
#include <optional>
#include <memory>
#include <unordered_map>

inline std::string wind_direction_text(std::optional<double> deg) {
   static constexpr const char* directions[] = {
       "N", "NNO", "NO", "ONO", "O", "OSO", "SO", "SSO",
       "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
      };
   return deg ? directions[static_cast<int>((*deg + 11.25) / 22.5) % 16] : "n/a";
   }

inline std::pair<std::string, std::string> wind_beaufort_text(double speed_kmh) {
   static const RuleSet<double> beaufort = {
       {  0.0,   1.0, {"Windstille", "Calm"}},
       {  1.0,   5.0, {"Leiser Zug", "Light air"}},
       {  5.0,  11.0, {"Leichte Brise", "Light breeze"}},
       { 11.0,  19.0, {"Schwache Brise", "Gentle breeze"}},
       { 19.0,  28.0, {"Mäßige Brise", "Moderate breeze"}},
       { 28.0,  38.0, {"Frische Brise", "Fresh breeze"}},
       { 38.0,  49.0, {"Starker Wind", "Strong breeze"}},
       { 49.0,  61.0, {"Steifer Wind", "Near gale"}},
       { 61.0,  74.0, {"Stürmischer Wind", "Gale"}},
       { 74.0,  88.0, {"Sturm", "Severe gale"}},
       { 88.0, 102.0, {"Schwerer Sturm", "Storm"}},
       {102.0, 117.0, {"Orkanartiger Sturm", "Violent storm"}},
       {117.0, std::numeric_limits<double>::max(), {"Orkan", "Hurricane"}},
      };

   for (const auto& [from, to, msg] : beaufort) {
      if (speed_kmh >= from && speed_kmh < to) {
         return msg;
         }
      }
   return { "Unbekannt", "Unknown" };
   }

std::string describe_uv_index(std::optional<double> uv_index, bool German) {
   static RuleSet<double> const rules = {
        { std::numeric_limits<double>::lowest(), 0.0,  { "Ungültig",  "undefined"   } },
        { 0.0,  2.1,  { "Gering",    "Low"         } },
        { 2.1,  5.1,  { "Mäßig",     "Moderate"    } },
        { 5.1,  7.1,  { "Hoch",      "High"        } },
        { 7.1, 10.1,  { "Sehr hoch", "Very high"   } },
        { 10.1, std::numeric_limits<double>::infinity(), { "Extrem", "Extreme" } }
      };

   /*
   static RuleSet<double> const detailed_rules = {
        { std::numeric_limits<double>::lowest(), 1.0,  { "Sehr gering",   "Very low"     } },
        { 1.0,  2.1,  { "Gering",        "Low"          } },
        { 2.1,  3.6,  { "Mäßig",         "Moderate"     } },
        { 3.6,  5.1,  { "Deutlich",      "Marked"       } },
        { 5.1,  6.6,  { "Hoch",          "High"         } },
        { 6.6,  7.6,  { "Sehr hoch",     "Very high"    } },
        { 7.6,  9.0,  { "Extrem",        "Extreme"      } },
        { 9.0, 11.0,  { "Gefährlich",    "Hazardous"    } },
        { 11.0, std::numeric_limits<double>::infinity(), { "Kritisch", "Critical" } }
      };
   */

   std::ostringstream oss;
   apply_rules(oss, uv_index, rules, German);
   return oss.str();
   }


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


std::string describe_weather_code(std::optional<int> code) {
   if (!code) return "n/a";
   if (auto it = weather_code_descriptions.find(*code); it != weather_code_descriptions.end()) {
      return std::format("{}", it->second.first); // alternativ englisch
      }
   else {
      return std::format("unbekannt code {}", *code);
      }
   }

std::string generate_weather_summary(WeatherCurrentExtended const& wh, bool german) {
   std::ostringstream out;

   // Temperatur
   RuleSet<double> temp_rules = {
       {30.0, 60.0, {"Sehr heiß.", "Very hot."}},
       {25.0, 30.0, {"Warm.", "Warm."}},
       {-50.0, 5.0, {"Kalt.", "Cold."}}
      };
   apply_rules(out, wh.temperature_2m, temp_rules, german);

   // Feuchte (Taupunktdifferenz)
   if (wh.temperature_2m && wh.dew_point_2m) {
      std::optional<double> diff = *wh.temperature_2m - *wh.dew_point_2m;
      RuleSet<double> humidity_rules = {
          {-100.0, 2.5, {"Sehr feuchte, schwüle Luft.", "Very humid."}},
          {2.5, 5.0, {"Hohe Luftfeuchtigkeit.", "Humid."}},
          {10.0, 100.0, {"Trockene Luft.", "Dry air."}}
         };
      apply_rules(out, diff, humidity_rules, german);
      }

   // UV-Index
   RuleSet<double> uv_rules = {
       {6.0, 8.0, {"Hoher UV-Index – Sonnenschutz empfohlen.", "High UV – use sun protection."}},
       {8.0, 12.0, {"Sehr hoher UV-Index! Unbedingt Schutz!", "Very high UV – strong protection needed."}}
      };
   apply_rules(out, wh.uv_index, uv_rules, german);

   // Windstärke (Beaufort)
   if (wh.windspeed_10m) {
      auto [de, en] = wind_beaufort_text(*wh.windspeed_10m);
      out << (german ? de : en) << ' ';
      }

   // Windrichtung
   if (wh.winddirection_10m) {
      out << (german ? "aus Richtung " : "from ") << wind_direction_text(*wh.winddirection_10m) << ". ";
      }

   // Niederschlag
   /*
   RuleSet<double> rain_rules = {
       {5.0, 1000.0, {"Starker Regen möglich.", "Heavy rain possible."}},
       {1.0, 5.0, {"Leichter Regen.", "Light rain."}}
   };
   */
   RuleSet<double> const rain_rules = {
        {    0.0,   10.0, { "Leichter Regen möglich",                "Light rain possible"             } },
        {   10.0,   30.0, { "Mäßiger Regen wahrscheinlich",          "Moderate rain likely"            } },
        {   30.0,   50.0, { "Starker Regen wahrscheinlich",          "Heavy rain likely"               } },
        {   50.0,  100.0, { "Sehr starker Regen (Unwettergefahr)",   "Very heavy rain (severe weather possible)" } },
        {  100.0,  300.0, { "Extremer Regen (z. B. Monsun)",          "Extreme rain (e.g., monsoon)"    } },
        {  300.0, 1800.0, { "Regen im Bereich historischer Rekorde", "Rain within historic world record range" } },
        { 1800.0, std::numeric_limits<double>::infinity(), { "Noch nie gemessene Regenmenge",  "Rain amount never recorded"      } }
      };
   apply_rules(out, wh.precipitation, rain_rules, german);

   // Luftdruck
   RuleSet<double> pressure_rules = {
       {980.0, 1000.0, {"Tiefer Luftdruck. Möglicherweise instabil.", "Low pressure. Unstable weather possible."}},
       {1025.0, 1060.0, {"Hoher Luftdruck. Stabil und trocken.", "High pressure. Stable and dry."}}
      };
   apply_rules(out, wh.pressure_msl, pressure_rules, german);


   // Wettercodewarnung (z. B. Gewitter)
   if (wh.weather_code) {
      RuleSet<int> weathercode_alerts = {
          {95, 100, {"Gewittergefahr!", "Thunderstorm risk!"}},
          {61, 66, {"Regenwetter.", "Rainy weather."}},
          {71, 76, {"Schneefall möglich.", "Snow possible."}},
          {85, 87, {"Starker Schneefall!", "Heavy snowfall!"}},
          {45, 49, {"Nebel kann die Sicht beeinträchtigen.", "Fog may reduce visibility."}}
         };
      apply_rules(out, wh.weather_code, weathercode_alerts, german);
      }

   // Strahlung
   RuleSet<double> radiation_rules = {
       {500.0, 800.0, {"Starke Sonneneinstrahlung.", "Strong sunlight."}},
       {800.0, 2000.0, {"Sehr starke Sonnenstrahlung. Schutzmaßnahmen nötig!", "Very intense radiation – protection needed!"}}
      };
   apply_rules(out, wh.shortwave_radiation, radiation_rules, german);

   return out.str().empty() ? (german ? "Keine besonderen Wettererscheinungen." : "No special weather conditions.") : out.str();
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

void print(WeatherCurrent const& wc) {
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

void print(std::vector<WeatherHour> const& data) {
   for (const auto& wh : data) {
      std::println("{:%d.%m.%Y %X}: Temp {:4.1f} °C, Taupunkt {:4.1f} °C, Wolken: {:3.0f} %, UV: {:3.1f} ({:<9}), Druck: {:4.0f} hPA, Tag: {:<5},  WCode: {:2} ({})",
         wh.timestamp, wh.temperature_2m, wh.dew_point_2m, wh.cloudcover, wh.uv_index, describe_uv_index(wh.uv_index), wh.surface_pressure,
         wh.is_day, wh.weather_code, describe_weather_code(wh.weather_code)
         );
      }
   }
