FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt GIT_TAG 6.2.1)
FetchContent_MakeAvailable(fmt)

list(APPEND DepIncludes  ${fmt_SOURCE_DIR}/include)
list(APPEND DepLibraries fmt)

set(DepIncludes  ${DepIncludes}  PARENT_SCOPE)
set(DepLibraries ${DepLibraries} PARENT_SCOPE)