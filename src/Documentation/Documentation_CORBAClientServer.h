// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \page CORBAClientServerDesign CORBAClientServer Design and Mechanism

  \section overview Overview

  The `CORBAClientServer` class enables the composition of CORBA server skeletons and client stubs 
  into a unified interface using template metaprogramming. It makes extensive use of modern C++20 features 
  such as concepts, variadic templates, type traits, and `std::conditional_t` to ensure correctness 
  entirely at compile time.

  This abstraction is crucial in middleware-heavy environments where runtime configuration is undesired or 
  impossible, and system correctness must be statically verifiable.

  \section motivation Motivation

  In standard C++ templates, it is not possible to distinguish two separate parameter packs (e.g., one 
  for stubs, one for skeletons) due to the greedy and ambiguous nature of parameter pack matching. This 
  restriction makes direct templating such as:

  ```cpp
  CORBAClientServer<CORBAStub<MyClient>, CORBASkeleton<MyServer>>;
  ```

  infeasible, since the compiler cannot unambiguously match which types belong to which role.

  Instead, the solution presented here introduces wrapper types `Stub<T>` and `Skel<T>`, combined with 
  compile-time filtering and inheritance to resolve role separation.

  This approach follows the advice from Andrei Alexandrescu’s talk 
  ["Corner of Hell"](https://www.youtube.com/watch?v=va9I2qivBOA) and circumvents the multiple pack 
  deduction problem entirely through explicit type wrapping.

  \section structure Structure

  The key components of the system are:

  - **Wrapper Tags**: `Stub<T>` and `Skel<T>` mark the role of each CORBA interface.
  - **Concepts**: Compile-time checks using `CORBAStubWrapper` and `CORBASkeletonWrapper` ensure valid roles.
  - **TypeList<>**: A simple recursive typelist pattern for variadic template handling.
  - **Filter**: A metafunction to extract only stub or skeleton roles using type traits (`is_stub`, `is_skeleton`).
  - **extract_inner**: Converts a filtered `TypeList<>` into the actual base class (`CORBAClient<>`, `CORBAServer<>`).
  - **conditional inheritance**: The final composed class inherits virtually from resolved base classes, or falls back to `ORBBase`.

  \section mechanics Mechanics and Compile-Time Guarantees

  - Role validation, class structure, and type matching are fully resolved at compile time.
  - Mismatched stub/skeleton counts result in `static_assert` errors.
  - All accessor methods (`server()`, `client()`, `get_stub<T>()`, `get_skel<T>()`) rely on `static_assert` to enforce their valid usage.

  \subsection example_typelist Example: Type Filtering

  The following type computation ensures the correct roles are extracted:

  ```cpp
  using SkeletonList = typename Filter<TypeList<Roles...>, is_skeleton>::type;
  using StubList = typename Filter<TypeList<Roles...>, is_stub>::type;
  ```

  These filtered lists are then transformed into actual base classes via:

  ```cpp
  using ServerBase = typename extract_inner<SkeletonList>::type;
  using ClientBase = typename extract_inner<StubList>::type;
  ```

  \subsection inheritance Inheritance Structure

  The final inheritance resolves to one of:
  - `CORBAServer<Ts...>`
  - `CORBAClient<Ts...>`
  - both
  - or `ORBBase` as fallback (empty configuration)

  Virtual inheritance is used to prevent ambiguous base class paths.

  \subsection constructors Constructors

  The class supports three construction modes:
  - **Skeleton-only** (no stubs): forwards to `CORBAServer` only
  - **Stub-only** (no skeletons): forwards to `CORBAClient` with service names
  - **Mixed mode**: initializes both bases

  Constructor selection uses C++20 constraints via `requires` clauses.

  \subsection accessor Accessors and Contracts

  Each accessor includes `static_assert` as a contract:

  ```cpp
  ServerBase& server();       // Only if at least one Skel<> is given
  ClientBase& client();       // Only if at least one Stub<> is given
  template<typename T> auto& get_stub();  // Requires stub list
  template<typename T> auto& get_skel();  // Requires skeleton list
  ```

  Any misuse results in a hard compile-time error, not runtime exceptions.

  \section benefits Benefits

  - Full compile-time structure enforcement.
  - Eliminates runtime errors due to misconfigured CORBA roles.
  - Uses standard C++ idioms with no need for external tools or frameworks.
  - Role separation is clear, maintainable, and extendable.

  \section example Usage Example

  ```cpp
  CORBAClientServer<
     Skel<MyServer_i>,
     Stub<MyClient>
  > system("AppName", argc, argv, "ClientService");

  auto& client = system.get_stub<MyClient>();
  auto& server = system.get_skel<MyServer_i>();
  ```

  \section summary Summary

  This design demonstrates how to harness modern C++ metaprogramming to build safe,
  composable, and explicit role-based middleware layers. All functionality is type-safe
  by design and forces correctness through template structure and compiler logic.
*/