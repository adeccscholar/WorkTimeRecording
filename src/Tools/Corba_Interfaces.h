// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Header-only CORBA client utility templates with RAII management for CORBA interfaces and concepts

  \details This header provides generic, reusable C++20 template utilities for working with CORBA stubs in the
           distributed time-tracking system developed as part of the **adecc Scholar** project. The templates
           simplify CORBA client initialization, naming service resolution, and resource management, leveraging
           modern C++ features like concepts and RAII.

  \details Key features include:
           - The `ORBClient` template class to initialize the ORB, resolve and narrow a CORBA object reference
             from the Naming Service by service name, and provide safe access to the CORBA factory/stub object.
           - Concept constraints (`CORBAStub`, `CORBAStubWithDestroy`) that ensure template parameters are
             valid CORBA stub types and optionally support a `destroy()` method.
           - The `Destroyable_Var` RAII wrapper class template manages CORBA stub pointers which support
             a `destroy()` method, automatically calling `destroy()` on destruction to properly release
             CORBA resources.
           - A helper factory function `make_destroyable` to create `Destroyable_Var` instances from raw pointers.

  \details These utilities are used throughout the distributed system to manage CORBA object lifecycles safely,
           prevent resource leaks, and encapsulate boilerplate CORBA naming service lookups and ORB setup.

  \details The project is part of the adecc Scholar educational initiative, demonstrating a
           distributed, cross-platform time-tracking system implemented with CORBA (TAO),
           Boost.SML, Qt6, and modern C++20 features.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \license This project is mostly licensed under the GNU General Public License v3.0.
           See the LICENSE file for details.

  \version 1.0
  \date    31.05.2025
*/

#pragma once

#include "my_logging.h"
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

template<typename ty>
concept CORBAStubWithDestroy = CORBAStub<ty> && 
   requires(ty& t) {
         { t.destroy() } -> std::same_as<void>;
   };


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



class ORBBase {
private:
   std::string                  strName         = ""s;
   CORBA::ORB_var               orb_            = nullptr;
   CosNaming::NamingContext_var naming_context_ = nullptr;
public:
   ORBBase(std::string const& paName, int argc, char* argv[]) : strName { paName } {
      orb_ = CORBA::ORB_init(argc, argv);
      if (CORBA::is_nil(orb_.in())) 
         throw std::runtime_error(std::format("[{} {}] Failed to initialized the ORB Object.", strName, ::getTimeStamp()));
      log_trace<2>("[{} {}] ORB initialized.", strName, ::getTimeStamp());

      CORBA::Object_var naming_obj = orb_->resolve_initial_references("NameService");
      naming_context_ = CosNaming::NamingContext::_narrow(naming_obj.in());
      if (CORBA::is_nil(naming_context_.in())) 
         throw std::runtime_error(std::format("[{} {}] Failed to narrow Naming Context.", strName, ::getTimeStamp()));
      log_trace<2>("[{} {}] Naming Service Context obtained.", strName, ::getTimeStamp());
      }

   ~ORBBase() {
      if (!CORBA::is_nil(orb_.in())) {
         // while (orb_->work_pending()) orb_->perform_work();
         try {
            orb_->destroy();
            log_trace<2>("[{} {}] ORB destroyed.", strName, ::getTimeStamp());
            }
         catch (CORBA::Exception const& ex) {
            log_error("[{} {}] CORBA Exception caught while destroying ORB: {}", strName, ::getTimeStamp(), toString(ex));
            }
         }
      }

   ORBBase() = delete;
   ORBBase(ORBBase const&) = delete;
   ORBBase& operator = (ORBBase const&) = delete;

   std::string const& Name() const { return strName; }

   CORBA::ORB_ptr orb() { return orb_.in(); }
   CORBA::ORB* orb() const { return orb_.in(); }

   CosNaming::NamingContext_ptr naming_context() { return naming_context_.in(); }

   std::vector<std::string> get_names() { return get_all_names(naming_context()); }
protected:
   std::string const& Text() const { return strName; }
};


template <CORBASkeleton corba_skel_ty>
class CorbaServer : virtual public ORBBase {
private:
   using corba_stub_ty  = typename corba_skel_ty::_stub_type;
   using corba_stub_var = typename corba_skel_ty::_stub_var_type;
   static_assert(CORBAStub<corba_stub_ty>, "servent stub type dosn't satisfy the CORBAStub concept");

   PortableServer::POA_var        root_poa_     = PortableServer::POA::_nil();
   PortableServer::POAManager_var poa_manager_  = PortableServer::POAManager::_nil();
   PortableServer::POA_var        servant_poa_  = PortableServer::POA::_nil();
   PortableServer::ObjectId_var   servant_oid_  = {};
   CosNaming::Name                servant_name_;
   corba_skel_ty*                 servant_      = nullptr;  
   corba_stub_var                 servant_var_  = {}; // corba_stub_ty::_narrow();
   std::function<void()>          cleanup_func_ = nullptr;
   std::thread                    orb_thread;
public:
   CorbaServer() = delete;
   CorbaServer(std::string const& pName, int argc, char* argv[]) : ORBBase(pName, argc, argv) {
      CORBA::Object_var poa_object = orb()->resolve_initial_references("RootPOA");
      root_poa_ = PortableServer::POA::_narrow(poa_object.in());
      if (CORBA::is_nil(root_poa_.in())) 
         throw std::runtime_error(std::format("[{} {}] Failed to narrow the POA.", Text(), ::getTimeStamp()));

      poa_manager_ = root_poa_->the_POAManager();
      poa_manager_->activate();
      log_trace<2>("[{} {}] RootPOA obtained and POAManager is activated.", Text(), ::getTimeStamp());

      CORBA::PolicyList pols;
      pols.length(1);
      pols[0] = root_poa_->create_lifespan_policy(PortableServer::PERSISTENT);


      servant_poa_ = root_poa_->create_POA("ServantPOA", poa_manager_.in(), pols);
      log_trace<2>("[{} {}] POA for the servant created.", Text(), ::getTimeStamp());

      for (uint32_t i = 0; i < pols.length(); ++i) pols[i]->destroy();
      }

   ~CorbaServer() {
      UnRegisterServant();
      log_trace<2>("[{} {}] Destroying servant POA ...", Text(), ::getTimeStamp());
      servant_poa_->destroy(true, true);

      log_trace<2>("[{} {}] Shutdown ORB ...", Text(), ::getTimeStamp());
      orb()->shutdown(true);
      if (orb_thread.joinable()) {
         log_trace<2>("[{} {}] Join the ORB Thread to wait ...", Text(), ::getTimeStamp());
         orb_thread.join();
         }
      else {
         log_trace<2>("[{} {}] ORB Thread isn't joinable, maybe already gone ...", Text(), ::getTimeStamp());
         }
      log_trace<2>("[{} {}] ORB Thread gone ...", Text(), ::getTimeStamp());
      }

   void RegisterServant(std::string const& pName, corba_skel_ty* pServant) {
      UnRegisterServant();

      if(pName.size() > 0 && pServant  != nullptr) {
         servant_ = pServant;
         servant_oid_ = servant_poa_->activate_object(servant_);

         CORBA::Object_var obj_ref = servant_poa_->id_to_reference(servant_oid_.in());
         servant_var_ = corba_stub_ty::_narrow(obj_ref.in());
         if (CORBA::is_nil(servant_var_)) {
            throw std::runtime_error(std::format("[{} {}] CORBA Error while narrowing Reference.", 
                                           "CORBAServent::RegisterServant", ::getTimeStamp()));
            }
         log_trace<2>("[{} {}] Company servant activate under CompanyPOA.", "CORBAServant::RegisterServant", ::getTimeStamp());
         }

      servant_name_.length(1);
      servant_name_[0].id = CORBA::string_dup(pName.c_str());
      servant_name_[0].kind = CORBA::string_dup("Object");

      naming_context()->rebind(servant_name_, servant_var_.in());
      std::println("[{} {}] Company servant registered with nameservice as {}.", Text(), ::getTimeStamp(), pName);

      cleanup_func_ = nullptr;
      }

   void RegisterServant(std::string const& pName, std::function<void()>&& cleanup_fn, corba_skel_ty* pServant) {
      RegisterServant(pName, pServant);
      std::swap(cleanup_func_, cleanup_fn);
      }

   void UnRegisterServant() {
      if (servant_name_.length() > 0) {
         log_trace<2>("[{} {}] Unbinding {} from nameservice ...", Text(), servant_name_[0].id.in(), ::getTimeStamp());
         try {
            naming_context()->unbind(servant_name_);
            }
         catch (CORBA::Exception const& ex) {
            log_error("[{} {}] Error unbinding {} from nameservice (maybe already gone): {}", Text(), servant_name_[0].id.in(), ::getTimeStamp(), toString(ex));
            // we continue because we belief the nameservice is already gone
            }
         servant_name_.length(0);
         }

      if(servant_ != nullptr) { //} && !CORBA::is_nil(servant_oid_.in())) {
         std::println("[{} {}] Deactivate servant and remove refcount()...", Text(), ::getTimeStamp());
         servant_poa_->deactivate_object(servant_oid_);
         servant_->_remove_ref();
         servant_ = nullptr; //  typename corba_skel_ty::_nil();
         }

      if (cleanup_func_ != nullptr) cleanup_func_();
      }

   void run(std::atomic<bool>& shutdown_requested) {
      // ---------------------------------------------------------------------------------
      //  Launch the ORB event loop in a separate thread
      // ---------------------------------------------------------------------------------
      log_trace<2>("[{} {}] Starting ORB event loop in separate thread ...", Name(), ::getTimeStamp());


      orb_thread = std::thread([&]() {
         std::string strOrb = "ORB Thread "s + Name();
         try {
            orb()->run();
            log_trace<2>("   [{} {}] orb->run() finished.", strOrb, ::getTimeStamp());
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
      // ---------------------------------------------------------------------------------
      // Wait for shutdown signal in main thread (soft closing)
      // ---------------------------------------------------------------------------------
      log_state("[{} {}] Server is ready. <Waiting for shutdown signal (e.g. Cntrl+C) ...", Name(), ::getTimeStamp());
      while (!shutdown_requested) {
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         }
      log_state("[{} {}] Server finsihing, ...", Name(), ::getTimeStamp());
      }

   CorbaServer(CorbaServer const&) = delete;
   CorbaServer& operator = (CorbaServer const&) = delete;

   PortableServer::POA_ptr root_poa() { return root_poa_.in(); }
   PortableServer::POAManager_ptr poa_manager() { return poa_manager_.in(); }
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
 \see \ref ORBClientPage for the single-stub version
 \todo Extend for nested naming contexts (multi-level CosNaming::Name support)
 \todo Optional policy injection (timeouts, retries, etc.)
 \todo Support for lazy resolution or runtime stub selection
*/
template <CORBAStub... Stubs>
class CORBAClient : virtual public ORBBase {
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
   static constexpr std::size_t NumStubs = sizeof...(Stubs);   ///< Number of CORBA services to resolve
   using NameArray = std::array<std::string, NumStubs>;        ///< Service name array (1 per stub)

   VarTuple stubs_;          ///< Tuple of resolved stub instances
   NameArray names_;         ///< Names used for resolution in Naming Service

   /**
     \brief Resolves all stubs using the provided service names.
     \tparam Is Parameter pack of compile-time indices (0 to N-1) used to expand stub resolution
     \param index_sequence Helper to expand the parameter pack via fold expression
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
     \param index_sequence Helper to expand \c release_single<I>() via fold expression
     \note Calls _nil() on each stub instance
    */
   template <std::size_t... Is>
   void release_all(std::index_sequence<Is...>) { (..., release_single<Is>()); }

   /**
     \brief Releases a single stub by resetting its reference to nil.
     \tparam I Index of the stub to release
     \post The stub at index I is set to a nil reference
    */
   template <std::size_t I>
   void release_single() {
      std::get<I>(stubs_) = std::tuple_element_t<I, VarTuple>::_obj_type::_nil();
      log_trace<2>("[{} {}] Successfully released reference for {} .", Name(), ::getTimeStamp(), names_[I]);
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

     \tparam I Index of the stub in the template parameter pack \c Stubs.
     \return A raw CORBA pointer of type \c StubInterface<I>* representing the resolved service.

     \details
     This function checks whether the stub at index \c I has already been resolved.
     If not, it calls \c resolve_single<I>() to perform Naming Service lookup and narrowing.

     The stub is stored internally as a \c _var_type, and this function returns the underlying raw pointer using \c .in().

     \throws std::runtime_error if resolution or narrowing fails during lazy initialization.

     \note This method uses lazy resolution. The ORB and Naming Service must be available at the time of the first call.
     \see resolve_single
     \see StubInterface
   */
   template<std::size_t I>
   StubInterface<I>* get() {
      if (CORBA::is_nil(std::get<I>(stubs_))) {
         resolve_single<I>();
         }
      return std::get<I>(stubs_).in();
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
   using ptr_type       = typename corba_ty*;
   using const_ptr_type = typename const corba_ty*;
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
         var_ = typename corba_ty::_nil();
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



