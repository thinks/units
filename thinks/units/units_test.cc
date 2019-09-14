// Copyright(C) Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#define _USE_MATH_DEFINES  // M_PI

#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <locale>
#include <sstream>
#include <system_error>

#include "thinks/units/units.h"

// Check compile-time constructs.
constexpr bool StaticTests() {
  using namespace thinks::unit_literals;

  // Construction.
  {
    // Explicit value type.
    static_assert(
        std::is_same_v<decltype(thinks::Millimeters<float>{1.23f}.value()),
                       float>,
        "");

    // Automatic value type from literal operator.
    static_assert(std::is_same_v<decltype((1.23_mm).value()), double>, "");
    static_assert(std::is_same_v<decltype((123_mm).value()), long long>, "");

    // Examples of invalid constructions.
    // 
    // Some operations are not possible if the arithmetic type
    // parameter is const.
    // auto x = thinks::Millimeters<const float>{1.23f};
    // x *= 2; // Invalid!
    //
    // References and pointers are not arithmetic types.
    // constexpr auto x = thinks::Millimeters<float&>{1.23f};
    //
    // Would require narrowing conversion.
    // constexpr auto x = thinks::Millimeters<std::uint16_t>{78312};
  }

  // unit_cast (allows casting value type and scale if same tag)
  {
    // cm -> mm
    static_assert(
        std::is_same_v<
            std::remove_const_t<decltype(
                thinks::unit_cast<thinks::Millimeters<double>>(1_cm))>,
            thinks::Millimeters<double>>,
        "");
    static_assert(1_cm == 10.0_mm, "");

    // Cast from literal operator value type to desired value type
    // (double -> float).
    static_assert(
        std::is_same_v<
            decltype(
                thinks::unit_cast<thinks::Millimeters<float>>(1.23_mm).value()),
            float>,
        "");

    // Cannot cast between different tags.
    // Doesn't compile, units have different tags (cm -> rad):
    // constexpr auto c = thinks::unit_cast<thinks::Radians<double>>(1_cm);
  }

  // Equality/inequality comparison.
  {
    static_assert(5.0_mm == 5_mm, "different value types");
    static_assert(50_mm == 5_cm, "different scale");
    static_assert(5.0_mm != 7_mm, "different value types");
    static_assert(41_mm != 4_cm, "different scale");

    // Not possible to compare units with different tags.
    // The following doesn't compile.
    // constexpr bool x = 5_cm == 5_deg;
  }

  // Arithmetic operations.
  {
    // Value type promotion follows the normal rules for built-in types.
    // Here: double + long long -> double
    static_assert(std::is_same_v<decltype((5.0_mm).value()), double>, "");
    static_assert(std::is_same_v<decltype((10_mm).value()), long long>, "");
    static_assert(std::is_same_v<decltype((5.0_mm + 10_mm).value()), double>,
                  "");

    // Unary addition (add-equals), supports different scales.
    // Return type is always same as lhs.
    static_assert((15_mm += 1_mm) == 16_mm && (15_mm += 1_mm) == 1.6_cm, "");
    static_assert((15_mm += 1_cm) == 25_mm && (15_mm += 1_cm) == 2.5_cm, "");
    // Doesn't compile, cannot add a scalar to a unit:
    // static_assert((10_mm += 1) == ???, "");

    // Unary subtraction (sub-equals), supports different scales.
    // Return type is always same as lhs.
    static_assert((15_mm -= 1_mm) == 14_mm && (15_mm -= 1_mm) == 1.4_cm, "");
    static_assert((15_mm -= 1_cm) == 5_mm && (15_mm -= 1_cm) == 0.5_cm, "");
    // Doesn't compile, cannot subtract a scalar from a unit:
    // static_assert((10_mm -= 1) == ???, "");

    // Unary multiplication by scalar.
    static_assert((1.4_cm *= 10) == 14.0_cm, "");

    // Binary addition.
    // Doesn't compile, units have different scale:
    // constexpr auto x = 5.0_cm + 10_mm;
    //
    // Need to manually cast to same scale:
    static_assert(2_cm + 3_cm == 5_cm, "");
    static_assert(
        2_cm + thinks::unit_cast<thinks::Centimeters<int>>(30_mm) == 5_cm, "");

    // Unary negation.
    static_assert(-thinks::Centimeters<double>{2.14} ==
                      thinks::Centimeters<double>{-2.14},
                  "");

    // Unary division.
    // Preserves dimensionality, simply divide the unit into "denom"
    // equal parts.
    static_assert((14_cm /= 7) == 2_cm, "");

    // Binary division.
    // Divide by unit, dimensionality is lost and we get a (unit-less) scalar.
    // The scalar type is determined by normal arithmetic type promotion.
    // Note that we support units of different scales since the return type
    // is not a unit.
    static_assert(14_cm / 7_cm == 2, "");
    static_assert(14_cm / 70_mm == 2, "");

    // Divide by scalar, dimensionality is preserved, result
    // is a unit (same as lhs, possibly different value type).
    static_assert(14_cm / 7.0 == 2.0_cm, "");

    // With C++17 this gives a warning due to the [[nodiscard]] attribute.
    // 12.3_mm + 3.2_mm;
  }

  return true; // If this function compiled we are good!

#if 0
  // Output stream operator.
  {
    std::cout.precision(15);

    const auto b = 3.73_cm;
    std::cout << thinks::unit_cast<thinks::Meters<double>>(b) << '\n';
    std::cout << thinks::unit_cast<thinks::Centimeters<double>>(b) << '\n';
    std::cout << thinks::unit_cast<thinks::Millimeters<double>>(b) << '\n';

    std::cout << thinks::Radians<double>{M_PI} << ", "
              << thinks::unit_cast<thinks::Radians<double>>(180.0_deg) << '\n';

    constexpr auto my_mm = 12.3_mm;
    constexpr auto my_cm = thinks::unit_cast<thinks::Centimeters<double>>(my_mm);
    constexpr auto my_m = thinks::unit_cast<thinks::Meters<double>>(my_mm);

    std::cout << my_mm << " is the same as " << my_cm << " or " << my_m << '\n';              
  }
  #endif
}

// For README.md.
bool Snippet0() {
  using namespace thinks::unit_literals;

  // Construction using literal, conversion using explicit casts.
  constexpr auto my_mm = 12.3_mm;
  constexpr auto my_cm = thinks::unit_cast<thinks::Centimeters<double>>(my_mm);
  constexpr auto my_m = thinks::unit_cast<thinks::Meters<double>>(my_mm);

  // Prints "12.3 [mm] is the same as 1.23 [cm] or 0.0123 [m]"
  std::ostringstream oss;
  oss << my_mm << " is the same as " << my_cm << " or " << my_m << '\n';
  return oss.str() != "12.3 [mm] is the same as 1.23 [cm] or 0.0123 [m]";
}

// For README.md.
bool Snippet1() {
  auto SomeFunctionThatReturnsCm = []() { return 42.3; };
  auto SomeFunctionThatTakesInMm = [](double offset) {};

  // Clearly wrong, but the compiler has no way of knowing since 
  // we are using the built-in type 'double' to represent both 
  // mm and cm quantities.
  const auto my_value = SomeFunctionThatReturnsCm();
  SomeFunctionThatTakesInMm(my_value);
  return true;
}

// For README.md.
bool Snippet2() {
  auto SomeFunctionThatReturnsCm = []() { return thinks::Centimeters<double>{42.3}; };
  auto SomeFunctionThatTakesInMm = [](thinks::Millimeters<double> offset) {};

  const auto my_value = SomeFunctionThatReturnsCm();
  
  // The following won't compile, there is no way to automatically 
  // convert mm to cm.
  //
  // SomeFunctionThatTakesInMm(my_value);
  // 
  // Force user to explicit convert the value to mm, which will automatically 
  // apply the required scaling.
  SomeFunctionThatTakesInMm(thinks::unit_cast<thinks::Millimeters<double>>(my_value));
  return true;
}

bool Snippet3() {
  using namespace thinks::unit_literals;

  auto SomeLegacyFunction = [](double offset_mm) {};

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
  // constexpr auto my_value = my_mm + my_cm;
  // SomeLegacyFunction(my_value.value());
  
  // Better to require explicit cast, the type of 'my_value' is clearly communicated.
  // We are adding [mm] quantities so we expect the result to also be in [mm].
  constexpr auto my_value = my_mm + thinks::unit_cast<decltype(my_mm)>(my_cm);
  SomeLegacyFunction(my_value.value());
  
  // Note that users can still make errors when dealing with raw values.
  //
  // Oops, [cm] passed as raw value to function that expects [mm].
  constexpr auto my_other_value = thinks::unit_cast<decltype(my_cm)>(my_mm) + my_cm;
  SomeLegacyFunction(my_other_value.value());
  return true;
}

void MainFunc() {
  std::cout << __cplusplus << '\n';

  auto success = true;
  success &= StaticTests();
  success &= Snippet0();
  success &= Snippet1();
  success &= Snippet2();
  success &= Snippet3();

  if (!success) {
    throw std::runtime_error("test failed");
  }
}

void OnFatalError(const std::exception& ex) {
  fprintf(stderr, "\n! %s\n", ex.what());
  fflush(stderr);  // It's here that failure may be discovered.
  if (ferror(stderr)) {
    throw ex;
  }
}

int main(int argc, char* argv[]) {
  // With g++ setlocale() isn't guaranteed called by the C++ level locale
  // handling. This call is necessary for e.g. wide streams.
  // "" is the user's natural locale.
  setlocale(LC_ALL, "");                 // C level global locale.
  std::locale::global(std::locale(""));  // C++ level global locale.
  try {
    MainFunc();  // The app's C++ level main function.
    return EXIT_SUCCESS;
  } catch (const std::system_error& ex) {
    // TODO(thinks): also retrieve and report error code.
    OnFatalError(ex);
  } catch (const std::exception& ex) {
    OnFatalError(ex);
  } catch (const int code) {
    std::ostringstream oss;
    oss << "Fatal error: " << code << "\n";
    OnFatalError(std::runtime_error(oss.str()));
    return code == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
  } catch (...) {
    OnFatalError(std::runtime_error("<unknown exception>"));
  }
  return EXIT_FAILURE;
}
