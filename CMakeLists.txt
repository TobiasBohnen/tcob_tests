set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

Include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest
  GIT_TAG v2.4.11
)

set(DOCTEST_NO_INSTALL ON)

FetchContent_MakeAvailable(doctest)

mark_as_advanced(FORCE DOCTEST_NO_INSTALL DOCTEST_USE_STD_HEADERS DOCTEST_WITH_MAIN_IN_STATIC_LIB DOCTEST_WITH_TESTS)

add_executable(tcob_tests)

set(TEST_SRCFILES
  AngleUnitTests.cpp
  AssetTests.cpp
  CameraTests.cpp
  ColorTests.cpp
  CSVTests.cpp
  FontFamilyTests.cpp
  ConfigTests.cpp
  ConfigIniTests.cpp
  ConfigJsonTests.cpp
  ConfigYamlTests.cpp
  ConfigXMLTests.cpp
  HelperTests.cpp
  ImageCodecTests.cpp
  ImageTests.cpp
  LSystemTests.cpp
  PointTests.cpp
  PropertyTests.cpp
  QuadtreeTests.cpp
  RandomTests.cpp
  RayTests.cpp
  ResultTests.cpp
  RectTests.cpp
  SignalTests.cpp
  StreamTests.cpp
  StyleTests.cpp
  TransformTests.cpp
  TweenTests.cpp
  UITests.cpp
  UTFTests.cpp
  main.cpp
)

if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
  list(APPEND TEST_SRCFILES
    LuaDSLTests.cpp
    LuaScriptTests.cpp
    LuaWrapperTests.cpp
  )
endif()

if(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)
  list(APPEND TEST_SRCFILES
    SquirrelScriptTests.cpp
    SquirrelWrapperTests.cpp
  )
endif()

if(TCOB_ENABLE_ADDON_DATA_SQLITE)
  list(APPEND TEST_SRCFILES
    SqliteTests.cpp
  )
endif()

if(NOT TCOB_IS_CI)
  list(APPEND TEST_SRCFILES
    FileSystemTests.cpp
  )
endif()

target_sources(tcob_tests PRIVATE ${TEST_SRCFILES})

target_link_libraries(tcob_tests PRIVATE doctest::doctest PUBLIC ${TCOB_LIBS} PRIVATE tcob_extlibs) # link to static/object libraries to prevent CI hiccups

target_include_directories(tcob_tests PRIVATE . PRIVATE ../../tcob/include)

target_compile_options(tcob_tests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
  $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra -Wno-switch-default -Wno-unsafe-buffer-usage -Wno-c++20-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-float-equal -Wno-old-style-cast -Wno-header-hygiene -Wno-ctad-maybe-unsupported -Wno-exit-time-destructors -Wno-global-constructors -Wno-double-promotion>
  $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic>
)
target_link_options(tcob_tests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>: /ignore:4217,4286>
)

set_target_properties(tcob_tests PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
)

list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake/)
include(doctest)
doctest_discover_tests(tcob_tests)

add_custom_target(tcob_tests_copyFiles ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testfiles ${CMAKE_CURRENT_BINARY_DIR}/testfiles
)

add_dependencies(tcob_tests_copyFiles tcob_tests)

# if(TCOB_ASAN)
# target_compile_definitions(tcob_tests PUBLIC _DISABLE_STRING_ANNOTATION PUBLIC _DISABLE_VECTOR_ANNOTATION)
# target_compile_options(tcob_tests PRIVATE -fsanitize=undefined -fsanitize=address)
# target_link_directories(tcob_tests PUBLIC "C:/Program Files/LLVM/lib/clang/17/lib/windows/")
# target_link_options(tcob_tests
# PUBLIC clang_rt.asan_dynamic-x86_64.lib
# PUBLIC clang_rt.asan_dynamic_runtime_thunk-x86_64.lib
# )
# endif()
