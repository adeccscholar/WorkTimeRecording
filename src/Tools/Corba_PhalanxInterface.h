// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Dynamic multi-server CORBA client implementation for homogenous interface access (Phalanx pattern).
 
  \details
  This header defines the \ref CORBAClientPhalanx class, a CORBA client pattern designed for managing
  a dynamic collection of service connections of the **same CORBA interface type** across multiple server instances.
  It is especially useful in scenarios such as:
  - Load-balanced service groups
  - High-availability clusters with transient endpoints
  - Distributed replicas of a single functional unit
 
  \par Key Concepts and Design
  This class builds on top of the \ref ORBBase infrastructure and provides:
  - Lazy `NamingService`-based connection management (`connect()`)
  - Ownership and cleanup of multiple `_var_type` references
  - Index-based access to individual service connections via `operator[]`
  - Safe and explicit resource release (`remove()` and destructor)
 
  \par Template Requirements
  - \c T must be a CORBAStub type (i.e., provide `_var_type` and `_obj_type`)
  - The interface is stored and accessed through smart references (\c _var_type)
 
  \par Modern C++ Techniques Used
  - Template metaprogramming for interface type enforcement
  - RAII for connection lifetime management
  - STL containers (e.g., \c std::vector and \c std::string) for flexible and safe dynamic behavior
  - Lazy evaluation and runtime control over service resolution
 
  \par Integration Notes
  This pattern extends the `CORBAClient` model by allowing **runtime addition** of connections,
  unlike the original `CORBAClient<...>` which binds all stubs at compile time.
 
  The class is intentionally non-copyable and non-movable to tightly control ORB and connection behavior.
 
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \date 17.06.2025
  \version 1.0
 
  \copyright
  Copyright © 2025 adecc Systemhaus GmbH
 
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

 */

#pragma once

#include "Corba_Interfaces.h"

/**
  \brief Client container for managing multiple instances of the same CORBA stub type connected to different servers.
 
  \tparam Stub The CORBA stub interface type, must define _var_type and _obj_type.
 
  \details
  CORBAClientPhalanx allows deferred (lazy) resolution of multiple connections of the same CORBA interface type
  from a naming service. It maintains ownership of all stub instances internally and provides dynamic
  connection, cleanup, and indexed access. Useful when talking to many replicas of the same service.
 */
template<CORBAStub Stub>
class CORBAClientPhalanx : virtual public ORBBase {
public:
   using StubVar = typename Stub::_var_type;
   using StubObj = typename Stub::_obj_type;

   /// Structure representing a single resolved entry
   struct Entry {
      std::string name;  ///< The NameService entry this stub was resolved from
      StubVar stub;      ///< The resolved CORBA stub
   };

private:
   std::vector<Entry> stubs_; ///< Vector holding all resolved entries

public:
   /// \brief Constructs the client without resolving any stubs yet
   CORBAClientPhalanx(const std::string& name, int argc, char* argv[])
      : ORBBase(name, argc, argv) {
   }

   /// \brief Destructor that releases all held CORBA references
   ~CORBAClientPhalanx() {
      for (auto& e : stubs_) {
         e.stub = StubObj::_nil();
         }
      log_trace<2>('[{} {}] CORBAClientPhalanx destroyed with {} connections.', Name(), ::getTimeStamp(), stubs_.size());
      }

   /// \brief Connects to a new instance and appends it to the list
   /// \param service_name The service name in the Naming Service
   /// \throws std::runtime_error if resolution or narrowing fails
   void connect(std::string const& service_name) {
      CosNaming::Name_var name = new CosNaming::Name;
      name->length(1);
      name[0].id = CORBA::string_dup(service_name.c_str());
      name[0].kind = CORBA::string_dup("Object");

      log_trace<2>('[{} {}] Connecting to {}', Name(), ::getTimeStamp(), service_name);
      CORBA::Object_var obj = naming_context()->resolve(name);
      StubVar narrowed = StubObj::_narrow(obj.in());

      if (CORBA::is_nil(narrowed)) {
         throw std::runtime_error(std::format("Failed to narrow stub for {}", service_name));
      }

      stubs_.emplace_back(Entry{ service_name, narrowed });
      log_trace<2>('[{} {}] Connected to {}', Name(), ::getTimeStamp(), service_name);
   }

   /// \brief Returns the number of connected stubs
   std::size_t size() const noexcept { return stubs_.size(); }

   /// \brief Access operator for indexed stub access
   /// \param idx Index of the target connection
   /// \return Raw CORBA pointer for the given stub
   /// \throws std::out_of_range if the index is invalid
   StubObj* operator[](std::size_t idx) {
      return stubs_.at(idx).stub.in();
   }

   /// \brief Removes a connection by index
   /// \param idx Index to remove
   /// \throws std::out_of_range if the index is invalid
   void remove(std::size_t idx) {
      if (idx >= stubs_.size()) throw std::out_of_range("Invalid index in CORBAClientPhalanx::remove()");
      stubs_[idx].stub = StubObj::_nil();
      stubs_.erase(stubs_.begin() + idx);
      log_trace<3>('[{} {}] Removed connection at index {}', Name(), ::getTimeStamp(), idx);
   }

   /// \brief Returns const access to all entries
   const std::vector<Entry>& entries() const noexcept { return stubs_; }
};
