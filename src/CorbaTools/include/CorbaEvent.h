#pragma once

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <orbsvcs/CosEventCommS.h>
#include <orbsvcs/CosEventChannelAdminC.h>

#include <tuple>
#include <type_traits>
#include <concepts>
#include <iostream>

// ============================================================================
// Concepts
// ============================================================================

template <typename Handler, typename Event>
concept handles_event = requires(Handler h, Event const* e) {
   { h.handle(e) } -> std::same_as<void>;
};

template <typename Handler, typename... Events>
concept handles_all_events = (handles_event<Handler, Events> && ...);

/**
  \brief Checks if a Supplier can push a specific Event
  \tparam Supplier A candidate push supplier type
  \tparam Event The event type to test
  \returns true if Supplier inherits from POA_CosEventComm::PushSupplier and provides push(Event const&)
*/
template<typename Supplier, typename Event>
concept can_push_event = std::derived_from<std::remove_cvref_t<Supplier>, POA_CosEventComm::PushSupplier> &&
   requires(Supplier& s, Event const& e) {
         { s.push(e) } -> std::same_as<void>;
   };

/**
 \brief Checks whether a Consumer can receive a specific Event
 \tparam Consumer The candidate consumer type
 \tparam Event The event type
 \returns true if the Consumer is a CORBA PushConsumer and can handle the given Event
*/
template<typename Consumer, typename Event>
concept can_receive_event = std::derived_from<std::remove_cvref_t<Consumer>, POA_CosEventComm::PushConsumer> &&
   requires(Consumer& c, Event const* e) {
         { c.handle(e) } -> std::same_as<void>;
   };

// ============================================================================
// Generic PushConsumer
// ============================================================================

template <typename Handler, typename... Events> requires handles_all_events<Handler, Events...>
class TEvent_PushConsumer : public POA_CosEventComm::PushConsumer, public Handler {
public:
   TEvent_PushConsumer()  = delete;
   TEvent_PushConsumer(CosEventChannelAdmin::EventChannel_ptr event_channel) { connect(event_channel);  }
   ~TEvent_PushConsumer() { disconnect(); }

   void connect(CosEventChannelAdmin::EventChannel_ptr event_channel) {
      CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = event_channel->for_consumers();
      the_supplier_proxy = consumer_admin->obtain_push_supplier();
      CosEventComm::PushConsumer_var myself = this->_this();
      the_supplier_proxy->connect_push_consumer(myself.in());
      }

   void disconnect() {
      if (!CORBA::is_nil(the_supplier_proxy)) {
         the_supplier_proxy->disconnect_push_supplier();
         the_supplier_proxy = CosEventChannelAdmin::ProxyPushSupplier::_nil();
         }
      }

   void push(const CORBA::Any& data) override {
      auto try_extract_event = [&](const auto* ptr) -> bool {
         if (data >>= ptr) {
            this->handle(ptr);
            return true;
            }
         return false;
         };

      bool found = (try_extract_event(static_cast<const Events*>(nullptr)) || ...);

      if (!found && !CORBA::is_nil(data.type())) {
         std::cerr << "[TEvent_PushConsumer] Ignored event with type: "
                   << data.type()->name() << " (id: " << data.type()->id() << ")\n";
         }
      }

   void disconnect_push_consumer() override {
      disconnect();
      }

private:
   CosEventChannelAdmin::ProxyPushSupplier_var the_supplier_proxy;
};

// ============================================================================
// Generic PushSupplier
// ============================================================================

//template <typename ty>
//class TEvent_PushSupplier;

template <typename... Events>
class TEvent_PushSupplier : virtual public POA_CosEventComm::PushSupplier {
public:
   TEvent_PushSupplier(CORBA::ORB_ptr orb,
                        PortableServer::POA_ptr poa,
                        CosEventChannelAdmin::ProxyPushConsumer_ptr consumer)
      : the_orb{ CORBA::ORB::_duplicate(orb) },
        the_poa{ PortableServer::POA::_duplicate(poa) },
        the_consumer{ CosEventChannelAdmin::ProxyPushConsumer::_duplicate(consumer) } { }

   ~TEvent_PushSupplier() {
      disconnect_push_supplier();
      the_consumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
      the_poa = PortableServer::POA::_nil();
      the_orb = CORBA::ORB::_nil();
      }

   template <typename Event> requires (std::same_as<Event, Events> || ...)
   void push_event(Event const& event_data) {
      if (!CORBA::is_nil(the_consumer)) {
         CORBA::Any event;
         event <<= event_data;
         the_consumer->push(event);
         }
      }

   void disconnect_push_supplier() override {
      if (!CORBA::is_nil(the_consumer)) {
         the_consumer->disconnect_push_consumer();
         the_consumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
         }
      }

private:
   CORBA::ORB_var                               the_orb;
   PortableServer::POA_var                      the_poa;
   CosEventChannelAdmin::ProxyPushConsumer_var  the_consumer;
};

template <typename... Events>
using TEvent_PushSupplier_tie = POA_CosEventComm::PushSupplier_tie<TEvent_PushSupplier<Events...>>;

// ============================================================================
// EventSystem Wrapper für zentrale Verwaltung
// ============================================================================

template <typename... Events>
struct EventSystem {
   template <typename Handler>
   using Consumer = TEvent_PushConsumer<Handler, Events...>;

   using Supplier = TEvent_PushSupplier<Events...>;
};


/*

#include "EventSystem.h"
#include "WeatherDataS.h"  // generierte CORBA-Typen

using MyWeatherEvents = EventSystem<
   SensorsModule::WeatherValues,
   SensorsModule::WeatherEvent,
   SensorsModule::WeatherStatus,
   SensorsModule::WeatherRequest,
   SensorsModule::WeatherRequested
>;

// Supplier:
MyWeatherEvents::Supplier* supplier = new MyWeatherEvents::Supplier(orb, poa, consumer);
supplier->push_event(some_weather_event);

// Consumer:
MyWeatherEvents::Consumer<TEventHandler> consumer;
consumer.connect(event_channel);

*/

/*
d:
cd D:\Projekte\GitHub\WorkTimeRecording\src\out\Windows\Release

Büro
-----------

tao_cosnaming -m 1 -ORBEndpoint iiop://192.168.10.189:10013
tao_cosevent -ORBEndpoint iiop://192.168.10.189:10020
tao_nslist -ORBInitRef NameService=iiop://192.168.10.189:10013/NameService

WeatherAPIServer -ORBEndpoint iiop://192.168.10.189 -ORBInitRef NameService=corbaloc:iiop:192.168.10.189:10013/NameService -ORBInitRef EventService=corbaloc:iiop:192.168.10.189:10020/EventService


zu hause
-----------
tao_cosnaming -m 1 -ORBEndpoint iiop://192.168.178.36:10013
tao_cosevent -ORBEndpoint iiop://192.168.178.36:10020 -ORBInitRef NameService=iiop://192.168.178.36:10013/NameService
tao_nslist -ORBEndpoint iiop://192.168.178.36 -ORBInitRef NameService=iiop://192.168.178.36:10013/NameService

WeatherAPIServer -ORBEndpoint iiop://192.168.178.36 -ORBInitRef NameService=corbaloc:iiop:192.168.178.36:10013/NameService -ORBInitRef EventService=corbaloc:iiop:192.168.178.36:10020/EventService

*/

