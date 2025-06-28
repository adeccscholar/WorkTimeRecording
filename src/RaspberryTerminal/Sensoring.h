#pragma once

#include "BME280.h"
#include "BH1750.h"
#include "Display20x4.h"

#include <gpiod.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <memory>
#include <atomic>
#include <thread>
#include <ranges>
#include <print>
#include <thread>
#include <chrono>

using namespace std::string_literals;


/**
 \brief Template class to read, format, and display sensor data.

 \tparam LightT A sensor conforming to LightSensor concept.
 \tparam EnvT   A sensor conforming to EnvironmentalSensor concept.
*/
template<LightSensor LightT, EnvironmentalSensor EnvT>
class SensorReading_ {
public:
   using sensorData = std::tuple<std::chrono::time_point<std::chrono::system_clock>, double, double, double, double>;

   SensorReading_(LightT lightSensor, EnvT envSensor)
      : lightSensor_(std::move(lightSensor)), envSensor_(std::move(envSensor)) {
      }

   void readAll() {
      envSensor_.read();
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      envSensor_.read();
      lightSensor_.read();

      latest_ = {
          std::chrono::system_clock::now(),
          lightSensor_.getCalibratedLux(),
          envSensor_.temperature(),
          envSensor_.pressure(),
          envSensor_.humidity()
      };
   }

   void print(std::ostream& os = std::cout) const {
      std::println(os, "{:5} lux", std::get<1>(latest_));
      std::println(os, "{:5.1f} °C", std::get<2>(latest_));
      std::println(os, "{:4.0f} hPa", std::get<3>(latest_));
      std::println(os, "{:4.1f} %", std::get<4>(latest_));
   }

   const sensorData& data() const noexcept { return latest_; }

private:
   LightT lightSensor_;
   EnvT envSensor_;
   sensorData latest_;
};

// ------------------------------------------------------------

class SensorReading {
   DISPLAY_20x4 display { 0x27 };
   BH1750Device lightSensor { 0x23 };
   BME280Device environmentalSensor { 0x76 };

   using sensorData = std::tuple<std::chrono::time_point<std::chrono::system_clock>, double, double, double, double>;
public:
   SensorReading() = default;

   void initExternDisplay() {
      display.print(0, 0, "light:");
      display.print(1, 0, "temperature:");
      display.print(2, 0, "pressure:");
      display.print(3, 0, "humidity:");
      }

   void writeValuesExternDisplay(sensorData const& sensors) {
      display.print(0, 20-9, "{:6.1f}lux", std::get<1>(sensors));
      display.print(1, 20-8, "{:5.1f} °C", std::get<2>(sensors));
      display.print(2, 20-7, "{:4.0f}hPa", std::get<3>(sensors));
      display.print(3, 20-5, "{:4.1f}%", std::get<4>(sensors));
      }

   void logging_sensors(sensorData const& sensors, std::ostream& out = std::cout) {
      std::println(out, "{:6.1f}lux", std::get<1>(sensors));
      std::println(out, "{:5.1f}C", std::get<2>(sensors));
      std::println(out, "{:4.0f}hPa", std::get<3>(sensors));
      std::println(out, "{:4.1f}%", std::get<4>(sensors));
      }

   void readSensors() {
      environmentalSensor.read(); // first read mostly incorrect
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      environmentalSensor.read();
      lightSensor.read();
      sensorData sensors = { std::chrono::system_clock::now(), lightSensor.getCalibratedLux(),
                             environmentalSensor.temperature(), environmentalSensor.pressure(), environmentalSensor.humidity() };
      writeValuesExternDisplay(sensors);
      logging_sensors(sensors);
   }
};

class TimeTracking {
   static constinit const std::array<unsigned int, 3> inline led_lights{ 13, 19, 26 };
   static constinit const std::array<unsigned int, 3> inline buttons{ 20, 21 };
   static constinit const unsigned int                inline beeper{ 12 };
   static constinit const unsigned int                inline reset_pin{ 25 }; // reset pin for rfid
   static constinit const unsigned int                inline move_pin{ 15 };  // motion decector


   std::unique_ptr<gpiod::chip>         chip;
   std::unique_ptr<gpiod::line_request> output_request;
   std::unique_ptr<gpiod::line_request> input_request;

   std::atomic<unsigned int>            input_demand;
   std::thread                          input_thread;
   bool                                 input_running = false;

   SensorReading& sensors;

public:
   TimeTracking() = delete;
   TimeTracking(SensorReading& sensors_) : sensors(sensors_) {}
   void Init() {
      chip = std::make_unique<gpiod::chip>("/dev/gpiochip4");
      gpiod::line::offsets lines;

      std::ranges::copy(led_lights, std::back_inserter(lines));
      lines.emplace_back(beeper);
      lines.emplace_back(reset_pin);

      gpiod::line_request _request = chip->prepare_request()
         .set_consumer("time tracking output")
         .add_line_settings(lines, gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT))
         .do_request();
      output_request = std::make_unique<gpiod::line_request>(std::move(_request));

      lines.clear();
      lines.emplace_back(move_pin);
      _request = chip->prepare_request()
         .set_consumer("time tracking input")
         .add_line_settings(lines, gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT))
         .do_request();
      input_request = std::make_unique<gpiod::line_request>(std::move(_request));


      std::cout << "Chip Information: " << chip->get_info() << '\n';
      std::cout << *output_request << '\n';
      std::cout << *input_request << '\n';
      // presence / absence - presence detector or sensing 
   }

   void check_input(std::atomic<unsigned int>& demand, bool& running) {
      gpiod::line::offsets lines;

      if (running == true) throw std::runtime_error("unexpected call of function");
      else running = true;
      std::ranges::copy(buttons, std::back_inserter(lines));

      auto response = chip->prepare_request()
         .set_consumer("time tracking answers")
         .add_line_settings(lines, gpiod::line_settings()
            .set_direction(gpiod::line::direction::INPUT)
            .set_edge_detection(gpiod::line::edge::BOTH)
            .set_bias(gpiod::line::bias::PULL_UP)
            .set_debounce_period(std::chrono::milliseconds(10)))
         .do_request();

      gpiod::edge_event_buffer buffer;

      auto old_val = gpiod::line::value::INACTIVE;
      while (running) {
         if (response.wait_edge_events(std::chrono::milliseconds(500))) {
            if (auto val = input_request->get_value(15); val == gpiod::line::value::ACTIVE)
               std::println(std::cout, "presence");
            else
               std::println(std::cout, "absence");
            
            response.read_edge_events(buffer);
            for (auto const& event : buffer) {

               switch (unsigned int iOffset = event.line_offset();  event.type()) {
               case gpiod::edge_event::event_type::RISING_EDGE:
                  demand.store(iOffset);
                  output_request->set_value(beeper, gpiod::line::value::INACTIVE);
                  std::println(std::cout, "Button {} rising", iOffset);
                  break;
               case gpiod::edge_event::event_type::FALLING_EDGE:
                  output_request->set_value(beeper, gpiod::line::value::ACTIVE);
                  std::println(std::cout, "Button {} falling", iOffset);
                  sensors.readSensors();
                  break;
               default: throw std::runtime_error("unexpected event time in check_input");
               }
            }
         }
         else {
            if (auto val = input_request->get_value(15); val != old_val) {
               output_request->set_value(beeper, gpiod::line::value::ACTIVE);
               if(val == gpiod::line::value::ACTIVE)  std::println(std::cout, "presence");
               else                                   std::println(std::cout, "absence");
               sensors.readSensors();
               std::this_thread::sleep_for(std::chrono::milliseconds(300));
               output_request->set_value(beeper, gpiod::line::value::INACTIVE);
               old_val = val;
               }
            ; // überlegen, wie im timeout reagieren
         }
      }

   }

   void Test_LEDs() {
      std::vector<gpiod::line::value> switch_on(led_lights.size(), gpiod::line::value::ACTIVE);
      std::vector<gpiod::line::value> switch_off(led_lights.size(), gpiod::line::value::INACTIVE);

      gpiod::line::offsets lines;
      std::ranges::copy(led_lights, std::back_inserter(lines));

      input_thread = std::thread(&TimeTracking::check_input, this, std::ref(input_demand), std::ref(input_running));

      for (uint32_t i : std::views::iota(0, 1000)) { // 80)) {
         if (i % 2 == 0) output_request->set_values(lines, switch_on);
         else output_request->set_values(lines, switch_off);
         std::this_thread::sleep_for(std::chrono::milliseconds(500));
         }
      output_request->set_values(lines, switch_off);

      input_running = false;
      if (input_thread.joinable()) input_thread.join();

   }

};