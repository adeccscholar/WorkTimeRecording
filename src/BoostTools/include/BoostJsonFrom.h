#pragma  once

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <format>

namespace boost_tools {

   // Fallback-Tag zum Überschreiben
   struct from_json_tag {};

   // Primäre from_json-Erkennung (SFINAE via ADL)
   template <typename ty, typename = void>
   struct has_from_json : std::false_type {};

   template <typename ty>
   struct has_from_json <ty, std::void_t<decltype(
             from_json(std::declval<ty&>(), std::declval<boost::json::object const&>(), from_json_tag{}))>> : std::true_type {};

   template <typename ty>
   constexpr bool has_from_json_v = has_from_json<ty>::value;

   // Entry Point
   template <typename ty>
   ty from_json(boost::json::value const& jv) {
      if (!jv.is_object())
         throw std::runtime_error("expected JSON object");

      boost::json::object const& obj = jv.as_object();
      ty result;

      if constexpr (has_from_json_v<ty>) {
         from_json(result, obj, from_json_tag{});
         }
      else {
         static_assert(sizeof(ty) == 0, "No from_json overload found for this type.");
         }
      return result;
      }

} // namespace boost_tools
