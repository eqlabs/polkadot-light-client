# Legend
[M] indicates that the rule is mandatory

[R] indicates that the rule is a recommendation

Any rule may be ignored if it defies common sense in a particular situation

This document is applicable to current repository and possible future **our** repositories in a scope of polkadot project. Any code in other existing repositories should be following guidelines of that project if such exist or just be consistent with surrounding code if doesn't.

## [M] Naming
```
Local variable:         my_variable
Class:                  MyClass
Interface class:        MyInterface (i.e. no special prefix or suffix)
Template parameter:     MyType, MyConstant, MyTemplate, T, TT, U, UU etc
Function:               doSomething
Private data member:    m_my_member
Public data member:     my_member
Constant:               my_constant
Member of enum:         MyEnumMember
Namespace:              my_namespace
File name:              my_header.h, my_cpp.cpp
Macro:                  PLC_MY_MACRO
```
 - Prefer self-descripting names
 - A variable name should clearly indicate its purpose, hiding unnecessary details
Don't use prefixes with our classes and namespaces to indicate the component that they come from. Use nested namespaces instead

## [R] Prefixes for function names
```
Setter:                setFoo
Getter:                getFoo
Bool getters:
State:                 isFoo (areFoo)
Possibility:           canFoo, mayFoo
Necessity:             needFoo, shouldFoo, mustFoo
Possession:            hasFoo, haveFoo
Existence:             fooExists, foosExist.
Bool setters:          setFoo
```
 - Function names should normally be based on a verb in imperative form. There is one exception. Since C++11 allows range-based for loops for user defined types, which have `begin()` and `end()` functions, then we allow such naming for these 2 particular cases.

## [R] Classes and structs
 - Use `struct` for simple types without private members or virtual methods, otherwise use `class`.

## [R] Scopes and accessibility
 - Always use the most narrow scope and lowest accessibility modifier:
 - If something can be made `constexpr` - mark it `constexpr`
 - If variable or method can be marked `const` - mark it `const`
 - If member can be marked as `private` - mark it `private`
 - Mark non-throwning methods as noexcept. This is extremely important for copy/move constructors and destructors
 - Reduce variable scope to a minimal possible. Remember that since C++ 17 you can define a variable in `if` or `switch` statement:
```
if (auto it = map.find(key); it != map.end()) {
    return it->second;
}
```

## [M] Use RAII for resource management
 - Don't leave your resources unmanaged

## [R] Definitions and declarations
 - Use .h extension for header files and .cpp for compilation unit files
 - Use `pragma once` in header files
 - Each .cpp file should have the corresponding .h file. Vice versa may be not true in case when it contains only declarations/inline or template methods. Corresponding .h and .cpp files should be placed in the same folder
 - Every definition that can be moved to .cpp file should be moved to .cpp files. Use headers files for template methods and small inline methods (for performance reasons)
 - In .cpp files prefer using anonymous namespaces for compilation unit-local entities.

## [R] Includes
 - Always try to use a minimal set of includes
 - Use forward declarations instead of includes whenever possible
 - Try to keep every header file self-contained (so it includes all the things it needs)
 - Include path to the header from the same component should start from the component root
 - Use `<>` for system and 3rdparty includes and `""` for project includes

## [R] Recommended includes order
 - In .cpp file the corresponding header file include comes first
 - After that the general rule is: more general includes come first, least general last:
    * System includes
    * Standard library includes
    * 3rdparty includes
    * Other project components includes
    * Current component includes

# [R] Namespaces
 - All the code within the project should be in `plc` root namespace (except specialization of entities in other libraries)
 - Use C++17 namespace format: ```namespace root::component::sub_component```
 - Add a corresponding comment for every closing namespace:
```
namespace foo::bar {
…
namespace detail {
…
} // namespace detail
…
namespace {
…
} // namespace
…
} // namespace foo::bar
```
 - Namespace part can be used as a part of type name to avoid duplication. E.g. if you  have a `crypto` namespace you may use `crypto::Algorithm` instead of `crypto::CryptoAlgorithm`
 - Do not put `using namespace xxx` in header files at a file scope.

## [R] Types and function signatures
 - Do not use owning raw pointers
 - If null is an invalid value prefer using references to pointers
 - Use std::optional for optional values
 - In function declarations `const` doesn’t bring any sense for parameters passed by value, so better avoid it. (However in function definitions it does make sense)
 - Prefer concepts and constraints for template parameters
 - Pass cheap to copy types by value (including `std::string_view` and `std::span`)
 - If an object takes ownership over the passed value or wants to have an owned copy inside, consider passing by rvalue reference instead of lvalue reference
 - When returning a local variable from a method don’t use `std::move` because it may prevent the compiler from doing copy elision

## [R] Braces
 - Use “egyptian” braces style everywhere:
```
namespace x {
struct Y {
    void z() {
        if (condition) {
            ...
        }
    }
};
} // namespace x
```
 - Add braces for every new scope even for one-line statements. E.g. use
```
if (something) {
    return 42;
}
```
instead of
```
if (something)
    return 42;
```

## [R] auto
 - Better use auto for variable types if possible. If code becomes less readable consider improve naming rather than specifying the type
 - However adding `const`, `&` and `*` to `auto` is usually helpful and is encouraged:
```
const auto* some_ptr = ...
auto& some_mutable_ref = ...
```

## [M] Inheritance
 - Mark classes `final` whenever possible
 - Mark overridden virtual methods with `override`. `virtual` should be omitted in this case

## [M] Error handling
 - Since most of our code is async we don’t use exceptions. To return an error use `plc::core::result` class which is an alias for `boost::outcome::result`, kind of `variant<Type, Error>`
 - Use asserts to check internal invariants. In all other cases return error.

## [R] Views
 - Prefer using `std::string_view` and `std/gsl::span` instead of `char*`/`std::string&`/`T*`/`T[]`/`const std::vector<T>&`
 - Ranges library may improve readability of the code

## [R] The recommended structure of a class declaration
```
class Class
{
    Private types, used in interfaces of non-private functions
    Friend declarations.
public:

    Public types and constants

    Public functions

    Implementation of interface SomeInterface1

protected:

    Protected types and constants

    Protected functions

    Implementation of interface SomeInterface2

private:

    Private types and constants

    Private functions

    Implementation of interface SomeInterface3

    Data
};
```
It's recommended to group similar entities in one of the following ways:
1. Separate groups with empty lines, optionally accompanying them with comments.
2. Separate groups with extra access specifiers with comments:
```
public:
 // types
   ...
public:
 // functions
   …
```
It's also recommended to group:
1. overridden virtual functions from a base class.
2. setters and getters (can be one separate group for each pair or accessors or two big all-setters and all-getters groups)
3. related data members

## [R] Macros
 - Avoid using macros if possible. If possible prefer `constexpr` variables and functions, metaprogramming and other language features
 - If using macros is unavoidable use `PLC_` prefix for all project macros

## [R] Coroutines usage
C++ coroutines are powerful and flexible mechanism for asynchronous programming however they also bring many places for potential bugs, so they should be used with care. Use-after-free errors are the most frequent ones. Here are some advices how to avoid them.

1. Test your code under address sanitizer
Run cmake with `-DSAN=ASAN`. Address sanitizer is very good at finding and explaining where the object is accessed and when it was freed in case of error in runtime

2. Be careful with passing parameters and accessing members in a "root" coroutine method
```
// Bad
cppcoro::task<void> Class::foo(Param& param) {
   param.bar() // At the time coroutine is executed param object may not be alive
   auto x = co_await some_async_method();
   m_member.set_x(x) // this pointer may not be valid at this point
}

```
  - Pass parameters to a root coroutine method by value or shared pointer if they don't have "global" lifetime.
  ```
  cppcoro::task<void> Class::foo(Param param)
  ```
  or
  ```
  cppcoro::task<void> Class::foo(std::shared_ptr<Param> param)
  ```
  - Save weak pointer to `this` or necessary members before the first suspension point:
  ```
  cppcoro::task<void> Class::foo(Param param) {
      auto log = m_log;
      auto x = co_await some_async_method();
      log->info("some message");
  }
  ```
  or
  ```
  cppcoro::task<void> Class::foo(Param param) {
      auto wp = weak_from_this(); // only valid if Class inherits from enable_shared_from_this
      auto x = co_await some_async_method();
      if (auto self = wp.lock()) {
          self->m_log->info("some message");
      }
  }
  ```
  This is applicable for "root" coroutines only (the ones that will be passed to `ClientRunner::dispatchTask` method). If one coroutine is co_awaiting another one it's safe to pass objects that are stored in the parent coroutine frame by reference because parent coroutine lifetime is not less than the called one:
  ```
  cppcoro::task<void> Class::foo() {
    Param param;
    // Passing param by reference here is safe
    auto res = co_await bar(param);
    ...
  }

  cppcoro::task<Result<void>> Class::bar(Param& param) {
    ...
  }
  ```
  The marker of the "root" coroutine is the task type (`cppcoro::task<void>`). In any other case a coroutine should have task type that may return an error (`cppcoro::task<Result<X>>`, where `X` may be `void`)

3. If you use lambda for a root coroutine do not capture anything:
```
// Bad
auto task = [foo]() -> cppcoro::task<void> {
    // some async code
    ...
}();

runner.dispatchTask(std::move(task));
```
Lambda object is an anonymous object which will be invalid at the time of coroutine execution. This effect is described in [this article](https://devblogs.microsoft.com/oldnewthing/20190116-00/?p=100715). The most simple workaround is to pass objects as parameters (by value or smart pointer as it was shown above):
```
// Good
auto task = [](Foo foo) -> cppcoro::task<void> {
    // some async code
    ...
}(std::move(foo));

runner.dispatchTask(std::move(task));
```
Some more advanced workarounds can be found [here](https://devblogs.microsoft.com/oldnewthing/20211103-00/?p=105870)

4. The quite similar situation is applicable to the lambdas the are passed to the `resumeInCallback` function. The difference is that you cannot capture anything **by value**:
```
    // Bad
    auto read_res = co_await resumeInCallback<MessageReadWriter::ReadCallback>(
        [reader_writer = m_read_writer](auto func) {
            reader_writer->read(std::move(func));
    });
```

Instead capture by references variables that are stored in the coroutine frame:
```
    // Good
    auto reader_writer = m_read_writer;
    auto read_res = co_await resumeInCallback<MessageReadWriter::ReadCallback>(
        [&reader_writer](auto func) {
            reader_writer->read(std::move(func));
    });
```