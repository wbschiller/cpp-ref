
#include "util/state/StateMachine.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_; // NOLINT(bugprone-reserved-identifier)
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::Return;
using ::testing::StrictMock;

using namespace wbs::util::state;

namespace {

struct Event1 {};
struct Event2 {};
struct Event3 {};
struct Event4 {};
using Event = std::variant<Event1, Event2, Event3, Event4>;

class MockState {
public:
  MOCK_METHOD(void, LogTransition, (const char *previous));
  // NOLINTNEXTLINE(modernize-use-trailing-return-type)
  MOCK_METHOD(const char *, GetName, ());
  MOCK_METHOD(void, Enter, ());
  MOCK_METHOD(void, Exit, ());
};

struct BaseState {
  MockState *mock = nullptr;
  void LogTransition(const char *previous) const
  {
    mock->LogTransition(previous);
  }
  [[nodiscard]] auto GetName() const -> const char * { return mock->GetName(); }
  void Enter() const { mock->Enter(); }
  void Exit() const { mock->Exit(); }
};

struct State1 : public BaseState {};

class State2 : public BaseState {};
using State = std::variant<State1, State2>;

struct Transitions {
  Context<Event> &ctx;
  StrictMock<MockState> ms1;
  StrictMock<MockState> ms2;
  using StateVar = std::optional<State>;
  auto GetInitState() -> State { return State1{&ms1}; }
  auto operator()(State1 & /*unused*/, const Event2 & /*unused*/) -> StateVar
  {
    return State2{&ms2};
  }
  auto operator()(State2 & /*unused*/, const Event1 & /*unused*/) -> StateVar
  {
    return State1{&ms1};
  }
  auto operator()(State1 & /*unused*/, const Event4 & /*unused*/) -> StateVar
  {
    ctx.Dispatch(Event2{}); // legal - Dispatches event without transition
    return std::nullopt;
  }
  auto operator()(State2 & /*unused*/, const Event4 & /*unused*/) -> StateVar
  {
    ctx.Dispatch(Event2{}); // illegal - Dispatches and transitions;
    return State1{&ms1};
  }
  template <typename StateAny, typename EventAny>
  auto operator()(StateAny & /*unused*/, const EventAny & /*unused*/) const
      -> StateVar
  {
    return std::nullopt;
  }
};

class StateMachineTest : public ::testing::Test {
public:
  StateMachineTest() : m_state_machine(m_transitions) {}
  ~StateMachineTest() override = default;

  StateMachineTest(StateMachineTest const &other) = delete;
  auto operator=(StateMachineTest const &other) -> StateMachineTest & = delete;
  StateMachineTest(StateMachineTest &&other) noexcept = delete;
  auto operator=(StateMachineTest &&other) noexcept
      -> StateMachineTest & = delete;

  void SetUp() override {}
  void TearDown() override {}

protected:
  // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
  Transitions m_transitions{m_state_machine};
  StateMachine<::State, Event, Transitions> m_state_machine;
  // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
};

TEST_F(StateMachineTest, dispatch_does_not_change_state_on_ignored_event)
{
  // in State1 - ignore Event1 and Event3
  m_state_machine.Dispatch(Event1{});
  m_state_machine.Dispatch(Event3{});
}

TEST_F(StateMachineTest, dispatch_transitions_to_expected_state)
{
  EXPECT_CALL(m_transitions.ms1, GetName());
  EXPECT_CALL(m_transitions.ms2, LogTransition(_));
  {
    InSequence seq;
    EXPECT_CALL(m_transitions.ms1, Exit());
    EXPECT_CALL(m_transitions.ms2, Enter());
  }
  m_state_machine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, multiple_dispatch_events_handled_in_order)
{
  // Configure State2 Enter() method to dispatch an event to transition to
  // State1
  ON_CALL(m_transitions.ms2, Enter()).WillByDefault([this]() {
    m_state_machine.Dispatch(Event1{});
  });
  EXPECT_CALL(m_transitions.ms1, GetName());
  EXPECT_CALL(m_transitions.ms2, GetName());
  EXPECT_CALL(m_transitions.ms1, LogTransition(_));
  EXPECT_CALL(m_transitions.ms2, LogTransition(_));
  {
    InSequence seq;
    EXPECT_CALL(m_transitions.ms1, Exit());
    EXPECT_CALL(m_transitions.ms2, Enter());
    EXPECT_CALL(m_transitions.ms2, Exit());
    EXPECT_CALL(m_transitions.ms1, Enter());
  }
  m_state_machine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, dispatch_ignores_events_in_exit)
{
  // Configure State1 Exit() method to dispatch an event - should be ignored -
  // no transition
  ON_CALL(m_transitions.ms1, Exit()).WillByDefault([this]() {
    m_state_machine.Dispatch(Event1{});
  });
  EXPECT_CALL(m_transitions.ms1, GetName());
  EXPECT_CALL(m_transitions.ms2, LogTransition(_));
  EXPECT_CALL(m_transitions.ms1, Exit());
  EXPECT_CALL(m_transitions.ms2, Enter());
  m_state_machine.Dispatch(Event2{});
}

TEST_F(StateMachineTest, dispatch_ignores_visit_events_when_transitioning)
{
  // Event4 on State1 dispatches Event2, which transitions State1 -> State2
  EXPECT_CALL(m_transitions.ms1, GetName());
  EXPECT_CALL(m_transitions.ms2, LogTransition(_));
  EXPECT_CALL(m_transitions.ms1, Exit());
  EXPECT_CALL(m_transitions.ms2, Enter());
  m_state_machine.Dispatch(Event4{});

  // Event4 on State2 dispatches Event2, and returns State1.
  // This is a confusing attempt to toggle the state and is dissallowed
  EXPECT_CALL(m_transitions.ms2, GetName());
  EXPECT_CALL(m_transitions.ms1, LogTransition(_));
  EXPECT_CALL(m_transitions.ms2, Exit());
  EXPECT_CALL(m_transitions.ms1, Enter());
  m_state_machine.Dispatch(Event4{});
}

} // namespace
