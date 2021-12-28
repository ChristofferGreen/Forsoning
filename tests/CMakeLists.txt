add_executable(ForsoningTest
  main.cpp
)

enable_testing()
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(ForsoningTest)