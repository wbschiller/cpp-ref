
#include "util/state/StateMachine.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::Return;
using ::testing::StrictMock;

using namespace wbs::util::state;

namespace
{

struct Event1
{
};
struct Event2
{
};
struct Event3
{
};
struct Event4
{
};
using Event = std::variant<Event1, Event2, Event3, Event4>;

class MockState
{
public:
  MOCK_METHOD(void, LogTransition, (const char *previous));
  MOCK_METHOD(const char *, GetName, ());
  MOCK_METHOD(void, Enter, ());
  MOCK_METHOD(void, Exit, ());
};

struct BaseState
{
  MockState *mock = nullptr;
  void LogTransition(const char *previous)
  {
    mock->LogTransition(previous);
  }
  const char *GetName()
  {
    return mock->GetName();
  }
  void Enter()
  {
    mock->Enter();
  }
  void Exit()
  {
    mock->Exit();
  }
};

struct State1 : public BaseState
{
};

class State2 : public BaseState
{
};
using State = std::variant<State1, State2>;

struct Transitions
{
  Context<Event> &ctx;
  StrictMock<MockState> ms1;
  StrictMock<MockState> ms2;
  using StateVar = std::optional<State>;
  State GetInitState()
  {
    return State1{&ms1};
  }
  StateVar operator()(State1 &, const Event2 &)
  {
    return State2{&ms2};
  }
  StateVar operator()(State2 &, const Event1 &)
  {
    return State1{&ms1};
  }
  StateVar operator()(State1 &, const Event4 &)
  {
    ctx.Dispatch(Event2{});  // legal - Dispatches event without transition
    return std::nullopt;
  }
  StateVar operator()(State2 &, const Event4 &)
  {
    ctx.Dispatch(Event2{});  // illegal - Dispatches and transitions;
    return State1{&ms1};
  }
  template <typename StateAny, typename EventAny> StateVar operator()(StateAny &, const EventAny &) const
  {
    return std::nullopt;
  }
};

class StateMachineTest : public ::testing::Test
{
protected:
  Transitions mTransitions{mStateMachine};
  StateMachine<State, Event, Transitions> mStateMachine;

public:
  StateMachineTest() : mStateMachine(mTransitions)
  {
  }
  ~StateMachineTest() override = default;
};

TEST_F(StateMachineTest, dispatch_does_not_change_state_on_ignored_event)
{
  // in State1 - ignore Event1 and Event3
  mStateMachine.Dispatch(Event1{});
  mStateMachine.Dispatch(Event3{});
}

TEST_F(StateMachineTest, dispatch_transitions_to_expected_state)
{
  EXPECT_CALL(mTransitions.ms1, GetName());
  EXPECT_CALL(mTransitions.ms2, LogTransition(_));
  {
    InSequence seq;
    EXPECT_CALL(mTransitions.ms1, Exit());
    EXPECT_CALL(mTransitions.ms2, Enter());
  }
  mStateMachine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, multiple_dispatch_events_handled_in_order)
{
  // Configure State2 Enter() method to dispatch an event to transition to State1
  ON_CALL(mTransitions.ms2, Enter()).WillByDefault([this]() { mStateMachine.Dispatch(Event1{}); });
  EXPECT_CALL(mTransitions.ms1, GetName());
  EXPECT_CALL(mTransitions.ms2, GetName());
  EXPECT_CALL(mTransitions.ms1, LogTransition(_));
  EXPECT_CALL(mTransitions.ms2, LogTransition(_));
  {
    InSequence seq;
    EXPECT_CALL(mTransitions.ms1, Exit());
    EXPECT_CALL(mTransitions.ms2, Enter());
    EXPECT_CALL(mTransitions.ms2, Exit());
    EXPECT_CALL(mTransitions.ms1, Enter());
  }
  mStateMachine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, dispatch_ignores_events_in_exit)
{
  // Configure State1 Exit() method to dispatch an event - should be ignored - no transition
  ON_CALL(mTransitions.ms1, Exit()).WillByDefault([this]() { mStateMachine.Dispatch(Event1{}); });
  EXPECT_CALL(mTransitions.ms1, GetName());
  EXPECT_CALL(mTransitions.ms2, LogTransition(_));
  EXPECT_CALL(mTransitions.ms1, Exit());
  EXPECT_CALL(mTransitions.ms2, Enter());
  mStateMachine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, dispatch_ignores_visit_events_when_transitioning)
{
  // Event4 on State1 dispatches Event2, which transitions State1 -> State2
  EXPECT_CALL(mTransitions.ms1, GetName());
  EXPECT_CALL(mTransitions.ms2, LogTransition(_));
  EXPECT_CALL(mTransitions.ms1, Exit());
  EXPECT_CALL(mTransitions.ms2, Enter());
  mStateMachine.Dispatch(Event4{});

  // Event4 on State2 dispatches Event2, and returns State1.
  // This is a confusing attempt to toggle the state and is dissallowed
  EXPECT_CALL(mTransitions.ms2, GetName());
  EXPECT_CALL(mTransitions.ms1, LogTransition(_));
  EXPECT_CALL(mTransitions.ms2, Exit());
  EXPECT_CALL(mTransitions.ms1, Enter());
  mStateMachine.Dispatch(Event4{});
}



} // namespace
