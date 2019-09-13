# Units
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This repository contains a single-file, header-only, C++ library for performing type safe operations on values representing units. A simple example illustrates these ideas.
```cpp
#include <iostream>
#include "thinks/units.h"

int main(int argc, char* argv[]) {
  using namespace units::literals;

  // Construction using literal.
  constexpr auto my_mm = 12.3_mm;
  constexpr auto my_cm = units::unit_cast<units::Centimeters<double>>(mm);

  std::cout << my_mm " is the same as " << my_cm << '\n';

  return 0;
}

```

## Pitfalls


C++14

 type safe units
