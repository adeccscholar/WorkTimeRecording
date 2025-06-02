// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Header-only CORBA client utility templates with RAII management for CORBA interfaces

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
#include <orbsvcs/CosNamingC.h>

#include <concepts>
#include <vector>
#include <string>
#include <tuple>
#include <array>

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
      while (orb_->work_pending()) orb_->perform_work();
      orb_->destroy();
      log_trace<2>("[{} {}] ORB destroyed.", strName, ::getTimeStamp());
   }

   ORBBase(ORBBase const&) = delete;
   ORBBase& operator = (ORBBase const&) = delete;

   std::string const& Name() const { return strName; }

   CORBA::ORB_ptr orb() { return orb_.in(); }
   CORBA::ORB* orb() const { return orb_.in(); }

   CosNaming::NamingContext_ptr naming_context() { return naming_context_.in(); }
};

/**
  \brief Generic CORBA client helper to initialize ORB and resolve a service stub via Naming Service.
 
  \tparam corba_ty The CORBA stub interface type.
 
  \details
  This class template encapsulates common ORB initialization and Naming Service resolution
  for a given CORBA interface type. It performs:
  - ORB initialization with command line arguments,
  - Resolution of the Naming Service context,
  - Lookup and narrowing of the named CORBA service,
  providing safe accessors to the ORB and factory interface.
 */
template <CORBAStub corba_ty>
class ORBClient : virtual ORBBase {
   using corba_var_ty = typename corba_ty::_var_type;

   std::string                  strService     = ""s;
   corba_var_ty                 factory_       = nullptr;
public:
   ORBClient() = delete;
   ORBClient(std::string const& paName, int argc, char* argv[], std::string const& paService) : ORBBase(paName, argc, argv), 
                 strService { paService }  {
      // test
      auto overview = get_all_names(naming_context());
      for (auto const& id_name : overview) std::println(std::cout, "{}", id_name);

      CosNaming::Name_var name = new CosNaming::Name;
      name->length(1);
      name[0].id   = CORBA::string_dup(strService.c_str());
      name[0].kind = CORBA::string_dup("Object");

      log_trace<2>("[{} {}] Resolving {}.", Name(), ::getTimeStamp(), strService);
      CORBA::Object_var factory_obj = naming_context()->resolve(name);

      // CORBA::string_free(name[0].id);
      // CORBA::string_free(name[0].kind);

      factory_ = typename corba_ty::_narrow(factory_obj.in());
      if (CORBA::is_nil(factory_.in())) throw std::runtime_error(std::format("Failed to narrow factory reference for {1:} in {0:}.", 
                                 Name(), strService));
      log_trace<2>("[{} {}] Successfully obtained reference for {}.", Name(), ::getTimeStamp(), strService);
      
      }


   ~ORBClient() {
      }

   ORBClient(ORBClient const&) = delete;
   ORBClient& operator = (ORBClient const&) = delete;

   CORBA::ORB_ptr orb() { return orb_.in();  }
   CORBA::ORB* orb() const { return orb_.in(); }

   corba_ty* factory() { return factory_.in(); }
   };


template <CORBAStub... Stubs>
class CORBAStubHolder {
private:
   using VarTuple = std::tuple<typename Stubs::_var_type...>;
   static constexpr std::size_t NumStubs = sizeof...(Stubs);
   using NameArray = std::array<std::string, NumStubs>;

   VarTuple stubs_;
   NameArray names_;

   std::string                  strClientName  = ""s;
   CORBA::ORB_var               orb_           = nullptr;
   CosNaming::NamingContext_var naming_context = nullptr;
public:
   CORBAStubHolder() = delete;

   // Variadic Konstruktor: erwartet genau so viele std::string wie Stubs
   template <typename... Names> requires (sizeof...(Names) == NumStubs) && (std::is_convertible_v<Names, std::string> && ...)
   CORBAStubHolder(std::string const& paClientName, int argc, char* argv[], Names&&... names) : 
      strClientName { paClientName }, names_{ { std::forward<Names>(names)... } } {
      }

   VarTuple& vars() { return stubs_; }
   VarTuple const& vars() const { return stubs_; }

   NameArray& names() { return names_; }
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
   using var_type       = TAO_Objref_Var_T<corba_ty>;

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
         var_ = nullptr;
         }
      }

   var_type var_;
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



