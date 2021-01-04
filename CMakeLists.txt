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

project(TradeBase VERSION 0.1)
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
set(Python_Inc "/usr/include/python3.8")

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

option(ENABLE_TESTING "Build test suite" ON)
if(${ENABLE_TESTING})
    enable_testing()
    add_subdirectory("Testing")
endif()
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

# install directories can be user defined
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries.")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries.")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation diretory for header files.")
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files.")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}")  # to find foo/config.h

export(TARGETS TradeBase FILE "${PROJECT_BINARY_DIR}/TradeBaseConfig.cmake")
export(PACKAGE TradeBase)

# Create the TradeBaseConfig.cmake and TradeBaseConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(TradeBaseConfig.cmake.in "${PROJECT_BINARY_DIR}/TradeBaseConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${TRADEBASE_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(TradeBaseConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TradeBaseConfig.cmake" @ONLY)
# ... for both
configure_file(TradeBaseConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/TradeBaseConfigVersion.cmake" @ONLY)

# Install the TradeBaseConfig.cmake and TradeBaseConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TradeBaseConfig.cmake"
  "${PROJECT_BINARY_DIR}/TradeBaseConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT TradeBaseTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)