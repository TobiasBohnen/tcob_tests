if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Fuzzing requires clang")
endif()

function(add_tcob_fuzzer TARGET SOURCE)
    add_executable(${TARGET} ${SOURCE})
    target_link_libraries(${TARGET}
        PRIVATE
        ${TCOB_LIBS}
        tcob_extlibs
    )
    target_include_directories(${TARGET}
        PRIVATE
        .
        ../../tcob/include
    )
    target_compile_features(${TARGET} PRIVATE cxx_std_23)
    target_compile_options(${TARGET} PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Wpedantic
        -Wno-sign-conversion
        -fsanitize=fuzzer,address
        -fno-omit-frame-pointer
        -fno-lto
        -g
        -O1
    )
    target_link_options(${TARGET} PRIVATE
        -fsanitize=fuzzer,address
        -fno-lto
    )
    target_compile_definitions(${TARGET} PRIVATE
        _DISABLE_STRING_ANNOTATION
        _DISABLE_VECTOR_ANNOTATION
        _DISABLE_OPTIONAL_ANNOTATION
    )
endfunction()

add_tcob_fuzzer(tcob_fuzz_gif fuzz_gif.cpp)
add_tcob_fuzzer(tcob_fuzz_pcx fuzz_pcx.cpp)
add_tcob_fuzzer(tcob_fuzz_pnm fuzz_pnm.cpp)
add_tcob_fuzzer(tcob_fuzz_tga fuzz_tga.cpp)
add_tcob_fuzzer(tcob_fuzz_png fuzz_png.cpp)
add_tcob_fuzzer(tcob_fuzz_qoi fuzz_qoi.cpp)
add_tcob_fuzzer(tcob_fuzz_bmp fuzz_bmp.cpp)