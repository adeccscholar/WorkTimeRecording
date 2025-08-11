#pragma once


#include <utility>
#include <convert.h>
#include <BasicsC.h>

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


/// \brief Zentrale Konvertierungsfunktion
/// \details Behandelt Optional-Werte (std::optional und CORBA Optionals) rekursiv und delegiert dann an Convert<To,From>

template<typename To, typename From>
To convert(From const& from) {
   // ----------------------------------------------------------------------
   // Fall: std::optional → etwas
   // ----------------------------------------------------------------------
   if constexpr (is_std_optional_v<From>) {
      static_assert(!std::is_same_v<To, From>, "convert<To>(optional<T>) zu optional<T> ist nicht erlaubt.");

      if constexpr (is_std_optional_v<To>) {
         using inner = typename To::value_type;
         return from.has_value() ? To{ convert<inner>(*from) } : std::nullopt;
      }
      else {
         if (!from.has_value())
            throw std::invalid_argument("convert<To>(std::optional) auf leeres optional");
         return convert<To>(*from);
      }
   }

   // ----------------------------------------------------------------------
   // Fall: CORBA Optional → etwas
   // ----------------------------------------------------------------------
   else if constexpr (CorbaOptionalStruct<From>) {
      static_assert(!std::is_same_v<To, From>, "convert<To>(CorbaOptional<T>) zu CorbaOptional<T> ist nicht erlaubt.");

      if constexpr (is_std_optional_v<To>) {
         using inner = typename To::value_type;
         return from.has_value ? To{ convert<inner>(from.value) } : std::nullopt;
      }
      else {
         if (!from.has_value)
            throw std::invalid_argument("convert<To>(CorbaOptional) auf leeres optional");
         return convert<To>(from.value);
      }
   }

   // ----------------------------------------------------------------------
   // Fall: CORBA Optional* → etwas
   // ----------------------------------------------------------------------
   else if constexpr (std::is_pointer_v<From> && CorbaOptionalStruct<std::remove_pointer_t<From>>) {
      using InnerCorba = std::remove_pointer_t<From>;
      static_assert(!std::is_same_v<To, InnerCorba>, "convert<To>(CorbaOptional<T>*) zu CorbaOptional<T> ist nicht erlaubt.");

      if (!from)
         throw std::invalid_argument("convert<To>(CORBA optional*) auf null");

      if constexpr (is_std_optional_v<To>) {
         using inner = typename To::value_type;
         return from->has_value ? To{ convert<inner>(from->value) } : std::nullopt;
      }
      else {
         if (!from->has_value)
            throw std::invalid_argument("convert<To>(CORBA optional*) auf leeres optional");
         return convert<To>(from->value);
      }
   }

   // ----------------------------------------------------------------------
   // Fall: etwas → std::optional
   // ----------------------------------------------------------------------
   else if constexpr (is_std_optional_v<To>) {
      using inner = typename To::value_type;
      return To{ convert<inner>(from) };
   }

   // ----------------------------------------------------------------------
   // Fall: etwas → CORBA Optional
   // ----------------------------------------------------------------------
   else if constexpr (CorbaOptionalStruct<To>) {
      using inner = decltype(To::value); // Struktur garantiert value
      To result{};
      result.has_value = true;
      result.value = convert<inner>(from);
      return result;
   }

   // ----------------------------------------------------------------------
   // Direkter Fall: atomare Konvertierung
   // ----------------------------------------------------------------------
   else {
      return Converter<To, From>::apply(from);
   }
}


// =========================================================================
// Beispielkonvertierungen für CORBA-Basistypen und C++-Typen
// =========================================================================

// CORBA::Long → int
template<>
struct Converter<int, CORBA::Long> {
   static int apply(CORBA::Long from) { return static_cast<int>(from); }
};

template<typename T>
   requires (!std::is_same_v<CORBA::Long, T>&&
std::is_arithmetic_v<T>&&
std::is_convertible_v<T, CORBA::Long>)
struct Converter<CORBA::Long, T> {
   static CORBA::Long apply(T from) {
      return static_cast<CORBA::Long>(from);
   }
};

// CORBA::Double → double
template<>
struct Converter<double, CORBA::Double> {
   static double apply(CORBA::Double from) { return static_cast<double>(from); }
};

// double → CORBA::Double
template<typename T>
   requires (std::is_arithmetic_v<T> &&
!std::is_same_v<CORBA::Double, T>&&
std::is_convertible_v<T, CORBA::Double>)
struct Converter<CORBA::Double, T> {
   static CORBA::Double apply(T from) {
      return static_cast<CORBA::Double>(from);
   }
};




template<>
struct Converter<std::string, TAO::String_Manager> {
   static std::string apply(TAO::String_Manager const& str) {
      return Converter<std::string, char const*>::apply(str.in());
   }
};


// std::string → CORBA::String (char*) – NICHT freigeben hier!
template<>
struct Converter<char*, std::string> {
   static char* apply(std::string const& str) {
      return CORBA::string_dup(str.c_str());
   }
};


// =========================================================================
// Konvertierung Basics::Date ↔ std::chrono::year_month_day
// =========================================================================

template<>
struct Converter<std::chrono::year_month_day, Basics::Date> {
   static std::chrono::year_month_day apply(Basics::Date const& d) {
      return std::chrono::year{ d.year } /
         std::chrono::month{ static_cast<unsigned>(d.month) } /
         std::chrono::day{ static_cast<unsigned>(d.day) };
   }
};

template<>
struct Converter<Basics::Date, std::chrono::year_month_day> {
   static Basics::Date apply(std::chrono::year_month_day const& ymd) {
      Basics::Date result;
      result.year = int(ymd.year());
      result.month = static_cast<short>(unsigned(ymd.month()));
      result.day = static_cast<short>(unsigned(ymd.day()));
      return result;
   }
};

// =========================================================================
// Konvertierung Basics::Time ↔ std::chrono::milliseconds
// =========================================================================

template<>
struct Converter<std::chrono::milliseconds, Basics::Time> {
   static std::chrono::milliseconds apply(Basics::Time const& t) {
      return std::chrono::milliseconds{ t.milliseconds };
   }
};

template<>
struct Converter<Basics::Time, std::chrono::milliseconds> {
   static Basics::Time apply(std::chrono::milliseconds const& ms) {
      Basics::Time t;
      t.milliseconds = ms.count();
      return t;
   }
};

/// \brief Konvertiert std::chrono::hh_mm_ss<Dur> nach Basics::Time als Millisekunden seit 00:00
template <typename Rep, typename Period>
struct Converter<Basics::Time, std::chrono::hh_mm_ss<std::chrono::duration<Rep, Period>>> {
   static Basics::Time apply(std::chrono::hh_mm_ss<std::chrono::duration<Rep, Period>> const& t) {
      using std::chrono::duration_cast;
      using ms = std::chrono::milliseconds;
      Basics::Time r{};
      r.milliseconds = static_cast<long long>(duration_cast<ms>(t.to_duration()).count());
      return r;
   }
};

// =========================================================================
// Konvertierung: std::chrono::hh_mm_ss <-> Basics::Time
// =========================================================================

template<>
struct Converter<Basics::Time, std::chrono::hh_mm_ss<std::chrono::seconds>> {
   static Basics::Time apply(std::chrono::hh_mm_ss<std::chrono::seconds> const& from) {
      using namespace std::chrono;
      auto total = hours(from.hours()) + minutes(from.minutes()) + seconds(from.seconds());
      return Basics::Time{ duration_cast<milliseconds>(total).count() };
   }
};

template<>
struct Converter<std::chrono::hh_mm_ss<std::chrono::seconds>, Basics::Time> {
   static std::chrono::hh_mm_ss<std::chrono::seconds> apply(Basics::Time const& from) {
      using namespace std::chrono;
      seconds secs{ duration_cast<seconds>(milliseconds(from.milliseconds)).count() };
      return std::chrono::hh_mm_ss<std::chrono::seconds>{secs};
   }
};

// =========================================================================
// Konvertierung Basics::TimePoint ↔ std::chrono::system_clock::time_point
// =========================================================================

template<>
struct Converter<std::chrono::system_clock::time_point, Basics::TimePoint> {
   static std::chrono::system_clock::time_point apply(Basics::TimePoint const& tp) {
      return std::chrono::system_clock::time_point{ std::chrono::milliseconds{tp.milliseconds_since_epoch} };
   }
};

template<>
struct Converter<Basics::TimePoint, std::chrono::system_clock::time_point> {
   static Basics::TimePoint apply(std::chrono::system_clock::time_point const& tp) {
      Basics::TimePoint result;
      result.milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
      return result;
   }
};

// =========================================================================
// convert ist jetzt EXKLUSIV für direkte Typen reserviert
// =========================================================================
// Optional-Handling und Wrapper-Zuständigkeiten liegen ausschliesslich
// in den umgebenden Strukturen (z.B. CorbaValueWrapper)
