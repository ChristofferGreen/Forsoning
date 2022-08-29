add_executable(ForsoningTest
  main.cpp
  catch.cpp
  cpp_concepts.cpp
  cpp_coroutine.cpp
  cpp_json.cpp
  cpp_asio.cpp
  cpp_random_string.cpp
  multithreaded_problems.cpp
  forge.cpp
  forge_upgradable_mutex.cpp
  path.cpp
  path_space.cpp
  path_space_insert.cpp
  path_space_grab.cpp
  path_space_grab_block.cpp
  path_space_read.cpp
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
    CATCH_AMALGAMATED_CUSTOM_MAIN
)

target_link_libraries(ForsoningTest
  PUBLIC
    ${DepLibraries}
)

#enable_testing()
#include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
#doctest_discover_tests(ForsoningTest)