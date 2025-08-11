

#include "WeatherC.h"
#include <convert.h>

#include <Corba_Interfaces.h>
#include <CorbaAccessor.h>

#include <chrono>
#include <print>

#ifdef _WIN32
#include <Windows.h>
#endif

[[nodiscard]] std::chrono::seconds nowInSeconds() {
   return std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch());
   }

int main(int argc, char* argv[]) {

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

   auto sec = nowInSeconds();
   std::println("{}", Converter<std::string, std::chrono::seconds>::apply(sec));

   try {
      CORBAClient<WeatherAPI::WeatherService> Client("Weather Client", argc, argv, "GlobalCorp/WeatherAPI");
      auto weather = [&Client]() { return Client.get<0>(); };

      if(auto value = CorbaValueWrapper<double>(weather()->temperature()); value.has_value()) {
         std::println("Temperature: {:.1f}", value.value());
         }

      if (auto value = CorbaValueWrapper<double>(weather()->pressure()); value.has_value()) {
         std::println("Luftdruck: {:.0f} hPa", value.value());
         }
      if (auto value = CorbaValueWrapper<double>(weather()->humidity()); value.has_value()) {
         std::println("Luftfeuchtigkeit: {:.1f} %", value.value());
         }
      /// \todo wrapper
      if (auto value = CorbaValueWrapper<std::string>(*weather()->summary()); value.has_value()) {
         std::println("Wetterdaten: {}", value.value());
         }
      }
   catch(std::exception const& ex) {
      std::println("C++ exception: ", ex.what());
      }

   /*
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
   */
   return 0;
   }