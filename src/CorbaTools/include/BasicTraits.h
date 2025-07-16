// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Generic CORBA trait and accessor framework for optional and non-optional values in CORBA-IDL generated types (TAO-compatible).

\version 1.0
\date    2025-07-13
\author  Volker Hillmann (adecc Systemhaus GmbH)
\copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH

\licenseblock{GPL-3.0-or-later}
This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License, version 3,
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see https://www.gnu.org/licenses/.
\endlicenseblock

\note This file is part of the adecc Scholar project - Free educational materials for modern C++.
*/

#pragma once

#include <BasicsC.h>
#include <type_traits>
#include <chrono>
#include <optional>
#include <string>

// --------------------------------------------------
// Concepts

template<typename ty>
concept CorbaOptionalStruct = requires(ty t) {
   { t.has_value } -> std::convertible_to<CORBA::Boolean>;
   { t.value };
};

template<typename ty>
concept CorbaBuiltin = std::is_arithmetic_v<ty>;

template<typename ty>
concept CorbaValueStruct = std::is_class_v<ty> && !CorbaOptionalStruct<ty> && !CorbaBuiltin<ty>;

// Only CORBA char* is treated as string requiring free
template<typename ty>
concept CorbaCharPointer = std::is_same_v<std::remove_cvref_t<ty>, char*>;

// --------------------------------------------------
// Traits Infrastruktur

template<typename ty>
struct CorbaValueTraitsImpl;

template<typename ty>
struct CorbaValueTraits;

// Builtins

template<CorbaBuiltin ty>
struct CorbaValueTraitsImpl<ty> {
   using value_type = ty;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }
   static void SetValue(value_type& target, value_type const& v) { target = v; }
   static void Reset(value_type& target) { target = value_type{}; }
   };

template<CorbaBuiltin ty>
struct CorbaValueTraits<ty> : CorbaValueTraitsImpl<ty> {};

// char* = CORBA::String
template<>
struct CorbaValueTraitsImpl<char*> {
   using value_type = std::string;
   static bool HasValue(char const* sz) { return sz && sz[0] != '\0'; }
   static std::string GetValue(char const* sz) { return sz ? std::string(sz) : std::string{}; }
   static void SetValue(char*& target, std::string const& str) {
      if (target) CORBA::string_free(target);
      target = CORBA::string_dup(str.c_str());
      }
   static void Reset(char*& target) {
      if (target) { CORBA::string_free(target); target = nullptr; }
      }
   static std::string GetAndFree(char* sz) {
      std::string result = GetValue(sz);
      CORBA::string_free(sz);
      return result;
      }
   };

template<>
struct CorbaValueTraits<char*> : CorbaValueTraitsImpl<char*> {};

template<>
struct CorbaValueTraits<std::string> : CorbaValueTraitsImpl<char*> {};

// Value Structs

template<CorbaValueStruct ty>
struct CorbaValueTraits<ty> {
   using value_type = ty;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }
   static void SetValue(value_type& target, value_type const& v) { target = v; }
   static void Reset(value_type& target) { target = value_type{}; }
   };

// Optional Traits

template<typename T>
struct CorbaOptionalTraits;

template<CorbaOptionalStruct ty>
struct CorbaOptionalTraits<ty> {
   using value_type = std::remove_cvref_t<decltype(std::declval<ty>().value)>;
   static bool HasValue(ty const& opt) { return opt.has_value != 0; }
   static value_type const& GetValue(ty const& opt) { return opt.value; }
   static void SetValue(ty& opt, value_type const& v) { opt.value = v; opt.has_value = true; }
   static void Reset(ty& opt) { opt.value = value_type{}; opt.has_value = false; } 
   };

// Optional_String

template<>
struct CorbaOptionalTraits<Basics::Optional_String> {
   using value_type = std::string;
   static bool HasValue(Basics::Optional_String const& aOpt) {
      return aOpt.has_value != 0 && aOpt.value.in() && aOpt.value.in()[0] != 0;
      }
   static std::string GetValue(Basics::Optional_String const& aOpt) {
      return HasValue(aOpt) ? std::string(aOpt.value.in()) : std::string{};
      }
   static void SetValue(Basics::Optional_String& aOpt, std::string const& s) {
      aOpt.value = CORBA::string_dup(s.c_str());
      aOpt.has_value = !s.empty();
      }
   static void Reset(Basics::Optional_String& aOpt) {
      aOpt.value = static_cast<char const*>(nullptr);
      aOpt.has_value = false;
      }
   };

// TimePoint

template<>
struct CorbaValueTraits<Basics::TimePoint> {
   using value_type = Basics::TimePoint;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }
   static void SetValue(value_type& v, std::chrono::sys_time<std::chrono::milliseconds> const& sysTime) {
      v.milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch()).count();
      }
   static std::chrono::sys_time<std::chrono::milliseconds> AsSysTime(value_type const& v) {
      return std::chrono::sys_time<std::chrono::milliseconds>(std::chrono::milliseconds(v.milliseconds_since_epoch));
      }
   };

// YearMonthDay

template<>
struct CorbaValueTraits<Basics::YearMonthDay> {
   using value_type = Basics::YearMonthDay;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }
   static void SetValue(value_type& v, std::chrono::year_month_day const& ymd) {
      v.year = static_cast<CORBA::Long>(static_cast<int>(ymd.year()));
      v.month = static_cast<CORBA::Short>(unsigned(ymd.month()));
      v.day = static_cast<CORBA::Short>(unsigned(ymd.day()));
      }
   static std::chrono::year_month_day AsYearMonthDay(value_type const& v) {
      return std::chrono::year_month_day(
         std::chrono::year(v.year), std::chrono::month(v.month), std::chrono::day(v.day));
      }
   };

// --------------------------------------------------
// Homogenes Interface


template<typename ty, typename = void>
struct CorbaAccessor;

// ValueTraits

template<typename ty>
struct CorbaAccessor<ty, std::void_t<typename CorbaValueTraits<ty>::value_type>> {
   using traits = CorbaValueTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& v) { return traits::HasValue(v); }
   static value_type Get(ty const& v) { return traits::GetValue(v); }
   static void Set(ty& v, value_type const& val) { traits::SetValue(v, val); }
   static void Reset(ty& v) { traits::Reset(v); }

   template<typename U>
   static ty Return(U const& val) {
      ty result{};
      Set(result, val);
      return result;
      }

   static std::string GetAndFree(ty& value) requires CorbaCharPointer<ty> {
      return CorbaValueTraits<char*>::GetAndFree(value);
      }
   };

// OptionalTraits

template<typename ty>
struct CorbaAccessor<ty, std::void_t<typename CorbaOptionalTraits<ty>::value_type>> {
   using traits = CorbaOptionalTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& v) { return traits::HasValue(v); }
   static value_type Get(ty const& v) { return traits::GetValue(v); }
   static void Set(ty& v, value_type const& val) { traits::SetValue(v, val); }
   static void Reset(ty& v) { traits::Reset(v); }

   template<typename U>
   static ty Return(U const& val) {
      ty result{};
      Set(result, val);
      return result;
      }

   static std::string GetAndFree(ty& value) requires std::is_same_v<ty, Basics::Optional_String> {
      std::string result = Get(value);
      Reset(value);
      return result;
      }
   };

// --------------------------------------------------
// Utility

template<CorbaOptionalStruct Opt>
inline void SetOptionalValue(Opt& opt, typename CorbaOptionalTraits<Opt>::value_type const& val) {
   CorbaOptionalTraits<Opt>::SetValue(opt, val);
   }

template<CorbaOptionalStruct Opt>
inline auto GetOptionalValue(Opt const& opt) {
   return CorbaOptionalTraits<Opt>::GetValue(opt);
   }

template<CorbaValueStruct Val>
inline void SetValue(Val& val, typename CorbaValueTraits<Val>::value_type const& value) {
   CorbaValueTraits<Val>::SetValue(val, value);
   }

template<CorbaValueStruct Val>
inline auto GetValue(Val const& val) {
   return CorbaValueTraits<Val>::GetValue(val);
   }

template<typename ty> requires CorbaBuiltin<ty>
inline void SetValue(ty& v, ty const& val) { v = val; }

template<typename ty> requires CorbaBuiltin<ty>
inline ty GetValue(ty const& v) { return v; }

inline void SetValue(char*& target, std::string const& str) {
   CorbaValueTraits<char*>::SetValue(target, str);
   }

inline std::string GetValue(char const* sz) {
   return CorbaValueTraits<char*>::GetValue(sz);
   }

template<CorbaOptionalStruct Opt>
inline void ResetOptionalValue(Opt& opt) { CorbaOptionalTraits<Opt>::Reset(opt); }

template<CorbaValueStruct Val>
inline void ResetValue(Val& val) { CorbaValueTraits<Val>::Reset(val); }

template<typename ty>
   requires CorbaBuiltin<ty>
inline void ResetValue(ty& v) { v = ty{}; }

inline void ResetValue(char*& sz) { CorbaValueTraits<char*>::Reset(sz); }

template<CorbaOptionalStruct Opt>
auto ToStdOptional(Opt const& aOpt) {
   using value_type = typename CorbaOptionalTraits<Opt>::value_type;
   if (CorbaOptionalTraits<Opt>::HasValue(aOpt))
      return std::optional<value_type>(CorbaOptionalTraits<Opt>::GetValue(aOpt));
   return std::optional<value_type>();
   }

template<typename StdOpt, CorbaOptionalStruct CorbaOpt>
CorbaOpt FromStdOptional(StdOpt const& aOpt) requires requires { aOpt.has_value(); } {
   CorbaOpt result{};
   if (aOpt.has_value())
      CorbaOptionalTraits<CorbaOpt>::SetValue(result, *aOpt);
   else
      CorbaOptionalTraits<CorbaOpt>::Reset(result);
   return result;
   }
