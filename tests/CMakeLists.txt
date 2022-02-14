add_executable(ForsoningTest
  main.cpp
  path.cpp
  path_space_insert.cpp
  cpp_coroutine.cpp
  cpp_json.cpp
  cpp_asio.cpp
)

target_include_directories(ForsoningTest 
  PUBLIC
    ../src
    ../ext
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