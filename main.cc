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

void main_func() {
  using namespace units;

  auto k = 5.0_mm + 10.0_mm;

  // Value type promotion.
  auto j = 5.0_mm + 10_mm;
  static_assert(std::is_same_v<decltype(j.value()), double>, "value type promotion");

  // Equality/inequality comparison.
  static_assert(5.0_mm == 5_mm, "equality comparison with different value types");
  static_assert(50_mm == 5_cm, "equality comparison with scale conversion");
  static_assert(5.0_mm != 7_mm, "inequality comparison with different value types");

  // Doesn't compile, units have different scale.
  // auto j = 5.0_cm + 10.0_mm; 
  constexpr auto jj = 5.0_cm + units::scale_cast<units::Centimeters<double>>(10.0_mm);
  static_assert(jj == 6.0_cm, "");

  constexpr auto i = units::scale_cast<units::Millimeters<double>>(1_cm);
  
  static_assert(std::is_same_v<decltype(i), const units::Millimeters<double>>, "bad cast");
  static_assert(i == 10.0_mm, "bad scaled value");

  // Doesn't compile, units have different tags.
  // constexpr auto n = units::scale_cast<units::Radians<double>>(1_cm);

  constexpr auto deg = 180.0_deg;
  constexpr auto rad = 3.14159265359_rad; // pi
  //static_assert(units::Radians<double>{M_PI} == deg, "deg/rad comparison"); // precision error
  std::cout.precision(15);
  std::cout << deg.value() << "\n";
  std::cout << rad.value() << "\n";
  std::cout << units::Radians<double>{M_PI}.value() << ", " << scale_cast<units::Radians<double>>(deg).value() << "\n";
  std::cout << scale_cast<Degrees<double>>(rad).value() << "\n";


  auto m = 10.0_mm;
  m += 3.0_mm;

  const auto b = 3.73_cm;
  std::cout << units::scale_cast<units::Meters<double>>(b) << "\n";
  std::cout << units::scale_cast<units::Centimeters<double>>(b) << "\n";
  std::cout << units::scale_cast<units::Millimeters<double>>(b) << "\n";

  const auto a = 180.0_deg;
  std::cout << units::scale_cast<units::Degrees<double>>(a) << "\n";
  std::cout << units::scale_cast<units::Radians<double>>(a) << "\n";

  // test negation, uint -> int
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
