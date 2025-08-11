
#include <Corba_Interfaces.h>
#include <Corba_CombiInterface.h>

#include <tao/Transport_Descriptor_Interface.h>

// public Event Server
#include <orbsvcs/CosEventCommS.h>
#include <orbsvcs/CosEventChannelAdminC.h>

// private event channel for a specific server
#include <orbsvcs/CosEvent/CEC_EventChannel.h>
#include <orbsvcs/CosEvent/CEC_Default_Factory.h>

#include <concepts>
#include <cstdint>
#include <type_traits>

// --------------------------------------------------------------------------
// In production code, these functions, enums, and classes should reside in 
//  a dedicated namespace to prevent name conflicts.
// --------------------------------------------------------------------------


/**
 \brief Generates a bitmask with a single bit set at the given index.
 \tparam ty An unsigned integral type
 \tparam uIndex Zero-based bit index
 \returns A value of type ty with only the bit at position uIndex set
 \note constexpr-safe for compile-time usage
 \note static_asserts are additional for safety
 \pre uIndex must be less than std::numeric_limits<ty>::digits
*/
template<typename ty, std::size_t uIndex> requires std::unsigned_integral<ty>
consteval ty MakeBit() {
   static_assert(std::numeric_limits<ty>::is_integer,      "ty must be an integer type");
   static_assert(std::is_unsigned_v<ty>,                   "ty must be unsigned");
   static_assert(uIndex < std::numeric_limits<ty>::digits, "Bit index out of range for type");

   return ty { 1 } << uIndex;
   }

/// kind of event service which used
enum class EventServerType : uint32_t {
   undefined       = 0,                       ///< there isn't an event server or client or its unknown
   send_private    = MakeBit<uint32_t, 1>(),  ///< sending private events about a channel
   receive_private = MakeBit<uint32_t, 2>(),  ///< receiving private events about a channel
   send_public     = MakeBit<uint32_t, 3>(),  ///< sending public events 
   receive_public  = MakeBit<uint32_t, 4>(),  ///< receiving public events 
   };


/**
 \brief Checks if the public sending bit is set
 \tparam uValue A uint32_t constant value
 \returns true if the bit for send_public is set
*/
template<uint32_t uValue>
concept has_public_sender = (uValue & static_cast<uint32_t>(EventServerType::send_public)) != 0;

/**
 \brief Checks if the private sending bit is set
 \tparam uValue A uint32_t constant value
 \returns true if the bit for send_private is set
*/
template<uint32_t uValue>
concept has_private_sender = (uValue & static_cast<uint32_t>(EventServerType::send_private)) != 0;

/**
 \brief Checks if the public receiving bit is set
 \tparam uValue A uint32_t constant value
 \returns true if the bit for receive_public is set
*/
template<uint32_t uValue>
concept has_public_receiver = (uValue & static_cast<uint32_t>(EventServerType::receive_public)) != 0;

/**
 \brief Checks if the private receiving bit is set
 \tparam uValue A uint32_t constant value
 \returns true if the bit for receive_private is set
*/
template<uint32_t uValue>
concept has_private_receiver = (uValue & static_cast<uint32_t>(EventServerType::receive_private)) != 0;

/**
 \brief Checks if either private sending or receiving bit is set
 \tparam uValue A uint32_t constant value
 \returns true if either send_private or receive_private bit is set
*/
template<uint32_t uValue>
concept has_private_events = has_private_sender<uValue> || has_private_receiver<uValue>;

/**
 \brief Checks if either public sending or receiving bit is set
 \tparam uValue A uint32_t constant value
 \returns true if either send_public or receive_public bit is set
*/
template<uint32_t uValue>
concept has_public_events = has_public_sender<uValue> || has_public_receiver<uValue>;

/**
 \brief Combines multiple EventServerType enum values into a single uint32_t mask
 \tparam aTypes Variadic list of EventServerType values
 \returns A uint32_t bitmask with all corresponding bits set
 \note Evaluated entirely at compile time
*/
template<EventServerType... aTypes>
consteval uint32_t MakeEventMask() {
   uint32_t uResult = 0;
   ((uResult |= static_cast<uint32_t>(aTypes)), ...);
   return uResult;
   }


constexpr uint32_t uMyTestFlags = MakeEventMask<
   EventServerType::send_private,
   EventServerType::receive_private,
   EventServerType::send_public>();

static_assert(has_private_sender<uMyTestFlags>);
static_assert(has_public_sender<uMyTestFlags>);
static_assert(!has_public_receiver<uMyTestFlags>);





/// structure to use RAII and initialize the default event factory before OrbInit
struct EventPrepare {
   EventPrepare() {
      TAO_CEC_Default_Factory::init_svcs(); // this line must be BEFORE OrbInit()
      }
   virtual ~EventPrepare() { }
   };

struct PublicEvent {
   CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;
   CosEventChannelAdmin::ProxyPushConsumer_var push_consumer;
   };
 
struct PrivateChannel : public virtual ORBBase {
   std::string strChannelName;

   PrivateChannel(std::string const& channel, std::string const& name, int argc, char* argv[]) :
         ORBBase(name, argc, argv), strChannelName{ channel } {}

   void Connect() {}

};

struct PrivateSupplier : virtual public PrivateChannel {
   CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;
   CosEventChannelAdmin::ProxyPushConsumer_var push_consumer;

   PrivateSupplier(std::string const& channel, std::string const& name, int argc, char* argv[]) : 
                 PrivateChannel(channel, name, argc, argv), ORBBase(name, argc, argv) { }
   };

struct PrivateConsumer : virtual public PrivateChannel {
   CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;
   CosEventChannelAdmin::ProxyPushConsumer_var push_consumer;

   PrivateConsumer(std::string const& channel, std::string const& name, int argc, char* argv[]) :
                 PrivateChannel(channel, name, argc, argv), ORBBase(name, argc, argv) { }
};



//template <EventServerType server_type>


template <uint32_t KindEvent, CORBASkeleton... Skeletons>
class Event_Service : public virtual EventPrepare, 
                      public CORBAServer<Skeletons...>, 
                      private std::conditional_t<has_private_sender<KindEvent>, PrivateSupplier, void>,
                      private std::conditional_t<has_private_receiver<KindEvent>, PrivateConsumer, void>,
                      private std::conditional_t<has_public_sender<KindEvent>, PublicEvent, void> {
   Event_Service() = delete;

   template <typename = std::enable_if_t<has_private_events<KindEvent>>>
   Event_Service(std::string const& channel, std::string const& name, int argc, char* argv[]) :
                       PrivateChannel ( channel, name, argc, argv ),
                       CORBAServer<Skeletons...>(name, argc, argv),
                       ORBBase(name, argc, argv) {
      }

   template <typename = std::enable_if_t<!has_private_events<KindEvent>>>
   Event_Service(std::string const& name, int argc, char* argv[]) : 
                        CORBAServer<Skeletons...>(name, argc, argv),
                        ORBBase(name, argc, argv) {
      /*
      CosNaming::Name public_channel_name;
      public_channel_name.length(1);
      public_channel_name[0].id = CORBA::string_dup("CosEventService");
      CORBA::Object_var pec_object = ns_context->resolve(public_channel_name);
      CosEventChannelAdmin::EventChannel_var pec = CosEventChannelAdmin::EventChannel::_narrow(pec_object.in());


      public_supplier_admin = pec->for_suppliers();
      public_consumer = public_supplier_admin->obtain_push_consumer();


      public_sensor_push_supplier = new TSensor_PushSupplier(orb.in(), poa.in(), public_consumer.in());
      public_sensor_push_supplier_tie = new SensorPushSupplier_tie(public_sensor_push_supplier, poa.in());
      public_consumer->connect_push_supplier(public_sensor_push_supplier_tie->_this());

      TSensor_PushConsumer_New<TEventHandler> public_pushconsumer;
      public_pushconsumer.bind_to(public_sensor_push_supplier);
      public_pushconsumer.connect(pec.in());
      */
      }
};


