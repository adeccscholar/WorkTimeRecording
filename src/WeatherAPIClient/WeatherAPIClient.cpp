

#include "WeatherC.h"

#include <Corba_Interfaces.h>
#include <BasicTraits.h>

#include <chrono>
#include <print>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char* argv[]) {

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   CORBAClient<WeatherAPI::WeatherService> Client("Weather Client", argc, argv, "GlobalCorp/WeatherAPI");

   if(auto value = Client.get<0>()->temperature(); value.has_value) {
      std::println("Temperature: {:.1f}", value.value);
      }
   if (auto value = Client.get<0>()->pressure(); value.has_value) {
      std::println("Luftdruck: {:.0f} hPa", value.value);
      }
   if (auto value = Client.get<0>()->humidity(); value.has_value) {
      std::println("Luftfeuchtigkeit: {:.1f} %", value.value);
      }
   if (auto value = Client.get<0>()->summary(); value && value->has_value) {
      //std::string test{ value->value }
      std::println("Wetterdaten: {}", value->value.in());
      delete value;
      }

   }