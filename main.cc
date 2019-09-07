#define _USE_MATH_DEFINES 

#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <locale>
#include <sstream>
#include <system_error>

#include "units.h"



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
  using namespace units;

  // Construction.
  {
    // Explicit value type.
    constexpr auto a = units::Millimeters<float>{1.23f};

    // Automatic value type from literal operator.
    constexpr auto b = 1.23_mm;
    static_assert(std::is_same_v<decltype(b.value()), double>, "");
  }

  // scale_cast (also allows casting value type)
  {
    // cm -> mm
    constexpr auto a = units::scale_cast<units::Millimeters<double>>(1_cm);

    static_assert(std::is_same_v<std::remove_const_t<decltype(a)>,
                                 units::Millimeters<double>>, "");
    static_assert(a == 10.0_mm, "scaled value");

    // Cast from literal operator value type to desired value type 
    // (double -> float).
    constexpr auto b = units::scale_cast<units::Millimeters<float>>(1.23_mm);
    static_assert(std::is_same_v<decltype(b.value()), float>, "");

    // Cannot cast between different tags.
    // Doesn't compile, units have different tags (cm -> rad):
    // constexpr auto c = units::scale_cast<units::Radians<double>>(1_cm);
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

    constexpr auto d = 5.0_cm;
    constexpr auto e = 10_mm;
    
    // Doesn't compile, units have different scale:  
    // constexpr auto f = d + e;
    //
    // Need to manually cast to same scale:
    constexpr auto f = d + units::scale_cast<decltype(d)>(e);
  }

  // Output stream operator.
  {
    std::cout.precision(15);

    const auto b = 3.73_cm;
    std::cout << units::scale_cast<units::Meters<double>>(b) << "\n";
    std::cout << units::scale_cast<units::Centimeters<double>>(b) << "\n";
    std::cout << units::scale_cast<units::Millimeters<double>>(b) << "\n";

    std::cout << units::Radians<double>{M_PI} << ", "
              << units::scale_cast<units::Radians<double>>(180.0_deg) << "\n";
  }

  // test negation, uint -> int
  // +=
  // -=
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
