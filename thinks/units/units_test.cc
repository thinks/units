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

#include <functional>

#include "thinks/units/units.h"

void OnFatalError(const std::exception& ex) {
  fprintf(stderr, "\n! %s\n", ex.what());
  fflush(stderr);  // It's here that failure may be discovered.
  if (ferror(stderr)) {
    throw ex;
  }
}

// Unit instances can interact with each other if they have the same
// tag (category), e.g. length, angle. Scaling is automatically handled by the
// types, so that cm's can interact with mm's. However, this interaction is not 
// always automatically handled. By design, we require an explicit cast to add 
// mm's to cm's. The rationale for this is that it is not obvious what unit 
// the result should be given in. By forcing the user to cast the values into 
// the same scale we are defering the choice to the user.

void main_func() {
  using namespace units::literals;

  // Construction.
  {
    // Explicit value type.
    constexpr auto a = units::Millimeters<float>{1.23f};

    // Automatic value type from literal operator.
    constexpr auto b = 1.23_mm;
    static_assert(std::is_same_v<decltype(b.value()), double>, "");
  }

  // unit_cast (also allows casting value type)
  {
    // cm -> mm
    constexpr auto a = units::unit_cast<units::Millimeters<double>>(1_cm);

    static_assert(std::is_same_v<std::remove_const_t<decltype(a)>,
                                 units::Millimeters<double>>, "");
    static_assert(a == 10.0_mm, "scaled value");

    // Cast from literal operator value type to desired value type 
    // (double -> float).
    constexpr auto b = units::unit_cast<units::Millimeters<float>>(1.23_mm);
    static_assert(std::is_same_v<decltype(b.value()), float>, "");

    // Cannot cast between different tags.
    // Doesn't compile, units have different tags (cm -> rad):
    // constexpr auto c = units::unit_cast<units::Radians<double>>(1_cm);
  }

  // Equality/inequality comparison.
  {
    static_assert(5.0_mm == 5_mm, "different value types");
    static_assert(50_mm == 5_cm, "different scale");
    static_assert(5.0_mm != 7_mm, "different value types");
    static_assert(41_mm != 4_cm, "different scale");
  }

  // Arithmetic operations.
  {
    // Value type promotion follows the normal rules for built-in types.
    // Here: double + long long -> double
    constexpr auto a = 5.0_mm;
    constexpr auto b = 10_mm;
    constexpr auto c = a + b;
    static_assert(std::is_same_v<decltype(a.value()), double>, "");
    static_assert(std::is_same_v<decltype(b.value()), long long>, "");
    static_assert(std::is_same_v<decltype(c.value()), double>, "value type promotion");

    // Unary addition (add-equals), supports different scales.
    // Return type is always same as lhs.
    static_assert((15_mm += 1_mm) == 16_mm &&
                  (15_mm += 1_mm) == 1.6_cm, "");
    static_assert((15_mm += 1_cm) == 25_mm && 
                  (15_mm += 1_cm) == 2.5_cm, "");
    // Doesn't compile, cannot add a scalar to a unit:
    // static_assert((10_mm += 1) == ???, "");

    // Unary subtraction (sub-equals), supports different scales.
    // Return type is always same as lhs.
    static_assert((15_mm -= 1_mm) == 14_mm &&
                  (15_mm -= 1_mm) == 1.4_cm, "");
    static_assert((15_mm -= 1_cm) == 5_mm && 
                  (15_mm -= 1_cm) == 0.5_cm, "");
    // Doesn't compile, cannot subtract a scalar from a unit:
    // static_assert((10_mm -= 1) == ???, "");

    // Unary multiplication by scalar.
    static_assert((1.4_cm *= 10) == 14.0_cm, "");

    // Binary addition.
    constexpr auto d = 5.0_cm;
    constexpr auto e = 10_mm;
    
    // Doesn't compile, units have different scale:  
    // constexpr auto f = d + e;
    //
    // Need to manually cast to same scale:
    constexpr auto f = d + units::unit_cast<decltype(d)>(e);
    static_assert(std::is_same_v<decltype(f), decltype(d)>, "");

    // Unary negation.
    constexpr auto g = 14.2_deg;
    constexpr auto h = -g;
    static_assert(h.value() == -g.value(), "");
    static_assert(-units::Centimeters<double>{2.14} == units::Centimeters<double>{-2.14}, "");
      
    // Unary division.
    // Preserves dimensionality, simply divide the unit in "denom"
    // equal parts.
    constexpr auto i = 14_cm /= 7;
    static_assert(i == 2_cm, "");

    // Binary division.
    // Divide by unit, dimensionality is lost and we get a (unit-less) scalar.
    // The scalar type is determined by normal arithmetic type promotion.
    // Note that we support units of different scales since the return type
    // is not a unit.
    constexpr auto j = 14_cm / 7_cm;
    static_assert(j == 2, "");
    constexpr auto k = 14_cm / 70_mm;
    static_assert(k == 2, "");

    // Divide by scalar, dimensionality is preserved, result
    // is a unit (same as lhs, possibly different value type).
    constexpr auto m = 14_cm / 7.0;
    static_assert(m == 2.0_cm, "");
  }

  // Output stream operator.
  {
    std::cout.precision(15);

    const auto b = 3.73_cm;
    std::cout << units::unit_cast<units::Meters<double>>(b) << '\n';
    std::cout << units::unit_cast<units::Centimeters<double>>(b) << '\n';
    std::cout << units::unit_cast<units::Millimeters<double>>(b) << '\n';

    std::cout << units::Radians<double>{M_PI} << ", "
              << units::unit_cast<units::Radians<double>>(180.0_deg) << '\n';

    constexpr auto my_mm = 12.3_mm;
    constexpr auto my_cm = units::unit_cast<units::Centimeters<double>>(my_mm);
    constexpr auto my_m = units::unit_cast<units::Meters<double>>(my_mm);

    std::cout << my_mm << " is the same as " << my_cm << " or " << my_m << '\n';              
  }
}

int main(int argc, char* argv[]) {
  // With g++ setlocale() isn't guaranteed called by the C++ level locale
  // handling. This call is necessary for e.g. wide streams.
  // "" is the user's natural locale.
  setlocale(LC_ALL, "");                 // C level global locale.
  std::locale::global(std::locale(""));  // C++ level global locale.
  try {
    main_func();  // The app's C++ level main function.
    return EXIT_SUCCESS;
  } catch (const std::system_error& ex) {
    // TODO: also retrieve and report error code.
    OnFatalError(ex);
  } catch (std::exception const& ex) {
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
