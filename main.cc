
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <locale>
#include <system_error>



void on_fatal_error(const std::exception& ex) {
  fprintf(stderr, "\n! %s\n", ex.what());
  fflush(stderr);  // It's here that failure may be discovered.
  if (ferror(stderr)) {
    throw ex;
  }
}

void main_func() {}

int main(int argc, char* argv[]) {
  // With g++ setlocale() isn't guaranteed called by the C++ level locale
  // handling. This call is necessary for e.g. wide streams. "" is the user's
  // natural locale.
  setlocale(LC_ALL, "");                 // C level global locale.
  std::locale::global(std::locale(""));  // C++ level global locale.
  try {
    main_func();  // The app's C++ level main function.
    return EXIT_SUCCESS;
  } catch (const std::system_error& x) {
    // TODO: also retrieve and report error code.
    on_fatal_error(x);
  } catch (std::exception const& x) {
    on_fatal_error(x);
  } catch (const int code) {
    on_fatal_error(std::runtime_error("Fatal error ($e::Exit_code)"));
    return code == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
  } catch (...) {
    on_fatal_error(std::runtime_error("<unknown exception>"));
  }
  return EXIT_FAILURE;
}
