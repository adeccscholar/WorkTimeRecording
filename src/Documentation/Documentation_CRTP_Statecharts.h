// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Doxygen Documentation Page for the CRTP Pattern and the boost::statechart

 \details Informations about boost::statechart and the metaprogramming pattern CRTP
          which is used for boost::statechart
 
 
 \author Volker Hillmann (adecc Systemhaus GmbH)
 
 \date 2025-07-05 first implementation for statechart
 
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.

*/

/**
 \page statechart_crtp Curiously Recurring Template Pattern (CRTP) as utilized in statechart frameworks
 
 \brief Demonstrates the Curiously Recurring Template Pattern (CRTP) as utilized in statechart frameworks.

\details
Statecharts are an advanced formalism for modeling finite state machines (FSM) that extend classical
state machines with hierarchical states, orthogonal regions, and event-driven transitions. This approach,
introduced by David Harel in the 1980s, enables complex system behavior to be described in a structured
and modular fashion.

\c boost::statechart is a powerful C++ library for implementing hierarchical state machines based on
the statechart formalism. It provides a declarative way to express states, transitions, and actions,
allowing for modular design and compile-time validation of state machine correctness. A key feature
of \c boost::statechart is its extensive use of the Curiously Recurring Template Pattern (CRTP).
CRTP enables the base state and event classes in \c boost::statechart to access type-specific functionality
in derived classes, resolve polymorphic behavior at compile time, and avoid runtime overhead
associated with traditional virtual functions.

CRTP is employed in multiple places within \c boost::statechart:
- **State Definitions**: User-defined state classes inherit from library-provided templates (e.g., \c state<MyState, MyMachine>).
- **Event Definitions**: Custom event classes inherit from \c boost::statechart::event<EventType>.
- **State Machine Root**: The root of the state machine inherits from \c boost::statechart::state_machine<Machine, InitialState>.

\details Advantages of this approach:
- Ensures that state and event types are fully known at compile time, enabling static type checking and improved safety.
- Maximizes performance by eliminating virtual dispatch and allowing inlining and other compiler optimizations.
- Facilitates modular design, as new states and events can be defined independently and composed hierarchically.

\see
 - https://www.boost.org/doc/libs/release/libs/statechart/doc/index.html
 - https://en.wikipedia.org/wiki/State_machine
 - https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern

\tparam Derived The derived type that provides custom implementation details.

\warning
This example only demonstrates the structural CRTP concept and does not provide a full statechart
framework. For industrial-strength hierarchical state machines, refer to \c boost::statechart.

\author Volker Hillmann

\code{.cpp}
template<typename Derived>
class StateBase
{
public:
   /// \brief Handles an event, forwarding to the derived class implementation.
   /// \param event The event to be handled.
   void HandleEvent(int const& event)
   {
      // Forward event handling to the derived class (static polymorphism).
      static_cast<Derived*>(this)->OnEvent(event);
   }
};

class MyState final : public StateBase<MyState>
{
public:
   /// \brief Implements event handling logic specific to this state.
   /// \param event The event identifier.
   void OnEvent(int const& event)
   {
      // Simulate state-specific event handling.
      std::cout << "MyState handles event: " << event << std::endl;
   }
};
\endcode

\code{.cpp}
#include <iostream>

int main()
{
   MyState theState;
   theState.HandleEvent(42); // Output: MyState handles event: 42
   return 0;
}
\endcode

 */