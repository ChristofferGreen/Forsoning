add_executable(ForsoningTest
  main.cpp
  catch.cpp

  path_space/cpp_concepts.cpp
  path_space/cpp_coroutine.cpp
  path_space/cpp_json.cpp
  path_space/cpp_asio.cpp
  path_space/cpp_random_string.cpp
  path_space/multithreaded_problems.cpp
  path_space/forge.cpp
  path_space/path.cpp
  path_space/path_space.cpp
  path_space/path_space_insert.cpp
  path_space/path_space_grab.cpp
  path_space/path_space_grab_block.cpp
  path_space/path_space_read.cpp

  renderer/basic_vulkan.cpp
)

target_include_directories(ForsoningTest 
  PUBLIC
    ../src
    ../ext
)

target_include_directories(ForsoningTest 
  SYSTEM
  PUBLIC
    ${GLFW_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
  PRIVATE
    ${DepIncludes}
)

target_compile_definitions(ForsoningTest
  PUBLIC
    CATCH_AMALGAMATED_CUSTOM_MAIN
)

target_link_libraries(ForsoningTest
  PUBLIC
    glfw
    Vulkan::Vulkan
)

