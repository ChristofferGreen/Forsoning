include(FetchContent)

set_property(GLOBAL PROPERTY BUILD_SHARED_LIBS False)
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS True)

add_subdirectory("doctest"    EXCLUDE_FROM_ALL) # Testing
add_subdirectory("fmt"        EXCLUDE_FROM_ALL) # Text formating

set(DepIncludes       ${DepIncludes}       PARENT_SCOPE) # Include paths
set(DepIncludesPublic ${DepIncludesPublic} PARENT_SCOPE) # Public include paths
set(DepLibraries      ${DepLibraries}      PARENT_SCOPE) # Libraries to link against
set(DepDefines        ${DepDefines}        PARENT_SCOPE) # Defines to use