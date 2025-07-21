#include "Weather_i.h"
#include "WeatherProxy.h"

#include <BasicTraits.h>
#include <BasicUtils.h>

WeatherService_i::WeatherService_i(WeatherProxy const& aData) : mData(aData.weather_data) {}


Basics::Optional_Time WeatherService_i::sunrise() {
   return CorbaAccessor<Basics::Optional_Time>::Return(mData.sunrise);
   }

Basics::Optional_Time WeatherService_i::sunset() {
   return CorbaAccessor<Basics::Optional_Time>::Return(mData.sunset);
   }

Basics::Optional_Double WeatherService_i::temperature() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.temperature);
   }

Basics::Optional_Double WeatherService_i::pressure() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.pressure);
   }

Basics::Optional_Double WeatherService_i::humidity() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.humidity);
   }

Basics::Optional_Double WeatherService_i::precipitation() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.precipitation);
   }

Basics::Optional_Double WeatherService_i::windspeed() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.windspeed);
   }

Basics::Optional_Double WeatherService_i::winddirection() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.winddirection);
   }

Basics::Optional_Double WeatherService_i::cloudcover() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.cloudcover);
   }

Basics::Optional_Double WeatherService_i::uv_index() {
   return CorbaAccessor<Basics::Optional_Double>::Return(mData.uv_index);
   }

Basics::Optional_Long WeatherService_i::weathercode() {
   return CorbaAccessor<Basics::Optional_Long>::Return(mData.weathercode);
   }

Basics::Optional_String* WeatherService_i::summary() {
   return new Basics::Optional_String(
      CorbaAccessor<Basics::Optional_String>::Return(mData.summary)
   );
}
