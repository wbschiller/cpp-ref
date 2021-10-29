#pragma once

#include <deque>
#include <optional>
#include <variant>

namespace wbs::util::state
{

/**
 * The Interface for the State Machine.  The Context is designed to be passed
 * to states for Dispatching events into the StateMachine.
 * @tparam EventVariant a variant holding the Events for the State Machine.
 */
template <typename EventVariant> class Context
{
public:
  virtual ~Context() = default;

  /**
   * Dispatch an event to the current state.
   * @note This call can result in the current state being deleted.  Do not access current
   * state memory after calling this method.
   * @note This method is not thread-safe. Only call this method from one thread.
   * @param evt a variant holding and Event for the State Machine.
   */
  virtual void Dispatch(const EventVariant &evt) = 0;
};

/**
 * This is a functional approach to the State Machine Design Pattern.  The design is
 * borrowed from http://www.vishalchovatiya.com/state-design-pattern-in-modern-cpp/
 * and is extended for our application.
 *
 * At a high-level, the purpose of a State Machine is to switch between States.  Each
 * State handles Events.  The handling of Events may lead to Transitions.  The purpose
 * of this Design Pattern is to decouple States, Events, and Transitions.  For instance,
 * it is preferred to remove knowledge of next/previous States from each individual
 * State.  This allows adding/removing States with minimal code change and leads to a
 * simpler implementation.
 *
 * This State Machine Design Pattern relies on C++17 features to implement a functional
 * programming approach.  Instead of using an enum or inheritance for the Events, the
 * Events are defined as structs and placed in a variant.  Likewise, the States are all
 * defined as a struct and placed in a variant.  With the Events and States in two
 * different variants, the visitor pattern (using std::visit) is used to define a
 * Visitor Transition table.  The Transition table then returns an std::optional to
 * allow returning the next state or null for no transition.
 *
 * The State Machine implements the Context interface.  This interface is passed to the
 * States to allow Dispatch() of events when needed.
 *
 * The State held in the StateVariant must provide four methods to support calls from
 * the State Machine, similar to this struct:
 * @code
 *   struct State {
 *     void Enter() { }
 *     void Exit() { }
 *     void LogTransition(const char *) { }
 *     const char *GetName() { return ""; }
 *   }
 * @endcode
 * @note Only the State::Enter() method is allowed to Dispatch events.  All other calls to
 * Dispatch State methods are ignored.
 *
 * The Transitions table type is implemented as a visitor. This type has overloaded
 * callable operator methods to match the combinations of State / Event variants that
 * are interesting for the transition.  This allows the Transition table to define how
 * a State responds to an Event.  Below is an example of a Transition implemented as a struct:
 * @code
 *   struct Transitions {
 *     StateVariant operator()(State1 &, const Event2 &) { return State2{}; }
 *     StateVariant operator()(State2 &, const Event1&) { return State1{}; }
 *   }
 * @endcode
 * The methods above are invoked through a call to std::visit.
 * @note The Transition table is allowed to Dispatch events.  However, Dispatch events will
 * be ignored if the visit method transitions to a new state.
 *
 * @tparam StateVariant a variant holding the States.
 * @tparam EventVariant a variant holding the Events.
 * @tparam Transitions a visitor that holds the transition table.
 */
template <typename StateVariant, typename EventVariant, typename Transitions>
class StateMachine : public Context<EventVariant>
{
public:
  StateMachine(Transitions &tr) : mCurrentState(tr.GetInitState()), mTransitions(tr)
  {
  }
  ~StateMachine() override = default;
  void Dispatch(const EventVariant &evt) override
  {
    auto inProcess = !evts.empty();
    evts.push_back(evt);
    if (!inProcess)
    {
      HandleEvents();
    }
  }

private:
  void HandleEvents()
  {
    while (!evts.empty())
    {
      auto count = evts.size();
      auto newState = std::visit(mTransitions, mCurrentState, evts.front());
      if (newState)
      {
        LogTransition(*newState, GetName(mCurrentState));
        Exit(mCurrentState);
        mCurrentState = std::move(*newState);
        while (evts.size() > count)
        {
          evts.pop_back(); // illegal events discarded per design
        }
        Enter(mCurrentState);
      }
      evts.pop_front();
    }
  }
  void Enter(StateVariant &state)
  {
    auto fcn = [](auto &st) { st.Enter(); };
    std::visit(fcn, state);
  }
  void Exit(StateVariant &state)
  {
    auto fcn = [](auto &st) { st.Exit(); };
    std::visit(fcn, state);
  }
  const char *GetName(StateVariant &state)
  {
    auto fcn = [](auto &st) { return st.GetName(); };
    return std::visit(fcn, state);
  }
  void LogTransition(StateVariant &state, const char *previous)
  {
    auto fcn = [&previous](auto &st) { st.LogTransition(previous); };
    std::visit(fcn, state);
  }

  StateVariant mCurrentState;
  Transitions &mTransitions;
  std::deque<EventVariant> evts{};
};

} // namespace wbs::util::state
