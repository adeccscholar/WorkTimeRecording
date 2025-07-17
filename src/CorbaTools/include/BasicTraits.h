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
// Konzepte
// ============================================================================

template<typename ty>
concept CorbaOptionalStruct = requires(ty t) {
   { t.has_value } -> std::convertible_to<CORBA::Boolean>;
   { t.value };
};

template<typename ty>
concept CorbaBuiltin = std::is_arithmetic_v<ty>;

template<typename ty>
concept CorbaCharPointer = std::is_same_v<std::remove_cvref_t<ty>, char*>;

template<typename ty>
concept CorbaValueStruct = std::is_class_v<ty> && !CorbaOptionalStruct<ty> && !CorbaBuiltin<ty>;


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
   template<typename U>
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

// CORBA Value-Structs (inkl. TimePoint)

template<CorbaValueStruct ty>
struct CorbaValueTraits<ty> {
   using value_type = ty;
   static bool HasValue(value_type const&) { return true; }
   static value_type const& GetValue(value_type const& v) { return v; }
   static void SetValue(value_type& target, value_type const& v) { target = v; }
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

   template<typename U>
   static void SetValue(ty& opt, U const& val) {
      opt.value = convert<value_type>(val);
      opt.has_value = true;
   }


   static void Reset(ty& opt) {
      opt.value = value_type{};
      opt.has_value = false;
   }
};

// Optional_String (speziell)

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
// Traits-Schnittstelle (CorbaAccessor)
// ============================================================================

template<typename ty, typename = void>
struct CorbaAccessor;

// Value-Typen

template<typename ty>
struct CorbaAccessor<ty, std::void_t<typename CorbaValueTraits<ty>::value_type>> {
   using traits = CorbaValueTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& v) { return traits::HasValue(v); }
   static value_type Get(ty const& v) { return traits::GetValue(v); }
   static void Set(ty& v, value_type const& val) { traits::SetValue(v, val); }
   static void Reset(ty& v) { traits::Reset(v); }

   template<typename to_ty>
   struct CorbaOptionalTraits {
      template<typename from_ty>
      static void SetValue(to_ty& aTarget, from_ty const& aValue) {
         if constexpr (is_std_optional_v<from_ty>) {
            if (aValue)
               aTarget = convert<typename to_ty::value_type>(*aValue);
            else
               aTarget = to_ty{}; // leer
         }
         else {
            aTarget = convert<typename to_ty::value_type>(aValue);
         }
      }
   };

   static std::string GetAndFree(ty& value) requires CorbaCharPointer<ty> {
      return CorbaValueTraits<char*>::GetAndFree(value);
   }
};

// Optional-Typen

template<typename ty>
struct CorbaAccessor<ty, std::void_t<typename CorbaOptionalTraits<ty>::value_type>> {
   using traits = CorbaOptionalTraits<ty>;
   using value_type = typename traits::value_type;

   static bool Has(ty const& v) { return traits::HasValue(v); }
   static value_type Get(ty const& v) { return traits::GetValue(v); }

   template<typename U>
   static void Set(ty& v, U const& val) { traits::SetValue(v, val); }
   static void Reset(ty& v) { traits::Reset(v); }

   template<typename U>
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


// ============================================================================
// Utility-Funktionen (Inline, generisch)
// ============================================================================

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



// Umwandlung zu std::optional

template<CorbaOptionalStruct Opt>
inline auto ToStdOptional(Opt const& opt) {
   using value_type = typename CorbaOptionalTraits<Opt>::value_type;
   if (CorbaOptionalTraits<Opt>::HasValue(opt))
      return std::optional<value_type>(CorbaOptionalTraits<Opt>::GetValue(opt));
   return std::optional<value_type>();
}

template<typename StdOpt, CorbaOptionalStruct CorbaOpt>
CorbaOpt FromStdOptional(StdOpt const& opt) requires requires { opt.has_value(); } {
   CorbaOpt result{};
   if (opt.has_value())
      CorbaOptionalTraits<CorbaOpt>::SetValue(result, *opt);
   else
      CorbaOptionalTraits<CorbaOpt>::Reset(result);
   return result;
}
