#pragma once

//#include "WeatherProxy.h"

#include <WeatherS.h>
#include <BasicsC.h>
#include <optional>
#include <string>

class WeatherProxy;
struct WeatherProxyData;

class WeatherService_i : public virtual POA_WeatherAPI::WeatherService {
public:
   explicit WeatherService_i(WeatherProxy const&);

   // Getter-Methoden gem‰ﬂ IDL
   Basics::Optional_Time      sunrise() override;
   Basics::Optional_Time      sunset() override;
   Basics::Optional_Double    temperature() override;
   Basics::Optional_Double    pressure() override;
   Basics::Optional_Double    humidity() override;
   Basics::Optional_Double    precipitation() override;
   Basics::Optional_Double    windspeed() override;
   Basics::Optional_Double    winddirection() override;
   Basics::Optional_Double    cloudcover() override;
   Basics::Optional_Double    uv_index() override;
   Basics::Optional_Long      weathercode() override;
   Basics::Optional_String*   summary() override;  // internal char* forced pointer to corba managed heap

private:
   WeatherProxyData const& mData;
};