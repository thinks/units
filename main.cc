
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <exception>
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

  auto j = 5.0_mm + 10_mm;
  static_assert(std::is_same_v<decltype(j.value), double>, "");

  // auto j = 5.0_cm + 10.0_mm; // Doesn't compile!
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
