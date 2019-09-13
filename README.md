# Units
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This repository contains a single-file, header-only, no-dependencies C++ library for performing type-safe operations on values representing units. A simple example illustrates these ideas.
```cpp
#include <iostream>
#include "thinks/units/units.h"

int main(int argc, char* argv[]) {
  using namespace units::literals;

  // Construction using literal, conversion using explicit casts.
  constexpr auto my_mm = 12.3_mm;
  constexpr auto my_cm = units::unit_cast<units::Centimeters<double>>(mm);
  constexpr auto my_m = units::unit_cast<units::Meters<double>>(mm);

  // Prints "12.3 [mm] is the same as 1.23 [cm] or 0.0123 [m]"
  std::cout << my_mm " is the same as " << my_cm << " or " << my_m << '\n';

  return 0;
}
```
The key idea is that a value representing a millimeter quantity has a different type than a value representing a centimeter quantity. Converting between millimeters and centimeters is of course trivial, but requires an explicit cast. This can be used to create type-safe interfaces, avoiding reliance on conventions or other hard to enforce constructs. Consider the following weakly enforced code:
```cpp
// Real function names probably less informative...
double SomeFunctionThatReturnsCm() { ... }
void SomeFunctionThatTakesInMm(double offset) { ... }

void Foo() {
  // Clearly wrong, but the compiler has no way of knowing since 
  // we are using the built-in type 'double' to represent both 
  // mm and cm quantities.
  const auto my_value = SomeFunctionThatReturnsCm();
  SomeFunctionThatTakesInMm(my_value);
}
```
Consider the following alternative:
```cpp
#include "thinks/units/units.h"

units::Centimeters<double> SomeFunctionThatReturnsCm() { ... }
void SomeFunctionThatTakesInMm(units::Millimeters<double> offset) { ... }

void Foo() {
  const auto my_value = SomeFunctionThatReturnsCm();
  
  // The following won't compile, there is no way to automatically 
  // convert mm to cm.
  //
  // SomeFunctionThatTakesInMm(my_value);
  // 
  // Force user to explicit convert the value to mm, which will automatically 
  // apply the required scaling.
  SomeFunctionThatTakesInMm(units::unit_cast<units::Millimeters<double>>(my_value));
}
```
Above

compile-time



## Pitfalls

## Tests
constexpr

C++14

 type safe units
