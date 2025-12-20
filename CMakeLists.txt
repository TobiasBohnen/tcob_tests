add_executable(tcob_tests)

set(TEST_SRCFILES
  LuaDSLTests.cpp
  LuaScriptTests.cpp
  LuaWrapperTests.cpp
  AudioCodecTests.cpp
  AngleUnitTests.cpp
  AssetTests.cpp
  CameraTests.cpp
  ColorTests.cpp
  CSVTests.cpp
  ConfigTests.cpp
  ConfigIniTests.cpp
  ConfigJsonTests.cpp
  ConfigYamlTests.cpp
  ConfigXMLTests.cpp
  DisplayModeTests.cpp
  FontFamilyTests.cpp
  HelperTests.cpp
  ImageCodecTests.cpp
  ImageTests.cpp
  LSystemTests.cpp
  PointTests.cpp
  PropertyTests.cpp
  QuadtreeTests.cpp
  RandomTests.cpp
  RayTests.cpp
  RectTests.cpp
  SignalTests.cpp
  SizeTests.cpp
  SqliteTests.cpp
  StreamTests.cpp
  StyleTests.cpp
  TransformTests.cpp
  TweenTests.cpp
  UITests.cpp
  UTFTests.cpp
  main.cpp
)

if(NOT TCOB_IS_CI)
  list(APPEND TEST_SRCFILES
    FileSystemTests.cpp
  )
endif()

target_sources(tcob_tests PRIVATE ${TEST_SRCFILES})

target_link_libraries(tcob_tests PUBLIC ${TCOB_LIBS} PRIVATE tcob_extlibs) # link to static/object libraries to prevent CI hiccups

target_include_directories(tcob_tests PRIVATE . PRIVATE ../../tcob/include)

target_compile_options(tcob_tests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
  $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra -Wconversion -Wpedantic
  -Wno-sign-conversion
  >
  $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic>
)

target_link_options(tcob_tests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>: /ignore:4217,4286>
)

set_target_properties(tcob_tests PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED TRUE
)

include(doctest/doctest.cmake)
doctest_discover_tests(tcob_tests)

add_custom_target(tcob_tests_copyFiles ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testfiles ${CMAKE_CURRENT_BINARY_DIR}/testfiles
)

add_dependencies(tcob_tests_copyFiles tcob_tests)

if(TCOB_ASAN)
  target_compile_definitions(tcob_tests PUBLIC _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
  target_compile_options(tcob_tests PRIVATE -fsanitize=address,undefined)
  target_link_options(tcob_tests PRIVATE -fsanitize=address,undefined)
endif()