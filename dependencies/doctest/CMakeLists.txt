FetchContent_Declare(doctest GIT_REPOSITORY https://github.com/onqtam/doctest GIT_TAG 2.4.7)
FetchContent_MakeAvailable(doctest)

list(APPEND DepIncludesPublic ${doctest_SOURCE_DIR}/doctest)
list(APPEND DepLibraries      doctest)

set(DepLibraries      ${DepLibraries}      PARENT_SCOPE)
set(DepIncludesPublic ${DepIncludesPublic} PARENT_SCOPE)