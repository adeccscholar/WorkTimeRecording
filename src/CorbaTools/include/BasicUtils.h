#pragma once

#include <BasicsC.h>
#include <chrono>
#include <optional>

/// \file
/// \brief Typkonvertierungen zwischen C++-Standardtypen und CORBA-Basistypen
/// \details Diese Datei enthält ausschließlich typische Konvertierungsoperationen
///          auf Wertebene ohne Logik für CORBA-Traits, Speicherverwaltung oder
///          CORBA-Zugriff.
/// \note Diese Datei darf keine Abhängigkeiten zu Traits enthalten.
///       Optionales Mapping erfolgt ausschließlich über `convert<std::optional<T>, U>`.

// ----------------------------------------------------------------------------
// Hilfskonzept: is_optional
// ----------------------------------------------------------------------------

template<typename T>
struct is_std_optional : std::false_type {};

template<typename T>
struct is_std_optional<std::optional<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

// ----------------------------------------------------------------------------
// convert<From, To> generische Auflösung
// ----------------------------------------------------------------------------

template<typename From, typename To, typename = void>
struct Convert;

// ----------------------------------------------------------------------------
// Fallback: identische Typen (z. B. double → double, int → int, etc.)
// ----------------------------------------------------------------------------
template<typename ty>
struct Convert<ty, ty> {
   static ty apply(ty const& val) { return val; }
   };

template<typename to_ty, typename from_ty>
to_ty convert(from_ty const& from) {
   return Convert<from_ty, to_ty>::apply(from);
}

// ----------------------------------------------------------------------------
// Basics::Date ↔ std::chrono::year_month_day
// ----------------------------------------------------------------------------

template<>
struct Convert<Basics::Date, std::chrono::year_month_day> {
   static std::chrono::year_month_day apply(Basics::Date const& date) {
      using namespace std::chrono;
      return year_month_day{
         year{static_cast<int>(date.year)},
         month{static_cast<unsigned>(date.month)},
         day{static_cast<unsigned>(date.day)}
      };
   }
};

template<>
struct Convert<std::chrono::year_month_day, Basics::Date> {
   static Basics::Date apply(std::chrono::year_month_day const& ymd) {
      return {
         .year = static_cast<CORBA::Long>(int(ymd.year())),
         .month = static_cast<CORBA::Short>(unsigned(ymd.month())),
         .day = static_cast<CORBA::Short>(unsigned(ymd.day()))
      };
   }
};

// ----------------------------------------------------------------------------
// Basics::TimePoint ↔ std::chrono::system_clock::time_point
// ----------------------------------------------------------------------------

template<>
struct Convert<Basics::TimePoint, std::chrono::system_clock::time_point> {
   static std::chrono::system_clock::time_point apply(Basics::TimePoint const& tp) {
      using namespace std::chrono;
      return system_clock::time_point{ milliseconds{tp.milliseconds_since_epoch} };
   }
};

template<>
struct Convert<std::chrono::system_clock::time_point, Basics::TimePoint> {
   static Basics::TimePoint apply(std::chrono::system_clock::time_point const& tp) {
      using namespace std::chrono;
      return { .milliseconds_since_epoch = duration_cast<milliseconds>(tp.time_since_epoch()).count() };
   }
};

// ----------------------------------------------------------------------------
// std::chrono::local_time<Duration> ↔ Basics::TimePoint
// ----------------------------------------------------------------------------

template<typename Duration>
struct Convert<std::chrono::local_time<Duration>, Basics::TimePoint> {
   static Basics::TimePoint apply(std::chrono::local_time<Duration> const& lt) {
      using namespace std::chrono;
      auto sysTime = clock_cast<system_clock>(time_point<local_t, Duration>{lt.time_since_epoch()});
      auto sinceEpoch = duration_cast<milliseconds>(sysTime.time_since_epoch());
      return Basics::TimePoint{ static_cast<decltype(Basics::TimePoint::milliseconds_since_epoch)>(sinceEpoch.count()) };
   }
};

template<typename Duration>
struct Convert<Basics::TimePoint, std::chrono::local_time<Duration>> {
   static std::chrono::local_time<Duration> apply(Basics::TimePoint const& tp) {
      using namespace std::chrono;
      return local_time<Duration>{ duration_cast<Duration>(milliseconds(tp.milliseconds_since_epoch)) };
   }
};

// ----------------------------------------------------------------------------
// std::chrono::sys_time<Duration> ↔ Basics::TimePoint
// ----------------------------------------------------------------------------

template<typename Duration>
struct Convert<std::chrono::sys_time<Duration>, Basics::TimePoint> {
   static Basics::TimePoint apply(std::chrono::sys_time<Duration> const& st) {
      using namespace std::chrono;
      auto sinceEpoch = duration_cast<milliseconds>(st.time_since_epoch());
      return Basics::TimePoint{ static_cast<decltype(Basics::TimePoint::milliseconds_since_epoch)>(sinceEpoch.count()) };
   }
};

template<typename Duration>
struct Convert<Basics::TimePoint, std::chrono::sys_time<Duration>> {
   static std::chrono::sys_time<Duration> apply(Basics::TimePoint const& tp) {
      using namespace std::chrono;
      return sys_time<Duration>{ duration_cast<Duration>(milliseconds(tp.milliseconds_since_epoch)) };
   }
};

// ----------------------------------------------------------------------------
// std::chrono::hh_mm_ss<Duration> ↔ Basics::Time
// ----------------------------------------------------------------------------

template<typename Duration>
struct Convert<std::chrono::hh_mm_ss<Duration>, Basics::Time> {
   static Basics::Time apply(std::chrono::hh_mm_ss<Duration> const& hms) {
      using namespace std::chrono;
      return Basics::Time{
         .milliseconds = duration_cast<milliseconds>(
            hms.hours() + hms.minutes() + hms.seconds() + hms.subseconds()
         ).count()
      };
   }
};

template<typename Duration>
struct Convert<Basics::Time, std::chrono::hh_mm_ss<Duration>> {
   static std::chrono::hh_mm_ss<Duration> apply(Basics::Time const& t) {
      using namespace std::chrono;
      return hh_mm_ss<Duration>{ duration_cast<Duration>(milliseconds{ t.milliseconds }) };
   }
};

// ----------------------------------------------------------------------------
// Generische chrono::time_point<Clock, Duration> ↔ Basics::TimePoint
// ----------------------------------------------------------------------------

template<typename Clock, typename Duration>
struct Convert<std::chrono::time_point<Clock, Duration>, Basics::TimePoint> {
   static Basics::TimePoint apply(std::chrono::time_point<Clock, Duration> const& aTime) {
      auto const sysTime = std::chrono::clock_cast<std::chrono::system_clock>(aTime);
      auto const sinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch());
      return Basics::TimePoint{
         .milliseconds_since_epoch = static_cast<decltype(Basics::TimePoint::milliseconds_since_epoch)>(sinceEpoch.count())
      };
   }
};

template<typename Clock, typename Duration>
struct Convert<Basics::TimePoint, std::chrono::time_point<Clock, Duration>> {
   static std::chrono::time_point<Clock, Duration> apply(Basics::TimePoint const& tp) {
      using namespace std::chrono;
      return time_point<Clock, Duration>{ duration_cast<Duration>(milliseconds(tp.milliseconds_since_epoch)) };
   }
};

// ----------------------------------------------------------------------------
// Optional: std::optional<T> → std::optional<U>
// ----------------------------------------------------------------------------

template<typename T, typename U>
   requires requires (T const& t) { convert<U>(t); }
struct Convert<std::optional<T>, std::optional<U>> {
   static std::optional<U> apply(std::optional<T> const& opt) {
      if (opt) return std::optional<U>{ convert<U>(*opt) };
      return std::nullopt;
   }
};

template<typename T, typename U>
   requires requires (T const& t) { convert<U>(t); }
struct Convert<T, std::optional<U>> {
   static std::optional<U> apply(T const& val) {
      return std::optional<U>{ convert<U>(val) };
   }
};