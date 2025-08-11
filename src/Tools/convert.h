#pragma once

#include <type_traits>
#include <string>
#include <optional>
#include <chrono>
#include <string_view>
#include <format>

// =========================================================================
// Hilfsmetaprogramm: Prüfung auf std::optional
// =========================================================================

template<typename ty>
struct is_std_optional : std::false_type {};

template<typename ty>
struct is_std_optional<std::optional<ty>> : std::true_type {};

template<typename ty>
constexpr bool is_std_optional_v = is_std_optional<ty>::value;

// =========================================================================
// Converter-Primärtemplate und einige Spezialisierungen
// =========================================================================

template<typename To, typename From, typename = void>
struct Converter;  // Primärtemplate – kein Default

// const char* → std::string
template<>
struct Converter<std::string, char const*> {
   static std::string apply(char const* sz) {
      return sz ? std::string(sz) : std::string{};
   }
};

// std::chrono::seconds → std::string (ISO)
template<>
struct Converter<std::string, std::chrono::seconds> {
   static std::string apply(std::chrono::seconds const sec) {
      std::chrono::sys_seconds const tp{ sec };
      return std::format("{:%FT%TZ}", tp);
   }
};

// std::string_view → std::string
template<>
struct Converter<std::string, std::string_view> {
   static std::string apply(std::string_view sv) {
      return std::string(sv);
   }
};

template<typename ty>
concept ConvertibleStringViewViaString =
(!std::same_as<ty, std::string>) &&
   requires(std::string_view sv) {
      { Converter<ty, std::string>::apply(std::string{ sv }) } -> std::same_as<ty>;
};

template<typename ty>
   requires (!std::same_as<ty, std::string>&& ConvertibleStringViewViaString<ty>)
struct Converter<ty, std::string_view> {
   static ty apply(std::string_view sv) {
      return Converter<ty, std::string>::apply(std::string{ sv });
   }
};

// =========================================================================
// robustes ConvertibleTo-Konzept (ohne rekursive Abhängigkeit von convert())
// =========================================================================

template<typename From, typename To>
concept ConvertibleTo =
std::same_as<std::remove_cvref_t<From>, std::remove_cvref_t<To>> ||
std::is_constructible_v<To, From> ||
std::is_convertible_v<From, To> ||
   requires(From && f) {
      { Converter<To, std::remove_cvref_t<From>>::apply(std::forward<From>(f)) } -> std::same_as<To>;
};

// =========================================================================
// zentrale convert<To>(From&&) – nutzt Identität, Konstruktor, cast oder Converter
// =========================================================================

template<typename To, typename From>
To convert(From&& from)
{
   using F = std::remove_cvref_t<From>;
   if constexpr (std::same_as<std::remove_cvref_t<To>, F>) {
      // Identitätskonvertierung
      if constexpr (std::is_reference_v<To>)
         return from;
      else
         return static_cast<To>(from);
   }
   else if constexpr (requires { Converter<To, F>::apply(std::forward<From>(from)); }) {
      // benutzerdefinierter Converter
      return Converter<To, F>::apply(std::forward<From>(from));
   }
   else if constexpr (std::is_constructible_v<To, From>) {
      return To(std::forward<From>(from));
   }
   else if constexpr (std::is_convertible_v<From, To>) {
      return static_cast<To>(std::forward<From>(from));
   }
   else {
      static_assert([] {return false; }(), "No convert<To>(From) available and not implicitly convertible.");
   }
}
