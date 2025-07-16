#include "Weather_i.h"

#include <BasicTraits.h>

// 
// using namespace WeatherAPI;

Basics::Optional_TimePoint WeatherService_i::sunrise() {
   return FromStdOptional<decltype(mData.sunrise), Basics::Optional_TimePoint>(mData.sunrise);
}

Basics::Optional_TimePoint WeatherService_i::sunset() {
   return FromStdOptional<decltype(mData.sunset), Basics::Optional_TimePoint>(mData.sunset);
}

Basics::Optional_Double WeatherService_i::temperature() {
   return FromStdOptional<decltype(mData.temperature), Basics::Optional_Double>(mData.temperature);
}

Basics::Optional_Double WeatherService_i::pressure() {
   return FromStdOptional<decltype(mData.pressure), Basics::Optional_Double>(mData.pressure);
}

Basics::Optional_Double WeatherService_i::humidity() {
   return FromStdOptional<decltype(mData.humidity), Basics::Optional_Double>(mData.humidity);
}

Basics::Optional_Double WeatherService_i::precipitation() {
   return FromStdOptional<decltype(mData.precipitation), Basics::Optional_Double>(mData.precipitation);
}

Basics::Optional_Double WeatherService_i::windspeed() {
   return FromStdOptional<decltype(mData.windspeed), Basics::Optional_Double>(mData.windspeed);
}

Basics::Optional_Double WeatherService_i::winddirection() {
   return FromStdOptional<decltype(mData.winddirection), Basics::Optional_Double>(mData.winddirection);
}

Basics::Optional_Double WeatherService_i::cloudcover() {
   return FromStdOptional<decltype(mData.cloudcover), Basics::Optional_Double>(mData.cloudcover);
}

Basics::Optional_Double WeatherService_i::uv_index() {
   return FromStdOptional<decltype(mData.uv_index), Basics::Optional_Double>(mData.uv_index);
}

Basics::Optional_Long WeatherService_i::weathercode() {
   return FromStdOptional<decltype(mData.weathercode), Basics::Optional_Long>(mData.weathercode);
}

Basics::Optional_String* WeatherService_i::summary() {
   return new Basics::Optional_String(
      CorbaAccessor<Basics::Optional_String>::Return(mData.summary)
   );
}
