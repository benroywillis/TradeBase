cmake_minimum_required(VERSION 3.10)

# For some reason cmake/ninja checks whether the system headers have changed, causing the entire project to rebuild each time
# This patch seems to fix it, ripped from https://stackoverflow.com/questions/47087237/cmake-and-ninja-rebuild-unnecessary-files
if (CMAKE_GENERATOR MATCHES "Ninja")
    file(WRITE "${CMAKE_BINARY_DIR}/CLANGMakeRulesOverwrite.cmake" 
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_C \"\${CMAKE_DEPFILE_FLAGS_C}\")\n"
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_CXX \"\${CMAKE_DEPFILE_FLAGS_CXX}\")\n"
    )
    set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_BINARY_DIR}/CLANGMakeRulesOverwrite.cmake" CACHE INTERNAL "")
endif ()

project(TradeBase VERSION 0.0.1)
set (CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
# debug mode
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
    message(STATUS "Setting libCXX debug flag")
endif()

set(Platform_Inc "${CMAKE_CURRENT_SOURCE_DIR}/Platform/inc/")
set(Indicators_Inc "${CMAKE_CURRENT_SOURCE_DIR}/Alpha/Indicators/inc/")
set(Strategies_Inc "${CMAKE_CURRENT_SOURCE_DIR}/Alpha/Strategies/inc/")

find_package(spdlog CONFIG REQUIRED)

option(ENABLE_LINTER "Run linter" OFF)
option(ENABLE_LINTER_ERROR "Run linter with errors" OFF)
if(${ENABLE_LINTER})
    find_program(TIDY NAMES clang-tidy-10 clang-tidy)
    if(${TIDY} STREQUAL "TIDY-NOTFOUND")
        message(FATAL_ERROR "clang-tidy not found! Necessary for linting!")
    else()
        message(STATUS "Found clang-tidy in ${TIDY}")
    endif()
    if(${ENABLE_LINTER_ERROR})
        set(CMAKE_CXX_CLANG_TIDY 
            ${TIDY};
            --warnings-as-errors=*;
            --checks=readability-*,-readability-magic-numbers,performance-*,portability-*,modernize-*,-modernize-use-trailing-return-type,-cppcoreguidelines-*;
        )
        add_compile_options(-Werror)
    else()
        set(CMAKE_CXX_CLANG_TIDY 
            ${TIDY};
            --checks=readability-*,-readability-magic-numbers,performance-*,portability-*,modernize-*,-modernize-use-trailing-return-type,-cppcoreguidelines-*;
        )
    endif()
endif()

add_subdirectory("Alpha")
add_subdirectory("Backtesting")
add_subdirectory("Platform")

set(CLANG_FORMAT_EXCLUDE_PATTERNS "build" "vcpkg")
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp *.h)
foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    foreach (EXCLUDE_PATTERN ${CLANG_FORMAT_EXCLUDE_PATTERNS})
        string(FIND ${SOURCE_FILE} ${EXCLUDE_PATTERN} PROJECT_TRDPARTY_DIR_FOUND)
        if (NOT ${PROJECT_TRDPARTY_DIR_FOUND} EQUAL -1)
            list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
        endif ()
    endforeach()
endforeach ()

find_program(FORMAT NAMES clang-format-10 clang-format)
if(${FORMAT} STREQUAL "FORMAT-NOTFOUND")
message(FATAL_ERROR "clang-format not found! Necessary for linting!")
else()
message(STATUS "Found clang-format in ${FORMAT}")
endif()
add_custom_target(
    format
    COMMAND ${FORMAT}
    -style=file
    -i
    ${ALL_SOURCE_FILES}
)