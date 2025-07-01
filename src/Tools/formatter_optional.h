#pragma once

#include <format>
#include <optional>

#include <chrono>
#include <string_view>
#include <type_traits>

template <typename ty>
constexpr ty formatter_dummy_value() {
   if constexpr (std::is_same_v<ty, std::chrono::system_clock::time_point>) {
      return std::chrono::system_clock::now();
      }
   else if constexpr (std::is_same_v<ty, std::chrono::hh_mm_ss<std::chrono::seconds>>) {
      auto now = std::chrono::system_clock::now();
      auto time_today = now - std::chrono::floor<std::chrono::days>(now);
      return std::chrono::hh_mm_ss<std::chrono::seconds>{std::chrono::floor<std::chrono::seconds>(time_today)};
      }
   else if constexpr (std::is_same_v<ty, std::chrono::year_month_day>) {
      auto now = std::chrono::system_clock::now();
      //auto today = std::chrono::floor<std::chrono::days>(now);
      return std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(now) };
      }
   else {
      return ty{};
      }
   }

template <typename ty>
struct std::formatter<std::optional<ty>, char> {
   std::formatter<ty, char> value_formatter; // für den enthaltenen Typ
   std::basic_string_view<char> format_str;

   constexpr auto parse(std::format_parse_context& ctx) {
      auto it = value_formatter.parse(ctx);
      format_str = { ctx.begin(), it };
      return it;
      }

   template <typename FormatContext>
   auto format(const std::optional<ty>& opt, FormatContext& ctx) const {
      std::string fmt = std::format("{{:{}}}", format_str);  // funktioniert auch bei leerem format_str

      if (opt) {
         return std::vformat_to(ctx.out(), fmt, std::make_format_args(*opt));
         }
      else {
         static constexpr std::string_view fallback = "n/a";
         ty dummy = formatter_dummy_value<ty>();

         std::string dummy_output = std::vformat(fmt, std::make_format_args(dummy));
         std::size_t width = dummy_output.size();

         if (width > fallback.size()) {
            std::string padding(width - fallback.size(), ' ');
            return std::format_to(ctx.out(), "{}{}", padding, fallback);
            }
         else {
            return std::format_to(ctx.out(), "{}", fallback);
            }
         }
      }
   };
