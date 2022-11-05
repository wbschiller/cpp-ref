CppRef
======

CppRef is a collection of C++ reference examples as well as utilities.  This work relies
on C++v17 or later features to implement Design Patterns following a Functional Programming
approach.

## Build
Build and run the unit tests (tested on Ubuntu):
```
cmake -B build
cd build
make
ctest -V
```
Build and run the unit tests as using clang
```
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B build
cd build
make
ctest -V
```

## Design
### State Machine Design Pattern
A State Machine template is provided by [src/util/state/StateMachine.hpp](src/util/state/StateMachine.hpp).  This Design Pattern allows decoupling of states, events, and transitions.  It also allows for testing the states and transtions independently.  Tests are
provided for this State Machine using googletest / gmock.

Before using this in your application, consider the type of state machine needed. 
There are two common categories of state machines:
1. Sequential State Machines
2. Finite State Machines

The Sequential State Machine is more common and is actually a series of steps.  It is usually implemented as a state machine due to the asynchronous completion of the steps.  An alternate design approach is to use a Future object to capture completion of steps, then the sequence of steps can be implemented more cleanly with an Active object in a Worker queue.  But in cases where there are too many concurrent sequences and threads become too expensive, the Sequential State Machine has to fall back to a pattern like this State Machine.

In the second category of Finite State Machine, the states and transitons occur repeatedly (consider a transmission shifting between gears).  This design pattern is a nice fit for the Finite State Machine.

As a related note, some languages like Rust, provide a preferred approach for handling asynchronous operation.  For instance, the Rust Tokio Crate provides a runtime that supports async operations and allows for tasks that look to be blocking but are actually receiving async feedback.  The result is that much of the complexity falls out.