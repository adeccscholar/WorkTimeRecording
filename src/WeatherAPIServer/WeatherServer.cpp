
#include "EventScheduler.h"
#include "WeatherProxy.h"
#include "Utility.h"

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>

#include <iostream>
#include <chrono>
#include <condition_variable>
#include <format>
#include <print>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <thread>
#include <vector>
#include <atomic>

#include <csignal>

#ifdef _WIN32
   #include <Windows.h>
#endif

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

// using date_ty      = std::chrono::year_month_day;
//using timepoint_ty = std::chrono::sys_seconds; //   std::chrono::system_clock::time_point;
using timepoint_ty = std::chrono::local_time<std::chrono::seconds>;


struct EvStart       : sc::event<EvStart> {};
struct EvIdle        : sc::event<EvIdle> {};
struct EvReadCurrent : sc::event<EvReadCurrent> {};
struct EvDaily       : sc::event<EvDaily> {};
struct EvShutdown    : sc::event<EvShutdown> {};
struct EvTerminate   : sc::event<EvTerminate> {};

struct OffState;
struct StartingState;
struct OnState;
   struct OnIdleState;
   struct OnDailyState;
   struct OnCurrentState;
struct StoppingState;

/*
struct FetchWeatherMachine : sc::state_machine<FetchWeatherMachine, OffState> {
   WeatherProxy& api;
   TimedEvents::Scheduler& scheduler;
   std::atomic_bool running = true;
   std::thread scheduler_thread;

   FetchWeatherMachine(WeatherProxy& a, TimedEvents::Scheduler& s) : api(a), scheduler(s) {}

   template <typename event_ty>
   void safe_process(event_ty&& ev) {
      try {
         process_event(std::forward<event_ty>(ev));
         }
      catch (std::exception const& ex) {
         std::println("[FetchWeatherMachine] process_event error: {}", ex.what());
         }
      }

   void run() {
      if (scheduler_thread.joinable()) {
         throw std::runtime_error("Thread already running");
         }

      if (state_cast<const OffState*>() != nullptr) {
         safe_process(EvStart{});
         }

      scheduler_thread = std::thread([this]() {
         while (running) {
            auto next = NextStep<timepoint_ty>(std::chrono::minutes{ 1 });
            auto sleep_for = SecondsTo(next);
            if (sleep_for > std::chrono::seconds{ 0 })
               std::this_thread::sleep_for(sleep_for);

            while (running) {
               if (auto ev = scheduler.pollEvent(running)) {
                  ev->trigger();
                  }
               else {
                  break; // Keine Events mehr: aus der Schleife, wieder schlafen
                  }
               }
            }
         });
      }

   };
*/

/**
  \brief Weather fetch state machine integrating with timed event scheduling
  \details This state machine handles weather data retrieval, time-based scheduling via Scheduler,
           and manages a dedicated thread for event-driven progression.
 */
struct FetchWeatherMachine : sc::state_machine<FetchWeatherMachine, OffState> {
   WeatherProxy& api;
   TimedEvents::Scheduler& scheduler;
   std::atomic_bool running = true;
   std::atomic_bool wakeup_notified = false;

   std::thread scheduler_thread;
   std::mutex wakeup_mutex;
   std::condition_variable wakeup_cv;

   /**
     \brief Constructor
     \param a Reference to WeatherProxy
     \param s Reference to Scheduler
    */
   FetchWeatherMachine(WeatherProxy& a, TimedEvents::Scheduler& s)
      : api(a), scheduler(s) {
   }

   /**
    * \brief Safe event processing wrapper with logging
     \tparam event_ty Type of the event
     \param ev Event to process
    */
   template <typename event_ty>
   void safe_process(event_ty&& ev) {
      try {
         process_event(std::forward<event_ty>(ev));
      }
      catch (std::exception const& ex) {
         std::println("[FetchWeatherMachine] process_event error: {}", ex.what());
      }
   }

   /**
     \brief Starts the scheduler thread and optionally posts initial state
    */
   void run() {
      if (scheduler_thread.joinable()) {
         throw std::runtime_error("Scheduler thread already running");
      }

      // Start with initial event if in OffState
      if (state_cast<const OffState*>() != nullptr) {
         safe_process(EvStart{});
         }

      // Set the wakeup callback in scheduler to notify this machine
      scheduler.setWakeup([this]() {
         std::lock_guard lock(wakeup_mutex);
         wakeup_notified = true;
         wakeup_cv.notify_one();
         });

      scheduler_thread = std::thread([this]() {
         std::unique_lock lock(wakeup_mutex);

         while (running) {
            // Wait until next event is due or wakeup triggered
            if (auto next_tp = scheduler.peekNextEventTime()) {
               auto sys_tp = std::chrono::current_zone()->to_sys(*next_tp);
               wakeup_notified = false;
               wakeup_cv.wait_until(lock, sys_tp, [this]() {
                     return !running || wakeup_notified.load();
                     });
               wakeup_notified = false;
               }
            else {
               // Wait indefinitely until an event is added or shutdown triggered
               wakeup_cv.wait(lock, [this]() {
                  return !running || scheduler.peekNextEventTime().has_value() || wakeup_notified.load();
                  });
               wakeup_notified = false;
               }

            if (!running) break;

            // Process all due events
            while (auto ev = scheduler.pollEvent(running)) {
               ev->trigger();
            }
         }
         });
   }

   /**
     \brief Stops the machine and joins the background thread
    */
   void stop() {
      if (!running) return;
      running = false;
      {
         std::lock_guard lock(wakeup_mutex);
         wakeup_cv.notify_all();
      }
      if (scheduler_thread.joinable()) {
         scheduler_thread.join();
      }
   }

   ~FetchWeatherMachine() {
      stop();
   }
};


struct OffState : sc::state<OffState, FetchWeatherMachine> {
   using reactions = mpl::list<sc::custom_reaction<EvStart>,
                               sc::custom_reaction<EvTerminate>>;

   OffState(my_context ctx) : my_base(ctx) {
      std::println("[OffState] System is offline");
      }

   sc::result react(EvStart const&) {
      std::println("[OffState] Starting system ...");
      return transit<StartingState>();
      }


   sc::result react(EvTerminate const&) {
      std::println("[OffState] terminating system ...");
      outermost_context().terminate();
      return discard_event();
      }

   };

struct StartingState : public sc::state <StartingState, FetchWeatherMachine> {
   using reactions = mpl::list<sc::custom_reaction<EvIdle>>;
   StartingState(my_context ctx) : my_base(ctx) {
      std::println("[StarttingState] System starting");
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      machine.api.FetchDailyData();
      machine.api.FetchCurrentData();
      machine.running = true;
      auto next_current = NextStep<timepoint_ty>(std::chrono::minutes(15));
      //next_current -= std::chrono::minutes(1);
      std::println("[StarttingState] next current Event at {:%d.%m.%Y %X}", next_current);
      auto next_day = NextStep<timepoint_ty>(std::chrono::days(1));
      std::println("[StarttingState] next daily Event at {:%d.%m.%Y %X}", next_day);

      machine.scheduler.addEvent({ next_current, [&machine]() { machine.safe_process(EvReadCurrent{}); } });
      machine.scheduler.addEvent({ next_day,     [&machine]() { machine.safe_process(EvDaily{}); } });
      std::println("[StarttingState] System successfully started.");
      post_event(EvIdle{});
      }

   sc::result react(EvIdle const&) {
      return transit<OnState>();
      }
};

struct StoppingState : public sc::state <StoppingState, FetchWeatherMachine> {
   using reactions = mpl::list<sc::custom_reaction<EvShutdown>>;
   StoppingState(my_context ctx) : my_base(ctx) {
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      std::println("[StoppingState] System stopping");
      machine.scheduler.clearEvents();
      // aufräumen 
      post_event(EvShutdown{});;
      }

   sc::result react(EvShutdown const&) {
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      std::println("[StoppingState] Shutdown received, stopping...");
      machine.running = false;
      return transit<OffState>();
      }
   };

struct OnState : public sc::state<OnState, FetchWeatherMachine, OnIdleState> {
   using my_base = sc::state<OnState, FetchWeatherMachine, OnIdleState>;
   using reactions = mpl::list<sc::custom_reaction<EvShutdown>,
                               sc::custom_reaction<EvTerminate>>;

   OnState(my_context ctx) : my_base(ctx) {
      std::println("[OnState] System is online");
      }

   sc::result react(EvShutdown const&) {
      std::println("[OnState] Shutdown received, terminating...");
      return transit<StoppingState>();
      }

   sc::result react(EvTerminate const&) {
      std::println("[OnState] terminating system ...");
      outermost_context().terminate();
      return discard_event();
      }

   };

struct OnDailyState : sc::state<OnDailyState, OnState> {
   using reactions = mpl::list<sc::custom_reaction<EvIdle>>;
   OnDailyState(my_context ctx) : my_base(ctx) {
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      std::println("[OnDailyState] fetching daily data");
      if(!machine.api.FetchDailyData()) {
         //auto next_day = NextStep(std::chrono::days(1));
         auto next_day = NextStep<timepoint_ty>(std::chrono::minutes(1));
         std::println("[OnDailyState] repeat daily event at {:%d.%m.%Y %X}", next_day);
         machine.scheduler.addEvent({ next_day, [&machine]() { machine.safe_process(EvDaily{}); } });
         }
      else {
         auto next_day = NextStep<timepoint_ty>(std::chrono::days(1));
         std::println("[OnDailyState] next daily event at {:%d.%m.%Y %X}", next_day);
         machine.scheduler.addEvent({ next_day, [&machine]() { machine.safe_process(EvDaily{}); } });
         }
      post_event(EvIdle{});
      }

   sc::result react(EvIdle const&) {
      return transit<OnIdleState>();
      }
   };

struct OnCurrentState : sc::state<OnCurrentState, OnState> {
   using reactions = mpl::list<sc::custom_reaction<EvIdle>>;
   OnCurrentState(my_context ctx) : my_base(ctx) {
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      std::println("[OnCurrentState] fetching current data");
      if (!machine.api.FetchCurrentData()) {
         auto next_current = NextStep<timepoint_ty>(std::chrono::minutes(1));
         std::println("[OnCurrentState] repeat current event at {:%d.%m.%Y %X}", next_current);
         machine.scheduler.addEvent({ next_current, [&machine]() { machine.safe_process(EvReadCurrent{}); } });
         }
      else {
         auto next_current = NextStep<timepoint_ty>(std::chrono::minutes(15));
         std::println("[OnCurrentState] next current event at {:%d.%m.%Y %X}", next_current);
         machine.scheduler.addEvent({ next_current, [&machine]() { machine.safe_process(EvReadCurrent{}); } });
         }
      post_event(EvIdle{});
      }

   sc::result react(EvIdle const&) {
      return transit<OnIdleState>();
   }
};

struct OnIdleState : sc::state<OnIdleState, OnState> {
   using reactions = mpl::list<sc::custom_reaction<EvDaily>,
                               sc::custom_reaction<EvReadCurrent>>;

   OnIdleState(my_context ctx) : my_base(ctx) {
      FetchWeatherMachine& machine = context<FetchWeatherMachine>();
      std::print("[Idle] waiting, next event:");
      if (machine.scheduler.peekNextEventTime())
         std::println("{:%d.%m.%Y %X}", *machine.scheduler.peekNextEventTime());
      else
         std::println("there are not new events");
      }

   sc::result react(EvDaily const&) {
      std::println("[Idle] daily data event received");
      return transit<OnDailyState>();
      }

   sc::result react(EvReadCurrent const&) {
      std::println("[Idle] current data event received");
      return transit<OnCurrentState>();
      }
   };

std::function<void()> exit_func;

void signal_handler(int sig_num) {
   std::println(std::cout);
   std::println(std::cout, "[signal handler] signal {} received. Requesting shutdown ...", sig_num);
   if (exit_func) exit_func();
   }
   
int main() {
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   WeatherProxy weather_data;
   TimedEvents::Scheduler  scheduler;
   FetchWeatherMachine machine(weather_data, scheduler);

   exit_func = [&machine]() { machine.safe_process(EvShutdown{}); };
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

   std::println(std::cout, "server to use the open-meteo.com Rest API");
 
   machine.initiate();
   machine.run();

   machine.scheduler_thread.join();
   exit_func = nullptr;

   std::println("[Main] Machine exited cleanly.");
   return 0;
}
