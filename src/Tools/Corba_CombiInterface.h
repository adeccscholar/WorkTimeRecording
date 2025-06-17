// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Generic CORBA Client/Server composition with wrapper concepts.

  \details
  This header defines a generic, type-safe and composable system for
  managing CORBA stubs and skeletons via wrapper templates and concepts.

  \details
  Its provides type-safe wrappers and composition tools to combine CORBA stubs and skeletons
  into a single class through template metaprogramming. Core features include:
   - Wrapper types for CORBA stubs (\c Stub) and skeletons (\c Skel)
   - Concepts to ensure compile-time interface compliance
   - Compile-time type filtering and extraction using typelists
   - A generic \c CORBAClientServer class that automatically inherits from appropriate base classes

  \details
  A class CORBAClientServer<CORBAStub ...stubs, CORBASkeleton ...skels> can't work
  because you need two paramter packs. The Rule of Greedy matching here and make
  it impossible, the Rule of fair matching for template pack deduction too.
  This is often called as Corner of Hell. And yes, we are in the Hell really
  when we arrive this point in the meta programming.
  \see Andrei Alexandrescu: https://www.youtube.com/watch?v=va9I2qivBOA

  \date 16.06.2025
  \version 1.0

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \license This program is free software: you can redistribute it and/or modify it
           under the terms of the GNU General Public License, version 3,
           as published by the Free Software Foundation.

           This program is distributed in the hope that it will be useful,
           but WITHOUT ANY WARRANTY; without even the implied warranty of
           MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
           See the GNU General Public License for more details.

           You should have received a copy of the GNU General Public License
           along with this program. If not, see <https://www.gnu.org/licenses/>.

 */
#pragma once

#include "Corba_Interfaces.h"

 /**
  * \brief Concept matching either a CORBA skeleton or a CORBA stub type
  * \tparam ty Any type
  * \details This concept matches if the given type satisfies either CORBASkeleton or CORBAStub concepts
  */
template <typename ty>
concept CORBASkeleton_OR_Stub = CORBASkeleton<ty> || CORBAStub<ty>;

/**
 * \brief Wrapper for CORBA stubs
 * \tparam ty A type satisfying CORBAStub
 * \details This struct wraps a CORBA stub type to distinguish it for filtering and composition
 */
template<CORBAStub ty> struct Stub { using type = ty; };

/**
 * \brief Wrapper for CORBA skeletons
 * \tparam ty A type satisfying CORBASkeleton
 * \details This struct wraps a CORBA skeleton type for role-based recognition
 */
template<CORBASkeleton ty> struct Skel { using type = ty; };

/**
 * \brief Concept ensuring the type is a Stub<X> where X is a CORBAStub
 * \tparam ty Type to validate
 * \details Requires the type to expose a member type and match the Stub template structure
 */
template<typename ty>
concept CORBAStubWrapper = requires {
   typename ty::type;
   requires std::same_as<ty, Stub<typename ty::type>>;
   requires CORBAStub<typename ty::type>;
};

/**
 * \brief Concept ensuring the type is a Skel<X> where X is a CORBASkeleton
 * \tparam ty Type to validate
 * \details Ensures that the wrapper is Skel<X> with X being a valid CORBASkeleton
 */
template<typename ty>
concept CORBASkeletonWrapper = requires {
   typename ty::type;
   requires std::same_as<ty, Skel<typename ty::type>>;
   requires CORBASkeleton<typename ty::type>;
};

/**
 * \brief Concept ensuring the type is either a CORBA stub or skeleton wrapper
 * \tparam ty Type to evaluate
 */
template<typename ty>
concept CORBAStubOrSkeletonWrapper = CORBAStubWrapper<ty> || CORBASkeletonWrapper<ty>;

/**
 * \brief A variadic typelist that only accepts CORBA stub or skeleton wrappers
 * \tparam Ts Types wrapped in Stub<> or Skel<>
 */
template<CORBAStubOrSkeletonWrapper... Ts>
struct TypeList {};

/**
 * \brief Trait to determine whether a type is a skeleton wrapper
 * \tparam ty Type to test
 * \details Default is false; specialized for Skel<X>
 */
template<typename ty>
struct is_skeleton : std::false_type {};

/**
 * \brief Specialization: returns true if type is Skel<X>
 */
template<CORBASkeleton ty>
struct is_skeleton<Skel<ty>> : std::true_type {};

/**
 * \brief Trait to determine whether a type is a stub wrapper
 * \tparam ty Type to test
 * \details Default is false; specialized for Stub<X>
 */
template<typename ty>
struct is_stub : std::false_type {};

/**
 * \brief Specialization: returns true if type is Stub<X>
 */
template<CORBAStub ty>
struct is_stub<Stub<ty>> : std::true_type {};

/**
 * \brief Filter metafunction: selects types from a typelist based on a predicate
 * \tparam List Typelist to filter
 * \tparam Pred Predicate metafunction
 */
template<typename List, template<typename> class Pred>
struct Filter;

/**
 * \brief Base case: filtering an empty list results in an empty list
 */
template<template<typename> class Pred>
struct Filter<TypeList<>, Pred> {
   using type = TypeList<>;
};

/**
 * \brief Recursive filtering of TypeList
 */
template<typename T, typename... Ts, template<typename> class Pred>
struct Filter<TypeList<T, Ts...>, Pred> {
private:
   using Tail = typename Filter<TypeList<Ts...>, Pred>::type;

public:
   using type = std::conditional_t < Pred<T>::value, decltype(TypeList<T>{} + Tail{}), Tail > ;
};

/**
 * \brief Concatenation of two TypeLists
 */
template<typename... A, typename... B>
constexpr TypeList<A..., B...> operator+(TypeList<A...>, TypeList<B...>) { return {}; }

/**
 * \brief Extracts the inner types from a TypeList and produces the corresponding CORBA base type
 */
template<typename List>
struct extract_inner;

/**
 * \brief Specialization: extract Skel<X...> -> CORBAServer<X...>
 */
template<typename... Ts>
struct extract_inner<TypeList<Skel<Ts>...>> {
   using type = CORBAServer<Ts...>;
};

/**
 * \brief Specialization: extract Stub<X...> -> CORBAClient<X...>
 */
template<typename... Ts>
struct extract_inner<TypeList<Stub<Ts>...>> {
   using type = CORBAClient<Ts...>;
};

/**
 * \brief Fallback-Spezialisierung für leere oder nicht passende Listen.
 */
template<typename... Ts>
struct extract_inner<TypeList<Ts...>> {
   using type = ORBBase;
};

/**
 * \brief Empty typelist fallback
 */
template<>
struct extract_inner<TypeList<>> {
   using type = ORBBase;
};

/**
 * \brief Computes the length of a TypeList
 */
template<typename List>
struct length;

template<typename... Ts>
struct length<TypeList<Ts...>> {
   static constexpr std::size_t value = sizeof...(Ts);
};

/**
 * \brief Composed CORBA client/server base class.
 * \tparam Roles Variadic list of role wrappers (Stub<T> or Skel<T>)
 * \details Based on the given wrapper roles, this class deduces whether to inherit from CORBAClient, CORBAServer, or both.
 */
template<CORBAStubOrSkeletonWrapper... Roles>
class CORBAClientServer : virtual public std::conditional_t<length<typename Filter<TypeList<Roles...>, is_skeleton>::type>::value != 0,
   typename extract_inner<typename Filter<TypeList<Roles...>, is_skeleton>::type>::type,
   ORBBase>,
   virtual public std::conditional_t<length<typename Filter<TypeList<Roles...>, is_stub>::type>::value != 0,
   typename extract_inner<typename Filter<TypeList<Roles...>, is_stub>::type>::type,
   ORBBase> {

   using SkeletonList = typename Filter<TypeList<Roles...>, is_skeleton>::type; ///< Filtered list of skeleton wrappers
   using StubList = typename Filter<TypeList<Roles...>, is_stub>::type;         ///< Filtered list of stub wrappers

   using ServerBase = typename extract_inner<SkeletonList>::type; ///< Resolved server base class
   using ClientBase = typename extract_inner<StubList>::type;     ///< Resolved client base class

   static constexpr std::size_t NumStubs = length<StubList>::value;           ///< Number of stub interfaces
   static constexpr std::size_t NumSkeletons = length<SkeletonList>::value;   ///< Number of skeleton interfaces

   /**
    * \brief Checks client stub count match.
    * \return True if ClientBase is ORBBase or matches declared count.
    */
   static constexpr bool _check_client_count() {
      if constexpr (std::is_same_v<ClientBase, ORBBase>) return true;
      else return NumStubs == ClientBase::NumStubs;
   }

   /**
    * \brief Checks server skeleton count match.
    * \return True if ServerBase is ORBBase or matches declared count.
    */
   static constexpr bool _check_server_count() {
      if constexpr (std::is_same_v<ServerBase, ORBBase>) return true;
      else return NumSkeletons == ServerBase::NumSkeletons;
   }

   static_assert(_check_client_count(), "Stub count mismatch");
   static_assert(_check_server_count(), "Skeleton count mismatch");

public:
   /**
    * \brief Constructor for server-only composition.
    * \param name ORB name
    * \param argc Argument count
    * \param argv Argument vector
    */
   template<typename = void>
      requires (NumStubs == 0 && NumSkeletons > 0)
   CORBAClientServer(const std::string& name, int argc, char* argv[])
      : ORBBase(name, argc, argv),
      ServerBase(name, argc, argv)
   {
   }

   /**
    * \brief Constructor for client-only composition.
    * \tparam Names Name string types
    * \param name ORB name
    * \param argc Argument count
    * \param argv Argument vector
    * \param names Stub instance names
    */
   template<typename... Names>
      requires (NumSkeletons == 0 && NumStubs > 0) &&
   (sizeof...(Names) == NumStubs) && (std::is_convertible_v<Names, std::string> && ...)
      CORBAClientServer(const std::string& name, int argc, char* argv[], Names&&... names)
      : ORBBase(name, argc, argv),
      ClientBase(name, argc, argv, std::forward<Names>(names)...)
   {
   }

   /**
    * \brief Constructor for client-server mixed mode.
    * \tparam Names Name string types
    * \param name ORB name
    * \param argc Argument count
    * \param argv Argument vector
    * \param names Stub instance names
    */
   template<typename... Names> requires (NumSkeletons > 0 && NumStubs > 0) &&
                                        (sizeof...(Names) == NumStubs) && (std::is_convertible_v<Names, std::string> && ...)
   CORBAClientServer(const std::string& name, int argc, char* argv[], Names&&... names) : ORBBase(name, argc, argv),
                            ServerBase(name, argc, argv), ClientBase(name, argc, argv, std::forward<Names>(names)...) {
      }

   /**
    * \brief Access the server base interface
    * \pre Requires at least one Skel<> role to be provided.
    * \return Reference to the server implementation
    */
   ServerBase& server() {
      static_assert(NumSkeletons > 0, "Attempted to access server() without skeleton roles");
      return *this;
      }

   /**
    * \brief Access the client base interface
    * \pre Requires at least one Stub<> role to be provided.
    * \return Reference to the client implementation
    */
   ClientBase& client() {
      static_assert(NumStubs > 0, "Attempted to access client() without stub roles");
      return *this;
      }

   /**
    * \brief Get a specific stub interface
    * \tparam T The stub type
    * \pre Must be instantiated with one or more Stub<> roles
    * \return Reference to the stub interface
    */
   template<typename T>
   auto& get_stub() {
      static_assert(NumStubs > 0, "get_stub() requires at least one stub");
      return this->client().template get<T>();
      }

   /**
    * \brief Get a specific skeleton interface
    * \tparam T The skeleton type
    * \pre Must be instantiated with one or more Skel<> roles
    * \return Reference to the skeleton interface
    */
   template<typename T>
   auto& get_skel() {
      static_assert(NumSkeletons > 0, "get_skel() requires at least one skeleton");
      return this->server().template get<T>();
      }
};
