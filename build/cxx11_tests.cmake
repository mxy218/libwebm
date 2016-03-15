if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAVE_CXX11)
  if (HAVE_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  endif ()
endif ()

# C++11 compile tests.
# TODO(tomfinegan): Move the C++11 tests into a cmake include.
if (MSVC OR HAVE_CXX11)
  # std::unique_ptr
  check_cxx_source_compiles("
      #include <memory>
      int main(int argc, const char* argv[]) {
        std::unique_ptr<int> ptr;
        (void)ptr;
        return 0;
      }"
      HAVE_UNIQUE_PTR)

  # default member values
  check_cxx_source_compiles("
      struct Foo {
        int a = 0;
      };
      int main(int argc, const char* argv[]) {
        Foo bar;
        (void)bar;
        return 0;
      }"
      HAVE_DEFAULT_MEMBER_VALUES)

  # defaulted methods
  check_cxx_source_compiles("
      struct Foo {
        Foo() = default;
        ~Foo() = default;
      };
      int main(int argc, const char* argv[]) {
        Foo bar;
        (void)bar;
        return 0;
      }"
      HAVE_DEFAULTED_MEMBER_FUNCTIONS)

  # deleted methods
  check_cxx_source_compiles("
      struct Foo {
        Foo() {}
        Foo(const Foo&) = delete;
      };
      int main(int argc, const char* argv[]) {
        Foo bar;
        (void)bar;
        return 0;
      }"
      HAVE_DELETED_MEMBER_FUNCTIONS)
endif ()

if (NOT HAVE_UNIQUE_PTR
    OR NOT HAVE_DEFAULT_MEMBER_VALUES
    OR NOT HAVE_DEFAULTED_MEMBER_FUNCTIONS
    OR NOT HAVE_DELETED_MEMBER_FUNCTIONS)
  set(ENABLE_TESTS OFF)
  set(ENABLE_WEBMTS OFF)
  message(WARNING "C++11 feature(s) not supported, tests and webmts disabled.")
endif ()

