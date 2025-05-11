
// later delete stubs from directory and make this to standard include
#include "OrganizationC.h"

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <tao/PortableServer/LifespanPolicyC.h>
#include <tao/PortableServer/LifespanPolicyA.h>

#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <print>

#include <csignal>

#ifdef _WIN32
   #include <Windows.h>
#endif

std::atomic<bool> shutdown_requested = false;

inline std::string getTimeStamp(std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now()) {
   auto const cz_ts  = std::chrono::current_zone()->to_local(now);
   auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
   return std::format("{:%d.%m.%Y %X},{:03d}", cz_ts, millis);
   }

inline std::chrono::year_month_day convertTo(Organization::YearMonthDay const& ymd) {
   return std::chrono::year_month_day { std::chrono::year  { ymd.year },
                                        std::chrono::month { static_cast<unsigned int>(ymd.month) },
                                        std::chrono::day   { static_cast<unsigned int>(ymd.day) }
                                       };
   }

inline std::chrono::system_clock::time_point from_corba_time_point(Organization::TimePoint const& tp) {
   return std::chrono::system_clock::time_point(std::chrono::milliseconds(tp.milliseconds_since_epoch));
   }

inline std::string toString(CORBA::Exception const& ex) {
   std::ostringstream os;
   os << ex;
   return os.str();
   }

void signal_handler(int sig_num) {
   std::println(std::cout);
   std::println(std::cout, "[signal handler {}] signal {} received. Requesting shutdown ...", getTimeStamp(), sig_num);
   shutdown_requested = true;
   }

int main(int argc, char *argv[]) {
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif


   return 0;
   }
