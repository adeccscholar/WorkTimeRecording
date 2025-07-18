// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Header-only CORBA utility templates for managing server and client lifecycles.
 
  \details This header defines a comprehensive set of C++20 CORBA template utilities used within 
           the *adecc Scholar* project, a distributed time-tracking system. It provides both client 
           and server-side facilities based on the CORBA middleware standard (TAO).
 
  \details These utilities are used throughout the distributed system to manage CORBA object lifecycles safely,
           prevent resource leaks, and encapsulate boilerplate CORBA naming service lookups and ORB setup.
 
           ### Included components:
 
           - \ref CORBAServer — a variadic server template for managing multiple servant types via a centralized POA
           - \ref CORBAClient — a variadic client template for resolving multiple stubs from the Naming Service
           - \ref ORBBase — a base class encapsulating common CORBA ORB initialization and NamingContext resolution
           - \ref Destroyable_Var — RAII wrapper for CORBA stubs with `destroy()`
           - \ref make_destroyable — factory helper to create \ref Destroyable_Var from raw pointers
 
           ### Concepts:
           - \ref CORBAStub — ensures valid CORBA client stubs (TAO-generated)
           - \ref CORBAStubWithDestroy — extends \ref CORBAStub with `destroy()`
           - \ref CORBASkeleton — ensures valid CORBA server-side servants (TAO-generated)
 
           ### Background:
           CORBA in modern C++ is verbose and fragile if done manually. These utilities abstract boilerplate 
           ORB setup, stub narrowing, servant activation, and CosNaming binding into reusable templates.
           This enables maintainable and type-safe distributed development using TAO, Qt, and Boost.SML.
 
           ### Usage scenarios:
           - Launching multiple CORBA services from one binary with type-safe servant management
           - Safely acquiring remote service interfaces using modern client-side RAII wrappers
           - Automatic resource cleanup for services that implement `destroy()`
 
  \details The project is part of the adecc Scholar educational initiative, demonstrating a
           distributed, cross-platform time-tracking system implemented with CORBA (TAO),
           Boost.SML, Qt6, and modern C++20 features.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.


  \version 1.0
  \date    31.05.2025 created
  \date    07.06.2025 variadic CORBAClient  
  \date    16.06.2025 variadic CORBAServer
*/

#pragma once

#include "my_logging.h"
#include <CorbaUtils.h>
#include <Tools.h>
#include "Corba_Nameservice.h"

#include "tao/Object.h"
#include <tao/PortableServer/Servant_Base.h>
#include <tao/PortableServer/PortableServer.h>

#include <orbsvcs/CosNamingC.h>


#include <concepts>
#include <vector>
#include <string>
#include <tuple>
#include <array>
#include <atomic>
#include <thread>

using namespace std::string_literals;

/**
 \concept CORBAStub
 \brief Concept for a valid CORBA stub interface type.

 \details
 This concept ensures that the given type \c ty:
 - Is derived from \c ::CORBA::Object
 - Provides the required CORBA type aliases: `_ptr_type`, `_var_type`, `_out_type`
 - Implements the CORBA-required static member functions:
   - `_duplicate(ptr)`, returning `_ptr_type`
   - `_narrow(Object_ptr)`, returning `_ptr_type`
   - `_unchecked_narrow(Object_ptr)`, returning `_ptr_type`
   - `_nil()`, returning `_ptr_type`

 \tparam ty The candidate type to be validated as CORBA stub type.

 \note Used as a constraint for client-side CORBA stub template parameters.
*/
template<typename ty>
concept CORBAStub = std::derived_from<ty, ::CORBA::Object> && requires {
           typename ty::_ptr_type;
           typename ty::_var_type;
           typename ty::_out_type;

           { ty::_duplicate(std::declval<typename ty::_ptr_type>()) } -> std::same_as<typename ty::_ptr_type>;
           { ty::_narrow(std::declval<::CORBA::Object_ptr>()) } -> std::same_as<typename ty::_ptr_type>;
           { ty::_unchecked_narrow(std::declval<::CORBA::Object_ptr>()) } -> std::same_as<typename ty::_ptr_type>;
           { ty::_nil() } -> std::same_as<typename ty::_ptr_type>;
          };


/**
 \concept CORBAStubWithDestroy
 \brief Concept for a CORBA stub type that supports destruction.

 \details
 This concept extends \ref CORBAStub and adds the requirement
 that the type has a member function `destroy()` returning void.

 \tparam ty The candidate stub type that supports `destroy()`.
*/
template<typename ty>
concept CORBAStubWithDestroy = CORBAStub<ty> && 
   requires(ty& t) {
         { t.destroy() } -> std::same_as<void>;
   };

/**
 \concept CORBASkeleton
 \brief Concept for a valid CORBA server skeleton class (servant).

 \details
 This concept ensures that a type \c ty:
 - Is derived from \c PortableServer::ServantBase
 - Provides required member typedefs for its associated stub:
   - `_stub_type`, `_stub_ptr_type`, `_stub_var_type`
 - Implements required servant functionality:
   - `_this()` returning a valid stub pointer
   - `_interface_repository_id()` returning const char*
   - `_dispatch()` method accepting TAO request types
   - `_is_a()` method to check interface compatibility

 \tparam ty The candidate skeleton type (servant implementation).
 \note Used to constrain server-side CORBA servant templates.
*/
template<typename ty>
concept CORBASkeleton = std::derived_from<ty, PortableServer::ServantBase> &&
     requires(ty t, TAO_ServerRequest& req, TAO::Portable_Server::Servant_Upcall* upcall) {
     typename ty::_stub_type;
     typename ty::_stub_ptr_type;
     typename ty::_stub_var_type;

     { t._this() } -> std::same_as<typename ty::_stub_ptr_type>;
     { t._interface_repository_id() } -> std::same_as<const char*>;
     { t._dispatch(req, upcall) };
     { t._is_a("IDL:some/interface:1.0") } -> std::convertible_to<::CORBA::Boolean>;
   };


/**
 \class ORBBase
 \brief Base class to initialize and manage a CORBA ORB instance including naming context resolution.

 \details
 This class encapsulates initialization and shutdown of a CORBA ORB, and provides access to the
 CORBA Naming Service. It implements RAII semantics for robust resource management. It also supports
 move operations, allowing transfer of ownership between instances.

 The naming context is resolved immediately upon construction and can be used to interact with
 the CORBA Naming Service (e.g., for `rebind`, `resolve`, `unbind`).

 \note Copy operations are explicitly deleted to ensure safe ownership of ORB resources.
*/
class ORBBase {
private:
   std::string                  strName         = ""s;     ///< Logical name of the ORB instance (used for logs, identification)
   CORBA::ORB_var               orb_            = nullptr; ///< CORBA ORB instance handle
   CosNaming::NamingContext_var naming_context_ = nullptr; //< Root naming context resolved from ORB
public:
   /// \brief Deleted default constructor (ORBBase must be explicitly constructed)
   ORBBase() = delete;

   /**
    \brief Constructor initializes ORB and resolves NamingContext.
    \param paName Name of the ORB (used for logging)
    \param argc Argument count (from main)
    \param argv Argument vector (from main)
    \throws std::runtime_error if ORB initialization or NamingContext resolution fails
   */
   ORBBase(std::string const& paName, int argc, char* argv[]) : strName { paName } {
      orb_ = CORBA::ORB_init(argc, argv);
      if (CORBA::is_nil(orb_.in())) 
         throw std::runtime_error(std::format("[{} {}] Failed to initialized the ORB Object.", strName, ::getTimeStamp()));
      log_trace<10>("[{} {}] ORB initialized.", strName, ::getTimeStamp());

      CORBA::Object_var naming_obj = orb_->resolve_initial_references("NameService");
      naming_context_ = CosNaming::NamingContext::_narrow(naming_obj.in());
      if (CORBA::is_nil(naming_context_.in())) 
         throw std::runtime_error(std::format("[{} {}] Failed to narrow Naming Context.", strName, ::getTimeStamp()));
      log_trace<10>("[{} {}] Naming Service Context obtained.", strName, ::getTimeStamp());
      log_trace<9>("[{} {}] ORBBase created, ORB initialized and naming context obtained.", strName, ::getTimeStamp());
      }

   /// \brief Deleted copy constructor (non-copyable due to ORB ownership)
   ORBBase(ORBBase const&) = delete;

   /**
    \brief Move constructor.
    \param other Temporary ORBBase instance to move from.
    \post Transfers ownership of the ORB and naming context from \c other.
   */
   ORBBase(ORBBase&& other) noexcept  : strName(std::move(other.strName)), 
                                        orb_(std::move(other.orb_)), 
                                        naming_context_(std::move(other.naming_context_)) { 
      log_trace<9>("[{} {}] ORB moved.", strName, ::getTimeStamp());
      }


   /**
    \brief Destructor.
    \details Cleans up the ORB if initialized, invoking `destroy()` on shutdown.
    \note Catches and logs CORBA exceptions on destruction.
   */
   virtual ~ORBBase() {
      if (!CORBA::is_nil(orb_.in())) {
         // while (orb_->work_pending()) orb_->perform_work();
         try {
            orb_->destroy();
            log_trace<10>("[{} {}] ORB destroyed.", strName, ::getTimeStamp());
            }
         catch (CORBA::Exception const& ex) {
            log_error("[{} {}] CORBA Exception caught while destroying ORB: {}", strName, ::getTimeStamp(), toString(ex));
            }
         }
      log_trace<9>("[{} {}] ORBBase deleted.", strName, ::getTimeStamp());
      }

   /// \brief Deleted copy assignment operator (non-copyable due to ORB ownership)
   ORBBase& operator = (ORBBase const&) = delete;

   /**
    \brief Move assignment operator.
    \param other Temporary ORBBase instance to move from.
    \return Reference to this instance.
    \post Transfers ownership of the ORB and naming context from \c other.
   */
   ORBBase& operator = (ORBBase&& other) noexcept {
      if (this != &other) {
         strName = std::move(other.strName);
         orb_ = std::move(other.orb_);
         naming_context_ = std::move(other.naming_context_);
         log_trace<10>("[{} {}] ORBBase assigned and moved.", strName, ::getTimeStamp());
         }
      return *this;
      }

   /**
    \brief Swaps internal state with another ORBBase.
    \param other Another ORBBase instance to swap with.
    \note Useful for move-and-swap assignment idiom.
   */
   void swap(ORBBase& other) noexcept {
      using std::swap;
      swap(strName, other.strName);
      swap(orb_, other.orb_);
      swap(naming_context_, other.naming_context_);
      }

   /**
    \brief Returns the name of this ORB instance.
    \return Const reference to the logical name string.
   */
   virtual std::string const& Name() const { return strName; }

   /**
    \brief Accessor for the CORBA ORB instance (non-const).
    \return Pointer to the internal CORBA ORB.
   */
   virtual CORBA::ORB_ptr orb() { return orb_.in(); }

   /**
    \brief Accessor for the CORBA ORB instance (const).
    \return Const pointer to the internal CORBA ORB.
   */
   virtual CORBA::ORB* orb() const { return orb_.in(); }

   /**
    \brief Accessor for the resolved NamingContext.
    \return CORBA pointer to the naming context.
   */
   virtual CosNaming::NamingContext_ptr naming_context() { return naming_context_.in(); }

   /**
    \brief Returns all names currently registered in the NamingContext.
    \return Vector of registered service names.
    \note Utility function using helper \c get_all_names().
   */
   virtual std::vector<std::string> get_names() { return get_all_names(naming_context()); }
protected:

   /**
    \brief Returns internal name for logging and diagnostics.
    \return Const reference to internal name string.
   */
   virtual std::string const& Text() const { return strName; }
};


/**
  \class CORBAServer
  \brief Templated CORBA server class for managing multiple servant types.
 
  \tparam Skeletons Variadic list of CORBA servant skeleton types that fulfill the CORBASkeleton concept.
 
  \details
  This class manages multiple CORBA servants by:
  - Creating and managing a single servant POA (Persistent lifespan)
  - Registering each servant with the naming service
  - Activating and deactivating servant objects
  - Launching the ORB event loop in a background thread
  - Automatically cleaning up on shutdown
 
  The internal storage and indexing are tuple-based and rely on compile-time indices.

  \see \ref appserver for more informations
  \see \ref app_lifecycle more informations about the lifecycle of a corba server
  \see \ref firstservertemplate a first template with a single skeleton
 */
template <CORBASkeleton... Skeletons>
class CORBAServer : virtual public ORBBase {
protected:
   static constexpr std::size_t NumSkeletons = sizeof...(Skeletons);  ///< Number of skeletons handled
   static_assert(NumSkeletons > 0, "CORBAServer requires at least one skeleton");
private:
   PortableServer::POA_var        root_poa_    = {}; ///< Root POA reference
   PortableServer::POAManager_var poa_manager_ = {}; ///< POA Manager reference
   PortableServer::POA_var        servant_poa_ = {}; ///< Dedicated POA for activating servants
   std::thread                    orb_thread_;       ///< Background thread for ORB event loop
   std::chrono::milliseconds      wait_interval_;    ///< Wait interval for shutdown polling

   /**
    * \struct ServantData
    * \brief Stores metadata and handles for each servant
    * \tparam Skeleton The specific CORBA skeleton type
    */
   template<typename Skeleton>
   struct ServantData {
      using corba_stub_ty  = typename Skeleton::_stub_type;
      using corba_stub_var = typename Skeleton::_stub_var_type;
      static_assert(CORBAStub<corba_stub_ty>, "servent stub type dosn't satisfy the CORBAStub concept");

      Skeleton*                     servant = nullptr; ///< Pointer to activated servant
      corba_stub_var                stub_var;          ///< CORBA stub variable
      PortableServer::ObjectId_var  oid;               ///< Object ID in the POA
      CosNaming::Name               name;              ///< Name bound in the Naming Service
      std::function<void()>         cleanup = nullptr; ///< Optional cleanup function
      };

   std::tuple<ServantData<Skeletons>...> servant_data_;    ///< Tuple holding servant metadata

   /**
    * \brief Accessor alias for a skeleton type at index I
    * \tparam I Index of the skeleton in the parameter pack
    */
   template<std::size_t I> requires (I < NumSkeletons)
   using SkeletonType = std::tuple_element_t<I, std::tuple<Skeletons...>>;

   /**
    * \brief Internal helper to retrieve ServantData for index I
    * \tparam I Index of the servant
    * \return Reference to ServantData at index I
    */
   template<std::size_t I> 
   ServantData<SkeletonType<I>>& get_data() { return std::get<I>(servant_data_); }

public:
   CORBAServer() = delete;                                ///< Deleted default constructor
   CORBAServer(CORBAServer const&) = delete;              ///< Deleted copy constructor
   CORBAServer& operator = (CORBAServer const&) = delete; ///< Deleted copy assignment

   /**
    * \brief Constructs the CORBA server and initializes the POA hierarchy.
    * \param name Logical server name
    * \param argc Argument count (from main)
    * \param argv Argument vector (from main)
    * \param interval Polling interval for shutdown condition
    * \throws std::runtime_error if POA creation or narrowing fails
    */
   CORBAServer(std::string const& name, int argc, char* argv[], std::chrono::milliseconds interval = std::chrono::milliseconds{ 200 })
      : ORBBase(name, argc, argv), wait_interval_(interval) {
      CORBA::Object_var poa_object = orb()->resolve_initial_references("RootPOA");
      root_poa_ = PortableServer::POA::_narrow(poa_object.in());
      if (CORBA::is_nil(root_poa_.in()))
         throw std::runtime_error(std::format("[{} {}] Failed to narrow the RootPOA.", Text(), ::getTimeStamp()));

      poa_manager_ = root_poa_->the_POAManager();
      poa_manager_->activate();
      log_trace<10>("[{} {}] RootPOA obtained and POAManager is activated.", Text(), ::getTimeStamp());

      CORBA::PolicyList policies;
      policies.length(1);
      policies[0] = root_poa_->create_lifespan_policy(PortableServer::PERSISTENT);
      servant_poa_ = root_poa_->create_POA("ServantPOA", poa_manager_.in(), policies);
      for (CORBA::ULong i = 0; i < policies.length(); ++i) policies[i]->destroy();
      log_trace<10>("[{} {}] POA for the servant created.", Text(), ::getTimeStamp());
      log_trace<9>("[{} {}] CORBAServer created, RootPOA obrained and servant POA created.", Text(), ::getTimeStamp());
      }

   /** \brief Destructor shuts down and deactivates all registered servants */
   virtual ~CORBAServer() {
      shutdown_all();
      log_trace<9>("[{} {}] CORBAServer deleted.", Text(), ::getTimeStamp());
      }

   /**
    * \brief Unregister a servant and cleanup its resources
    * \tparam I Index of the servant
    */
   template<std::size_t I>
   void unregister() {
      auto& data = get_data<I>();
      if (data.name.length() > 0) {
         log_trace<10>("[{} {}] Unbinding {} from nameservice ...", Text(), data.name[0].id.in(), ::getTimeStamp());
         try {
            naming_context()->unbind(data.name);
            }
         catch (CORBA::Exception const& ex) {
            log_error("[{} {}] Error unbinding {} from nameservice (maybe already gone): {}", Text(), data.name[0].id.in(), ::getTimeStamp(), toString(ex));
            // we continue because we belief the nameservice is already gone
            }
         data.name.length(0);
         }

      if (data.servant) {
         log_trace<11>("[{} {}] Deactivate servant and remove refcount()...", Text(), ::getTimeStamp());
         servant_poa_->deactivate_object(data.oid.in());
         data.servant->_remove_ref();
         data.servant = nullptr;
         }

      if (data.cleanup) data.cleanup();
      data.cleanup = nullptr;
      }

   /**
    * \brief Register a servant and bind to Naming Service
    * \tparam I Index of the servant
    * \param name Binding name in naming service
    * \param servant Pointer to the servant instance
    */
   template<std::size_t I>
   void register_servant(std::string const& name, SkeletonType<I>* servant) {
      unregister<I>();
      auto& data = get_data<I>();
      if (name.size() > 0 && servant != nullptr) {
         data.servant = servant;

         data.oid = servant_poa_->activate_object(servant);
         CORBA::Object_var obj_ref = servant_poa_->id_to_reference(data.oid.in());
         data.stub_var = SkeletonType<I>::_stub_type::_narrow(obj_ref.in());
         if (CORBA::is_nil(data.stub_var)) {
            throw std::runtime_error(std::format("[{}::register_servant<{}> {}] CORBA Error while narrowing Reference.",
                                                 Text(), I, ::getTimeStamp()));
            }
         log_trace<11>("[{}::register_servant<{}> {}] servant activate under servant poa.", Text(), I, ::getTimeStamp());
         data.name.length(1);
         data.name[0].id = CORBA::string_dup(name.c_str());
         data.name[0].kind = CORBA::string_dup("Object");

         naming_context()->rebind(data.name, data.stub_var.in());
         log_trace<10>("[{}::register_servant<{}> {}] servant registered with nameservice as {}.",
            Text(), I, ::getTimeStamp(), name);
         data.cleanup = nullptr;
         }
      else {

         data.cleanup = nullptr;
         throw std::runtime_error(std::format("[{}::register_servant<{}> {}] name or servant are undefined.",
                                    Text(), I, ::getTimeStamp()));
         }
      }

   /**
    * \brief Register a servant with cleanup callback
    * \tparam I Index of the servant
    * \param name Binding name
    * \param cleanup_fn Cleanup lambda to be invoked during unregistration
    * \param servant Servant instance pointer
    */
   template<std::size_t I>
   void register_servant(std::string const& name, std::function<void()> cleanup_fn, SkeletonType<I>* servant) {
      register_servant<I>(name, servant);
      get_data<I>().cleanup = std::move(cleanup_fn);
      }

   /**
    * \brief Accessor to the servant pointer at index I
    * \tparam I Index in the skeleton pack
    * \return Raw pointer to the servant instance
    */
   template<std::size_t I> requires (I < NumSkeletons)
   SkeletonType<I>* get() { return get_data<I>().servant;  }

   /** \brief Unregisters and shuts down all servants */
   void shutdown_all() {
      [&] <std::size_t... Is>(std::index_sequence<Is...>) {
         (unregister<Is>(), ...);
         }(std::make_index_sequence<NumSkeletons>{});

      log_trace<9>("[{} {}] All servants shutdown.", Text(), ::getTimeStamp());
      }

   /**
    * \brief Launches the ORB loop in a background thread and waits for shutdown
    * \param shutdown_requested Flag for stopping the loop
    */
   void run(std::atomic<bool>& shutdown_requested) {
      orb_thread_ = std::thread([&]() {
         std::string strOrb = "ORB Thread for "s + Name();
         try {
            orb()->run();
            log_trace<9>("   [{} {}] orb->run() finished.", strOrb, ::getTimeStamp());
            }
         catch (CORBA::Exception const& ex) {
            log_error("  [{} {}], CORBA Exception in orb->run(): {}", strOrb, ::getTimeStamp(), toString(ex));
            }
         catch (std::exception const& ex) {
            log_error("  [{} {}], CORBA Exception in orb->run(): {}", strOrb, ::getTimeStamp(), ex.what());
            }
         catch (...) {
            log_error("  [{} {}], unknown Exception in orb->run()", strOrb, ::getTimeStamp());
            }
         });
      log_state("[{} {}] Server is ready. <Waiting for shutdown signal (e.g. Cntrl+C) ...", Text(), ::getTimeStamp());
      while (!shutdown_requested) {
         std::this_thread::sleep_for(wait_interval_);
         }
      log_trace<9>("[{} {}] Server finsihing, ...", Text(), ::getTimeStamp());
      }

   /**
     * \brief Returns the root POA used for initializing the POA hierarchy.
     * \return A pointer to the Root POA used as the base for servant POA creation.
     */
   PortableServer::POA_ptr root_poa() { return root_poa_.in(); }

   /**
    * \brief Returns the POA manager associated with the Root POA.
    * \return A pointer to the POA manager used to activate and control POAs.
    */
   PortableServer::POAManager_ptr poa_manager() { return poa_manager_.in(); }

   /**
    * \brief Returns the POA dedicated to activating and managing the registered servants.
    * \return A pointer to the persistent POA used for servant activation.
    */
   PortableServer::POA_ptr servant_poa() { return servant_poa_.in(); }
};


/**
 \class CORBAClient
 \brief Generic variadic CORBA client template for resolving multiple CORBA service stubs via Naming Service.

 \tparam Stubs A parameter pack of CORBA stub interface types (e.g., MyService1, MyService2), each must define _var_type and _obj_type.

 \details
    This generalized client class initializes the ORB and resolves multiple CORBA service references in a uniform and safe way.
    For each given interface in the template parameter pack, a corresponding service name must be provided at construction time.

    Key features:
    - Compile-time fixed number of stub types using variadic templates
    - Automatic resolution of each stub from the Naming Service
    - Tuple-based storage of resolved `_var_type` smart references
    - Index-sequenced resolution and cleanup logic for scalable management

 \note The class is non-copyable and requires explicit service names matching the template parameter count.
 \see \ref appclient for the workcircle of a client
 \see \ref ORBClientPage for the single-stub version of this.
 \todo Extend for nested naming contexts (multi-level CosNaming::Name support)
 \todo Optional policy injection (timeouts, retries, etc.)
 \todo Support for lazy resolution or runtime stub selection
*/
template <CORBAStub... Stubs>
class CORBAClient : virtual public ORBBase {
protected:
   static constexpr std::size_t NumStubs = sizeof...(Stubs);   ///< Number of CORBA services to resolve
   static_assert(NumStubs > 0, "CORBAClient requires at least one stub");
public:
   using VarTuple = std::tuple<typename Stubs::_var_type...>;  ///< Tuple of CORBA smart stub types
   
   /**
    \brief Alias to access the CORBA stub interface type at a given index.

    \tparam I Index of the stub in the template parameter pack \c Stubs.
    \return Corresponding stub interface type (i.e., the actual CORBA object type such as \c MyService::_obj_type).

    \details
     This alias resolves the interface type stored at position \c I in the internal \c VarTuple.
     It is used to allow type-safe access and narrow operations on CORBA stubs.

    \note The index \c I must be in range \c [0, NumStubs).
   */
   template <std::size_t I>
   using StubInterface = typename std::tuple_element_t<I, VarTuple>::_obj_type;

private:
   using NameArray = std::array<std::string, NumStubs>;        ///< Service name array (1 per stub)

   VarTuple stubs_;          ///< Tuple of resolved stub instances
   NameArray names_;         ///< Names used for resolution in Naming Service

   /**
     \brief Resolves all stubs using the provided service names.
     \tparam Is Parameter pack of compile-time indices (0 to N-1) used to expand stub resolution
     \details The param std::index_sequence<Is...> is a Helper to expand the parameter pack via fold expression
     \note Uses a fold-expression over resolve_single<I>() to resolve each stub
    */
   template <std::size_t... Is>
   void resolve_all(std::index_sequence<Is...>) { ( ..., resolve_single<Is>() ); }
   
   /**
     \brief Resolves a single CORBA service stub.
     \tparam I The index of the stub (corresponds to service name at position I)
     \throws std::runtime_error if the service cannot be resolved or narrowed
     \post The resolved stub is stored in \c stubs_[I]
    */
   template <std::size_t I>  
   void resolve_single() {
      using VarType = std::tuple_element_t<I, VarTuple>;
      using StubInterface = typename VarType::_obj_type;

      auto const& strService = names_[I];

      CosNaming::Name_var name = new CosNaming::Name;
      name->length(1);
      name[0].id = CORBA::string_dup(strService.c_str());
      name[0].kind = CORBA::string_dup("Object");

      log_trace<2>("[{} {}] Resolving {}.", Name(), ::getTimeStamp(), strService);
      CORBA::Object_var factory_obj = naming_context()->resolve(name);

      auto stub = StubInterface::_narrow(factory_obj.in());
      if (CORBA::is_nil(stub)) {
         throw std::runtime_error(std::format("Failed to narrow factory reference for {1:} in {0:}.", Name(), strService));
         }
      std::get<I>(stubs_) = stub;
      log_trace<2>("[{} {}] Successfully obtained reference for {}.", Name(), ::getTimeStamp(), strService);
      }
 
   /**
     \brief Releases all CORBA references.
     \tparam Is Parameter pack of compile-time indices for each stub
     \details The Param std::index_sequence<Is...> is a Helper to expand \c release_single<Idx>() via fold expression
     \note Calls _nil() on each stub instance
    */
   template <std::size_t... Is>
   void release_all(std::index_sequence<Is...>) { (..., release_single<Is>()); }

   /**
     \brief Releases a single stub by resetting its reference to nil.
     \tparam Idx Index of the stub to release
     \post The stub at index Idx is set to a nil reference
    */
   template <std::size_t Idx>
   void release_single() {
      std::get<Idx>(stubs_) = std::tuple_element_t<Idx, VarTuple>::_obj_type::_nil();
      log_trace<8>("[{} {}] Successfully released reference for {} .", Name(), ::getTimeStamp(), names_[Idx]);
      }

public:
   /**
     \brief Deleted default constructor. Requires service names and ORB initialization.
    */
   CORBAClient() = delete;

   /**
     \brief Constructs the client, initializes the ORB, and resolves all stubs.

     \tparam Names A variadic template parameter pack; each type must be convertible to std::string.
                   The number of parameters must exactly match the number of CORBA stub types (\c Stubs).

     \param paClientName Logical name for the client (used in logging and ORBBase)
     \param argc Argument count (typically from \c main)
     \param argv Argument vector (typically from \c main)
     \param names Variadic list of service names to resolve (in order, one per stub)

     \throws std::runtime_error if any service name cannot be resolved or narrowed to the expected stub type

     \post All \c Stubs are resolved and stored in the internal \c stubs_ tuple
     \note The constructor uses static_assert-style constraints via \c requires to ensure proper usage
     \see resolve_all for the internal resolution logic
    */
   template<typename... Names> requires (sizeof...(Names) == NumStubs) && (std::is_convertible_v<Names, std::string> && ...)
   CORBAClient(std::string const& paClientName, int argc, char* argv[], Names&&... names) : ORBBase(paClientName, argc, argv),
            names_ { { std::forward<Names>(names)... } } {
      resolve_all(std::make_index_sequence<NumStubs>{});
      log_trace<2>("[{} {}] All references obtained successfully.", Name(), ::getTimeStamp());
      }

   /**
     \brief Deleted copy constructor to prevent copying.
    */
   CORBAClient(CORBAClient const&) = delete;

   /**
     \brief Deleted copy assignment operator.
    */
   CORBAClient& operator = (CORBAClient const&) = delete;

   /**
     \brief Destructor. Releases all references.
    */
   ~CORBAClient() { 
      release_all(std::make_index_sequence<NumStubs>{});  
      log_trace<2>("[{} {}] All references released successfully.", Name(), ::getTimeStamp());
      }

   /**
     \brief Accessor for the CORBA stub at the given index with lazy resolution.

     \tparam Idx Index of the stub in the template parameter pack \c Stubs.
     \return A raw CORBA pointer of type `StubInterface<Idx>*` representing the resolved service.

     \details
     This function checks whether the stub at index \c I has already been resolved.
     If not, it calls \c resolve_single<Idx>() to perform Naming Service lookup and narrowing.

     The stub is stored internally as a \c _var_type, and this function returns the underlying raw pointer using \c .in().

     \throws std::runtime_error if resolution or narrowing fails during lazy initialization.

     \note This method uses lazy resolution. The ORB and Naming Service must be available at the time of the first call.
     \see resolve_single
     \see StubInterface
   */
   template<std::size_t Idx>
   StubInterface<Idx>* get() {
      if (CORBA::is_nil(std::get<Idx>(stubs_))) {
         resolve_single<Idx>();
         }
      return std::get<Idx>(stubs_).in();
      }

   /**
     \brief Accessor for the tuple of resolved stub instances (mutable).
     \return Reference to the internal VarTuple
    */
   VarTuple& vars() { return stubs_; }

   /**
     \brief Accessor for the tuple of resolved stub instances (const).
     \return Const reference to the internal VarTuple
    */
   VarTuple const& vars() const { return stubs_; }

   /**
     \brief Accessor for the service name array (mutable).
     \return Reference to the internal name array
    */
   NameArray& names() { return names_; }

   /**
     \brief Accessor for the service name array (const).
     \return Const reference to the internal name array
    */
   NameArray const& names() const { return names_; }

};


/*
* 
* Corner of Hell
* 
template <CORBAStub... Stubs, CORBASkeleton... Skels>
class CORBAClientServer : public CORBAClient<Stubs>, public CORBAServer<Skels> {


template <typename... tys>
class CORBAClientServer : public CORBAClient<Stubs>, public CORBAServer<Skels> {

*/


/**
  \brief RAII wrapper for CORBA stubs supporting the 'destroy()' method.
 
  \tparam corba_ty The CORBA stub interface type with 'destroy()'.
 
  \details
  This class manages the lifecycle of CORBA stub pointers which provide a 'destroy()' method.
  Upon destruction, 'destroy()' is called to release resources properly.
  Copy operations are deleted to avoid double destroy calls. Move semantics are supported.
 */
template<CORBAStubWithDestroy corba_ty>
class Destroyable_Var {
public:
   using ptr_type       = corba_ty*;
   using const_ptr_type = const corba_ty*;
   using var_type       = typename corba_ty::_var_type; // TAO_Objref_Var_T<corba_ty>;

private:
   var_type var_;
public:

   Destroyable_Var() { var_ = typename corba_ty::_nil(); }
   explicit Destroyable_Var(corba_ty* ptr) { var_ = ptr; }

   Destroyable_Var(Destroyable_Var const&) = delete;
   Destroyable_Var& operator = (Destroyable_Var const&) = delete;

   Destroyable_Var(Destroyable_Var&& other) noexcept : var_(std::move(other.var_)) {
      other.var_ = nullptr;
      }

   Destroyable_Var& operator=(Destroyable_Var&& other) noexcept {
      if (this != &other) {
         maybe_destroy();
         var_ = std::move(other.var_);
         other.var_ = nullptr;
         }
      return *this;
      }

   var_type& get() { return var_; }
   var_type const& get() const { return var_; }

   ptr_type operator->() { return var_.in(); }
   ptr_type in() { return var_.in(); }

   const_ptr_type operator->() const { return var_.in(); }
   const_ptr_type in() const { return var_.in(); }

   ~Destroyable_Var() { maybe_destroy(); }

private:
   void maybe_destroy() {
      if (var_.in() != nullptr) {
         try {
            var_->destroy();
            }
         catch (...) {
            // Fehlerbehandlung optional, auf jeden Fall loggen
            }
         var_ = corba_ty::_nil();
         }
      }

};

/**
  \brief Helper function to create a 'Destroyable_Var' from a raw CORBA stub pointer.
 
  \param elem Raw pointer to a CORBA stub interface supporting 'destroy()'.
  \return 'Destroyable_Var' managing the given pointer.
 */
template <CORBAStubWithDestroy corba_ty>
auto make_destroyable(corba_ty* elem) {
   return Destroyable_Var<corba_ty>(elem);
   }



