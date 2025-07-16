#pragma once

#include <BasicsC.h>
#include <chrono>

template<typename From, typename To, typename = void>
struct Convert;

template<>
struct Convert<Basics::YearMonthDay, std::chrono::year_month_day> {
   static std::chrono::year_month_day apply(Basics::YearMonthDay const& ymd) {
      return { std::chrono::year(ymd.year), std::chrono::month(ymd.month), std::chrono::day(ymd.day) };
      }
   };

template<>
struct Convert<std::chrono::year_month_day, Basics::YearMonthDay> {
   static Basics::YearMonthDay apply(std::chrono::year_month_day const& ymd) {
      return { .year  = static_cast<CORBA::Long>(int(ymd.year())),
               .month = static_cast<CORBA::Short>(unsigned(ymd.month())),
               .day   = static_cast<CORBA::Short>(unsigned(ymd.day()))
             };
      }
   };

template<>
struct Convert<Basics::TimePoint, std::chrono::system_clock::time_point> {
   static std::chrono::system_clock::time_point apply(Basics::TimePoint const& tp) {
      return std::chrono::system_clock::time_point(std::chrono::milliseconds(tp.milliseconds_since_epoch));
      }
   };

template<>
struct Convert<std::chrono::system_clock::time_point, Basics::TimePoint> {
   static Basics::TimePoint apply(std::chrono::system_clock::time_point const& now) {
      return { .milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() };
      }
   };
   
   
template<typename to_ty, typename from_ty>
to_ty convert(from_ty const& from) {
   return Convert<from_ty, to_ty>::apply(from);
   }
