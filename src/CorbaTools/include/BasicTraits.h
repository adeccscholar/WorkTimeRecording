#pragma once

#include "BasicUtils.h"

#include <BasicsC.h>
#include <type_traits>
#include <string>
#include <chrono>

/// \file
/// \brief CORBA-Trait-Framework für Zugriff auf optionale und nicht-optionale CORBA-Typen.
/// \details Dieses Framework stellt ein homogenes Interface für den Zugriff auf
///          CORBA-generierte Structs und Builtins bereit, ohne Wissen über konkrete Typen.
/// \note Optional-Logik wird über dedizierte CorbaOptionalTraits<> abgebildet.



// ============================================================================
// Value Traits (nicht optional)
// ============================================================================

template<typename ty>
struct CorbaValueTraitsImpl;

template<typename ty>
struct CorbaValueTraits;

// Builtin-Typen
template<CorbaBuiltin ty>
struct CorbaValueTraitsImpl<ty> {
   using value_type = ty;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }

   template<typename U> requires ConvertibleTo<U, value_type>
   static void SetValue(value_type& target, U const& val) {
      target = convert<value_type>(val);
   }

   static void Reset(value_type& target) { target = ty{}; }
};

template<CorbaBuiltin ty>
struct CorbaValueTraits<ty> : CorbaValueTraitsImpl<ty> {};

// Strings (char*)
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
      if (target) {
         CORBA::string_free(target);
         target = nullptr;
      }
   }

   static std::string GetAndFree(char*& sz) {
      std::string result = GetValue(sz);
      Reset(sz);
      return result;
   }
};

template<>
struct CorbaValueTraits<char*> : CorbaValueTraitsImpl<char*> {};

template<>
struct CorbaValueTraits<std::string> : CorbaValueTraitsImpl<char*> {};

// Value-Structs
template<CorbaValueStruct ty>
struct CorbaValueTraits<ty> {
   using value_type = ty;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }

   template<typename U> requires ConvertibleTo<U, value_type>
   static void SetValue(value_type& target, U const& val) {
      target = convert<value_type>(val);
   }

   static void Reset(value_type& target) { target = value_type{}; }
};


// ============================================================================
// Optional Traits
// ============================================================================

template<typename T>
struct CorbaOptionalTraits;

template<CorbaOptionalStruct ty>
struct CorbaOptionalTraits<ty> {
   using value_type = std::remove_cvref_t<decltype(std::declval<ty>().value)>;

   static bool HasValue(ty const& opt) { return opt.has_value != 0; }
   static value_type const& GetValue(ty const& opt) { return opt.value; }

   template<typename U> requires ConvertibleTo<U, value_type>
   static void SetValue(ty& opt, U const& val) {
      opt.value = convert<value_type>(val);
      opt.has_value = true;
   }

   static void Reset(ty& opt) {
      opt.value = value_type{};
      opt.has_value = false;
   }
};

// Optional_String (explizit)
template<>
struct CorbaOptionalTraits<Basics::Optional_String> {
   using value_type = std::string;

   static bool HasValue(Basics::Optional_String const& opt) {
      return opt.has_value != 0 && opt.value.in() && opt.value.in()[0] != '\0';
   }

   static std::string GetValue(Basics::Optional_String const& opt) {
      return HasValue(opt) ? std::string(opt.value.in()) : std::string{};
   }

   static void SetValue(Basics::Optional_String& opt, std::string const& s) {
      opt.value = CORBA::string_dup(s.c_str());
      opt.has_value = !s.empty();
   }

   static void Reset(Basics::Optional_String& opt) {
      opt.value = static_cast<char const*>(nullptr);
      opt.has_value = false;
   }
};


// ============================================================================
// Traits-Schnittstelle: CorbaAccessor für value und optional
// ============================================================================

template<typename ty, typename = void>
struct CorbaAccessor;

// ----------------------------------------------------------
// Accessor für normale CORBA Value-Typen (auch Strings)
// ----------------------------------------------------------
template<typename ty>
   requires requires { typename CorbaValueTraits<ty>::value_type; }
struct CorbaAccessor<ty> {
   using traits = CorbaValueTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& val) {
      return traits::HasValue(val);
   }

   static value_type Get(ty const& val) {
      return traits::GetValue(val);
   }

   template<typename U>
      requires ConvertibleTo<U, value_type>
   static void Set(ty& target, U const& val) {
      traits::SetValue(target, val);
   }

   static void Reset(ty& val) {
      traits::Reset(val);
   }

   static ty Return(value_type const& val) {
      ty result{};
      Set(result, val);
      return result;
   }

   static std::string GetAndFree(ty& value) requires CorbaCharPointer<ty> {
      return CorbaValueTraits<char*>::GetAndFree(value);
   }
};

// ----------------------------------------------------------
// Accessor für CORBA Optional-Typen
// ----------------------------------------------------------
template<typename ty>
   requires requires { typename CorbaOptionalTraits<ty>::value_type; }
struct CorbaAccessor<ty> {
   using traits = CorbaOptionalTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& val) {
      return traits::HasValue(val);
   }

   static value_type Get(ty const& val) {
      return traits::GetValue(val);
   }

   template<typename U>
      requires ConvertibleTo<U, value_type>
   static void Set(ty& target, U const& val) {
      traits::SetValue(target, val);
   }

   static void Reset(ty& val) {
      traits::Reset(val);
   }

   template<typename U>
      requires ConvertibleTo<U, value_type> || is_std_optional_v<U>
   static ty Return(U const& val) {
      ty result{};
      if constexpr (is_std_optional_v<U>) {
         if (val.has_value())
            Set(result, *val);
         else
            Reset(result);
      }
      else {
         Set(result, val);
      }
      return result;
   }

   static std::string GetAndFree(ty& value) requires std::is_same_v<ty, Basics::Optional_String> {
      std::string result = Get(value);
      Reset(value);
      return result;
   }
};