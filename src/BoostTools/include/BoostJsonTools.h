
#pragma once


// Boost
#include <boost/json/value.hpp>
#include <boost/json/string.hpp>
#include <boost/json/parse.hpp>

#include <boost/system/system_error.hpp> 

// Standardbibliothek
#include <iomanip>        // für std::get_time falls chrono::parse nicht verfügbar ist
#include <string>
#include <sstream>
#include <chrono>
#include <optional>
#include <charconv>
#include <type_traits>
#include <concepts>
#include <stdexcept>

namespace boost_tools {

// Eigene Zeittypen 
using timepoint_ty = std::chrono::sys_seconds;
using date_ty      = std::chrono::year_month_day;
using time_ty      = std::chrono::hh_mm_ss<std::chrono::seconds>;


using namespace std::string_literals;


boost::json::object const& extract_subobject(boost::json::value const& root, std::string_view key) {
   if (!root.is_object())
      throw std::runtime_error("JSON root is not an object");

   auto const& obj = root.as_object();
   if (!obj.contains(key))
      throw std::runtime_error(std::format("Key '{}' not found in JSON object", key));

   auto const& sub = obj.at(key);
   if (!sub.is_object())
      throw std::runtime_error(std::format("Key '{}' does not contain a JSON object", key));

   return sub.as_object();
   }



boost::json::object extract_json_object(std::string_view json_str, std::string_view key) {
   boost::json::value jv;
   try {
      jv = boost::json::parse(json_str);
      }
   catch (std::exception const& ex) {
      throw std::runtime_error(std::format("Invalid JSON: {}", ex.what()));
      }
   return extract_subobject(jv, key);
   }

boost::json::object extract_json_object(std::string_view json_str) {
   try {
      boost::json::value jv = boost::json::parse(json_str);
      return jv.as_object();
      }
   catch (std::exception const& ex) {
      throw std::runtime_error(std::format("Invalid JSON: {}", ex.what()));
      }
   }

// Traits-basierte Delegation
// statt den gesamten Typabgleich in einem Template-Monolithen zu halten, können wir einen 
// Typspezialisierungsmechanismus mit struct value_converter<T> verwenden. 
// Das erlaubt:
// - einfache Erweiterbarkeit für neue Typen via Spezialisierung
// - saubere Trennung der Parsing - Logik
// - bessere Kompilierzeit und Wartbarkeit
//
// Stabile Typisierungsprüfung per SFINAE


// Primärtemplate – greift nur als Fallback
template <typename ty>
struct value_converter {
   static ty convert(boost::json::value const&) {
      static_assert(sizeof(ty) == 0, "No value_converter defined for this type");
   }
};

// Spezialisierung für bool
template <>
struct value_converter<bool> {
   static bool convert(boost::json::value const& value) {
      if (value.is_bool()) [[likely]] return value.as_bool();
      if (value.is_int64())           return value.as_int64() != 0;
      if (value.is_uint64())          return value.as_uint64() != 0;
      throw std::runtime_error("JSON value not convertible to bool");
   }
};

// Spezialisierung für std::string
template <>
struct value_converter<std::string> {
   static std::string convert(boost::json::value const& value) {
      if (value.is_string()) [[likely]] return std::string(value.as_string().c_str());
      throw std::runtime_error("JSON value not convertible to string");
   }
};

// Für unsigned integral types
template <typename ty>
   requires std::is_integral_v<ty>&& std::is_unsigned_v<ty> && !std::is_same_v<ty, bool>
struct value_converter<ty> {
   static ty convert(boost::json::value const& value) {
      if (value.is_uint64()) {
         uint64_t raw = value.as_uint64();
         if (raw > std::numeric_limits<ty>::max())
            throw std::runtime_error("unsigned integer value out of bounds");
         return static_cast<ty>(raw);
         }
      if (value.is_int64()) {
         int64_t raw = value.as_int64();
         if (raw < 0 || static_cast<uint64_t>(raw) > std::numeric_limits<ty>::max())
            throw std::runtime_error("signed value out of range for unsigned target");
         return static_cast<ty>(raw);
         }
      throw std::runtime_error("JSON value not convertible to unsigned integer");
      }
};

// Für signed integral types
template <typename ty>
   requires std::is_integral_v<ty>&& std::is_signed_v<ty> && !std::is_same_v<ty, bool>
struct value_converter<ty> {
   static ty convert(boost::json::value const& value) {
      if (value.is_uint64()) {
         uint64_t raw = value.as_uint64();
         if (raw > static_cast<uint64_t>(std::numeric_limits<ty>::max()))
            throw std::runtime_error("unsigned value out of bounds for signed type");
         return static_cast<ty>(raw);
      }
      if (value.is_int64()) {
         int64_t raw = value.as_int64();
         if (raw < static_cast<int64_t>(std::numeric_limits<ty>::min()) ||
            raw > static_cast<int64_t>(std::numeric_limits<ty>::max()))
            throw std::runtime_error("signed integer out of bounds");
         return static_cast<ty>(raw);
      }
      throw std::runtime_error("JSON value not convertible to signed integer");
   }
};

// Für floating point types
template <typename ty>
   requires std::is_floating_point_v<ty>
struct value_converter<ty> {
   static ty convert(boost::json::value const& value) {
      if (value.is_double()) [[likely]] return static_cast<ty>(value.as_double());
      if (value.is_int64())             return static_cast<ty>(value.as_int64());
      if (value.is_uint64())            return static_cast<ty>(value.as_uint64());

      if (value.is_string()) {
         std::string str = std::string(value.as_string().c_str());

         // Entferne Leerzeichen
         str.erase(std::remove_if(str.begin(), str.end(), [](char ch) {
                                    return std::isspace(static_cast<unsigned char>(ch));
                                    }), str.end());

         // Entferne typische Währungssymbole
         static constexpr std::string_view known_symbols[] = {
            "€", "$", "CHF", "EUR", "USD"
            };
         for (auto const& symbol : known_symbols) {
            if (auto pos = str.find(symbol); pos != std::string::npos) {
               str.erase(pos, symbol.size());
               }
            }

         // Ersetze ',' durch '.' wenn ',' vorkommt und '.' nicht – typischer EU-Stil
         if (str.find(',') != std::string::npos && str.find('.') == std::string::npos) {
            std::replace(str.begin(), str.end(), ',', '.');
            }
         else {
            // Entferne Tausendertrennzeichen (Komma oder Punkt vor dem Dezimalpunkt)
            str.erase(std::remove(str.begin(), str.end(), ','), str.end());
            }

         // Versuch, mit from_chars zu parsen
         ty result{};
         auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
         if (ec == std::errc()) return result;
         }

      throw std::runtime_error("JSON value not convertible to floating point");
   }
};

// Für date_ty
template <>
struct value_converter<date_ty> {
   static date_ty convert(boost::json::value const& value) {
      if (value.is_string()) [[likely]] {
         std::chrono::sys_days tp;
         std::istringstream ss(std::string(value.as_string().c_str()));
         ss >> std::chrono::parse("%Y-%m-%d", tp);
         if (ss.fail()) throw std::runtime_error("Failed to parse date string");
         return std::chrono::year_month_day{ tp };
      }
      throw std::runtime_error("JSON value not convertible to date");
   }
};

// Für time_ty
template <>
struct value_converter<time_ty> {
   static time_ty convert(boost::json::value const& value) {
      if (value.is_string()) [[likely]] {
         std::chrono::local_time<std::chrono::seconds> lt;
         std::istringstream ss(std::string(value.as_string().c_str()));
         ss >> std::chrono::parse("%Y-%m-%dT%H:%M", lt);
         if (ss.fail()) throw std::runtime_error("Failed to parse time string");
         return std::chrono::hh_mm_ss{ lt.time_since_epoch() % std::chrono::days{1} };
         }

      if (value.is_double()) {
         double sec = value.as_double();
         if (sec < 0.0 || sec >= 86400.0)
            throw std::runtime_error(std::format("invalid floating seconds for hh_mm_ss: {}", sec));
         return time_ty{ std::chrono::seconds{ static_cast<int64_t>(sec) } };
         }

      if (value.is_int64()) {
         int64_t const sec = value.as_int64();
         if (sec < 0 || sec >= 86400)
            throw std::runtime_error(std::format("invalid seconds for hh_mm_ss: {}", sec));
         return time_ty{ std::chrono::seconds{ sec } };
         }

      if (value.is_uint64()) {
         uint64_t const sec = value.as_uint64();
         if (sec >= 86400)
            throw std::runtime_error(std::format("invalid seconds for hh_mm_ss: {}", sec));
         return time_ty{ std::chrono::seconds{ sec } };
         }
      throw std::runtime_error("JSON value not convertible to time");
   }
};

// Für timepoint_ty
template <>
struct value_converter<timepoint_ty> {
   static timepoint_ty convert(boost::json::value const& value) {
      if (value.is_string()) [[likely]] {
         std::chrono::sys_seconds tp;
         std::istringstream ss(std::string(value.as_string().c_str()));
         ss >> std::chrono::parse("%Y-%m-%dT%H:%M", tp);
         if (ss.fail()) throw std::runtime_error("Failed to parse timepoint string");
         return tp;
      }
      if (value.is_int64())  return timepoint_ty{ std::chrono::seconds(value.as_int64()) };
      if (value.is_uint64()) return timepoint_ty{ std::chrono::seconds(value.as_uint64()) };
      throw std::runtime_error("JSON value not convertible to timepoint");
   }
};


//  Validierungsstrategie via Policy-Pattern
// Optional können wir  erlauben, dass wir eine Validierung(z.B.Gültigkeit von Datum, Zeitbereich, Wertebereich) 
// über zusätzliche Traits oder Funktionale(Callables) einschleusen.
//
// Das Policy Pattern ist ein idiomatisches C++-Entwurfsmuster, bei dem Verhalten nicht per Vererbung, 
// sondern durch Typparameter (Templates) eingebracht wird. 
// Du trennst das Was (Konvertierung) vom Wie validiere ich das Ergebnis.
//
// Ziel ist es, zusätzlich zur Typkonvertierung auch kontextabhängige Prüfungen durchführbar zu machen, 
// ohne die eigentliche Logik der value_converter<T> zu verändern.So kann man z.B.Datenbereiche, Pflichtwerte, 
// reguläre Ausdrücke, Plausibilitäten etc.prüfen.

template <typename ty>
struct default_validator {
   static void check(ty const&) noexcept {}
   };

/*
Dieser Trick verzögert die Evaluierung des Defaults bis zur Instanziierung – 
damit kann der Compiler Validator klar auflösen, 
ohne Konflikt mit Spezialisierungen auf value_converter<ty, ...>.
*/
template <typename ty>
using validator_for = default_validator<ty>;

template <std::int32_t forecast_days>
struct DateForecastValidator {
   static void check(date_ty const& date) {
      auto const today_tp = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
      std::chrono::year_month_day const today{ std::chrono::sys_days{ today_tp } };
      std::chrono::year_month_day const upper{ std::chrono::sys_days{ today_tp + std::chrono::days{ forecast_days } } };

      if (date < today || date > upper) {
         throw std::runtime_error(std::format("date out of forecast range: allowed = [{} - {}], got = {}",
                                  today, upper, date));
         }
      }
   };

template <auto min_val, auto max_val>
struct IntegerRangeValidator {
   static_assert(std::is_integral_v<decltype(min_val)>, "IntegerRangeValidator requires integral values");
   static_assert(std::is_integral_v<decltype(max_val)>, "IntegerRangeValidator requires integral values");
   static_assert(min_val <= max_val, "min_val must be less than or equal to max_val");

   template <typename ty>
   static void check(ty const& value) {
      static_assert(std::is_integral_v<ty>, "IntegerRangeValidator can only be used with integral types");
      if (value < min_val || value > max_val) {
         throw std::runtime_error(std::format(
            "value out of allowed range: [{} - {}], got = {}",
            min_val, max_val, value));
         }
      }
   };


/**
\brief Konvertiert einen boost::json::value in den gewünschten Typ \c ty

\details
Je nach gewünschtem Zieltyp wird der JSON-Wert ausgewertet und typgerecht konvertiert.
Unterstützt werden aktuell folgende Zieltypen:

- \c bool: direkt aus bool oder implizit aus int
- \c int, \c uint64_t etc.: direkt aus integer
- \c double: direkt oder über implizite Konvertierung
- \c timepoint_ty: entweder als ISO-String ("2024-06-29T13:00") oder als Unix-Zeit
- \c date_ty: ISO-Datum ("2024-06-29")
- \c time_ty: Uhrzeit-Anteil aus ISO-Timestamp ("2024-06-29T13:00")

Die Rückgabe erfolgt optional als \c std::optional<ty> bei \c opt_val = true.

\throw std::runtime_error bei fehlgeschlagenem oder ungültigem Typ-Cast
*/
template <typename ty, bool opt_val = false, typename Validator = validator_for<ty>>
std::conditional_t<opt_val, std::optional<ty>, ty> get_value(boost::json::value const& value) {
   if constexpr (opt_val) {
      if (value.is_null()) return std::nullopt;
      }
   else {
      if (value.is_null()) throw std::runtime_error("unexpeced null value.");
      }
   ty result = value_converter<ty>::convert(value);
   Validator::check(result);
   return result;
   }

template <typename ty, bool opt_val = false, typename Validator = validator_for<ty>>
std::conditional_t<opt_val, std::optional<ty>, ty> get_value(boost::json::object const& current, std::string const& key) {
   if (!current.contains(key)) throw std::runtime_error(std::format("value {} does not exist.", key));
   try {
      return get_value<ty, opt_val, Validator>(current.at(key));
      }
   catch (std::exception const& ex) {
      throw std::runtime_error(std::format("error for field {}: {}", key, ex.what()));
      }
   }

template <typename ty, bool opt_val = false, typename Validator = validator_for<ty>>
std::conditional_t<opt_val, std::optional<ty>, ty> get_value(boost::json::array const& arr, size_t index) {
   if (index >= arr.size() ) throw std::runtime_error(std::format("index {} is out of range (0, {})", index, arr.size() - 1));
   try {
      return  get_value<ty, opt_val, Validator>(arr[index]);
      }
   catch (std::exception const& ex) {
      throw std::runtime_error(std::format("error for field with index {}: {}", index, ex.what()));
      }
   }


} // end of namespace boost_tools

