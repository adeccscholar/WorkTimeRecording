/**
\file 
\brief Introduction and Key Concepts of Finite State Machines (FSM) and Boost.Statechart

\page finite_state_machine_page Finite State Machines and Boost.Statechart

Finite State Machines (FSM) are fundamental mathematical models for describing the dynamic behavior of discrete systems. 
FSMs are widely used in engineering, computer science, and related fields for system modeling, control flows, and protocol implementation.

\section fsm_introduction Introduction

A finite state machine is a computational abstraction that consists of a finite set of states, a set of input events, and a set of transitions that describe how the system moves from one state to another depending on incoming events. 
FSMs provide a precise and understandable way to capture and analyze complex behavior in both software and hardware systems.

For a comprehensive discussion on FSMs, refer to the article:
- "Finite-State Machine" – Wikipedia: https://en.wikipedia.org/wiki/Finite-state_machine

\section fsm_math_model Mathematical Model

Mathematically, a finite state machine is defined as a 5-tuple:
\verbatim
M = (Q, Σ, δ, q0, F)
\endverbatim

Where:
- \b Q is a finite set of states
- \b Σ is a finite input alphabet (set of possible input symbols)
- \b δ is the state transition function (δ: Q × Σ → Q)
- \b q0 ∈ Q is the initial state
- \b F ⊆ Q is the set of accepting or final states

The machine starts in the initial state q0 and, for each input symbol, applies the transition function δ to move from one state to another. Computation continues until all input symbols are consumed, at which point the current state determines acceptance (for acceptors) or defines the system configuration (for controllers).

\section fsm_concepts Core Concepts

- \b State: A specific condition or situation in which the system can exist.
- \b Event/Input: A stimulus or symbol that can trigger a state transition.
- \b Transition: The rule describing movement from one state to another, based on a given input.
- \b Initial State: The state in which the FSM starts its operation.
- \b Final State: A (possibly empty) set of states that signify successful processing (for acceptor-type FSMs).
- \b Action: An operation executed during entry, exit, or transition.

\section fsm_variants FSM Variants

- Deterministic FSM (DFA): For each state and input, exactly one transition is defined.
- Non-deterministic FSM (NFA): Multiple transitions may be possible for a state/input pair.
- Mealy Machine: Outputs depend on state and input (actions on transitions).
- Moore Machine: Outputs depend only on the state (actions on entry).

\section fsm_advanced Advanced Features

Modern FSM frameworks support:
- \b Hierarchical States: States can themselves contain nested state machines.
- \b Orthogonal Regions: Multiple independent state machines within one system (concurrency).
- \b Entry/Exit Actions: Code executed on entering or leaving a state.

\section boost_statechart Boost.Statechart in C++

Boost.Statechart is a modern, type-safe C++ library for implementing even complex state machines, closely following the UML state machine semantics. 
It allows you to define states, events, transitions, actions, and hierarchical compositions in a structured and maintainable way.

\code
// Example: Door State Machine
struct EvOpen : boost::statechart::event<EvOpen> {};
struct EvClose : boost::statechart::event<EvClose> {};

struct StOpen;
struct StClosed;

struct DoorStateMachine : boost::statechart::state_machine<DoorStateMachine, StClosed> {};

struct StClosed : boost::statechart::state<StClosed, DoorStateMachine> {
   typedef boost::mpl::list<
      boost::statechart::transition<EvOpen, StOpen>
   > reactions;
};

struct StOpen : boost::statechart::state<StOpen, DoorStateMachine> {
   typedef boost::mpl::list<
      boost::statechart::transition<EvClose, StClosed>
   > reactions;
};
\endcode

\section fsm_summary Summary

Finite state machines offer a compact and rigorous way to model dynamic system behavior. 
The Boost.Statechart library in C++ provides a powerful tool to implement these models in practice, supporting advanced concepts such as hierarchy, orthogonality, and entry/exit actions.

\author Volker Hillmann
*/
