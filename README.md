# Units
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This repository contains a single-file, header-only, no-dependencies, mostly compile-time C++ library for performing type-safe operations on values representing units. There are plenty of cases where misinterpretating the meaning of a quantity can have catastrophic consequences. Some classic example include `centimeters`/`millimeters` and `degrees`/`radians`. Even the clever people at [NASA](https://www.latimes.com/archives/la-xpm-1999-oct-01-mn-17288-story.html) seem to struggle with such issues from time to time. A simple example illustrates the basic idea of using types to represent units.
```cpp
#include <iostream>
#include "thinks/units/units.h"

void Foo() {
  using namespace thinks::unit_literals;

  // Construction using literal, conversions using explicit casts.
  constexpr auto my_mm = 12.3_mm;
  constexpr auto my_cm = thinks::unit_cast<thinks::Centimeters<double>>(my_mm);
  constexpr auto my_m = thinks::unit_cast<thinks::Meters<double>>(my_mm);

  static_assert(my_mm == my_cm && my_mm == my_m, "equal lengths");

  // Prints "12.3 [mm] is the same as 1.23 [cm] or 0.0123 [m]"
  std::cout << my_mm << " is the same as " << my_cm << " or " << my_m << '\n';
}
```
The key idea is that a value representing a millimeter quantity has a different type than a value representing a centimeter quantity. Converting between millimeters and centimeters is of course trivial, but requires an explicit cast. This can be used to create type-safe interfaces, avoiding reliance on conventions or other hard to enforce constructs. 

## Motivation
Consider the following code, where units are weakly enforced using function names:
```cpp
// Real function names probably less informative...
double SomeFunctionThatReturnsCm() { ... }
void SomeFunctionThatTakesInMm(double offset) { ... }

void Foo() {
  // Clearly wrong, but the compiler has no way of knowing since 
  // we are using the built-in type 'double' to represent both 
  // [mm] and [cm] quantities.
  const auto my_value = SomeFunctionThatReturnsCm();
  SomeFunctionThatTakesInMm(my_value);
}
```
Above, the compiler has no way of detecting the error of passing centimeters when millimeters are expected. Now, consider the following alternative:
```cpp
#include "thinks/units/units.h"

thinks::Centimeters<double> SomeFunctionThatReturnsCm() { ... }
void SomeFunctionThatTakesInMm(thinks::Millimeters<double> offset) { ... }

void Foo() {
  const auto my_value = SomeFunctionThatReturnsCm();
  
  // The following won't compile, there is no way to automatically 
  // convert [mm] to [cm].
  //
  // SomeFunctionThatTakesInMm(my_value);
  // 
  // Force user to explicit convert the value to [mm], which will automatically 
  // apply the required scaling.
  SomeFunctionThatTakesInMm(thinks::unit_cast<units::Millimeters<double>>(my_value));
}
```
In the snippet above, interfaces have been changed so that units are communicated explicitly using types (as opposed to function names). This makes things simple and clear, at the expense of being slightly more verbose. 

## Pitfalls
Inevitably, we are forced to interact with code we cannot change, be it in-house legacy code or third party libraries. In such cases we have no choice but to expose the raw built-in types across function boundaries. These scenarios have informed the design of this library, where we attempt to avoid confusion at the expense of introducing a little extra verbosity. The snipped below shows an example of such a scenario:
```cpp
#include "thinks/units/units.h"

void SomeLegacyFunction(double offset_mm) { ... }

void Foo() {
  using namespace thinks::unit_literals;

  constexpr auto my_mm = 12.3_mm;
  constexpr auto my_cm = 2.5_cm;
  
  // When calling legacy functions we might be forced to pass in 
  // the raw (untyped) unit value. In the code below (that doesn't compile) 
  // it is unclear what the type of 'my_value' is. It would be straight-forward
  // to implement a binary addition operator for [mm] and [cm] that returns the
  // result as either [mm] or [cm], but it would be difficult to prevent users
  // from making incorrect assumptions regarding the returned type. For this 
  // reason many binary operators require the two units to be the same (only allowing
  // value types to differ).
  //
  // constexpr auto my_value = my_mm + my_cm; // Unclear if my_value is [mm] or [cm].
  // SomeLegacyFunction(my_value.value());    // Are we passing in [mm] or [cm]?
  
  // Better to require explicit cast, the type of 'my_value' is clearly communicated.
  // We are adding [mm] quantities so we expect the result to also be in [mm].
  constexpr auto my_value = my_mm + thinks::unit_cast<decltype(my_mm)>(my_cm);
  SomeLegacyFunction(my_value.value());
}
```
The actual culprit above is the keyword `auto`, which essentially hides the type of `my_value`. Since `auto` sees widespread use (for good reasons), we have choosen to require that the operands of operators that return a newly constructed unit have the same tag (e.g. length) and scale. In cases where operators return booleans or scalars, this constraint is relaxed, such that units with the same tag, regardless of scale, can be used as operands, as shown below. Note that when value types differ normal arithmetic promotion rules apply.
```cpp
#include "thinks/units/units.h"

using namespace thinks::unit_literals;

// Binary division.
// Divide by unit, dimensionality is lost and we get a (unit-less) scalar.
// The scalar type is determined by normal arithmetic type promotion.
// Note that units with different scales are supported since the return type
// is not a unit.
static_assert(14_cm / 7_cm == 2, "");
static_assert(14_cm / 70_mm == 2, "");

// Divide by scalar, dimensionality is preserved, the result
// is a unit with the same as scale and tag as lhs, but with a promoted value type:
// long long / double -> double
static_assert(14_cm / 7.0 == 2.0_cm, "");
static_assert(std::is_same_v<decltype((14_cm).value()), long long>, "");
static_assert(std::is_same_v<decltype(7.0), double>, "");
static_assert(std::is_same_v<decltype((14_cm / 7.0).value()), double>, "");
```

## Compile-time
With modern C++ it is possible to 

behave like built-in types, value type promotion

changes base unit to get best precision, cm in our case






## Tests
constexpr

C++14

 type safe units
