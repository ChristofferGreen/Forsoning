add_executable(ForsoningTest
  main.cpp
  cpp_coroutine.cpp
)

target_include_directories(ForsoningTest 
  PUBLIC
    ../src
)

target_include_directories(ForsoningTest 
  SYSTEM
  PUBLIC
    ${DepIncludesPublic}
  PRIVATE
    ${DepIncludes}
)

target_compile_definitions(ForsoningTest
  PUBLIC
    ${DepDefines}
)

target_link_libraries(ForsoningTest
  PUBLIC
    ${DepLibraries}
)

enable_testing()
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(ForsoningTest)