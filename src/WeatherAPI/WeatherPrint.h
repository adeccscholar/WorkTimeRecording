#pragma once
#include "WeatherAPI.h"

#include "WeatherData.h"

#include <vector>

WEATHERAPI_API void print(WeatherMeta const& meta);
WEATHERAPI_API void print(WeatherCurrentExtended const& w);
WEATHERAPI_API void print(WeatherCurrent const& wc);
WEATHERAPI_API void print(std::vector<WeatherDay> const& data);
WEATHERAPI_API void print(std::vector<WeatherHour> const& data);
