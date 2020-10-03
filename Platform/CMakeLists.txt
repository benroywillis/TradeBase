file(GLOB SOURCES "*.cpp")
add_library(TBPlatform SHARED ${SOURCES})
set_target_properties(TBPlatform
    PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" 
)
target_compile_options(TBPlatform PRIVATE -fPIC)
target_include_directories(TBPlatform PRIVATE "inc/")
target_link_libraries(TBPlatform PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
install(TARGETS TBPlatform
    LIBRARY DESTINATION lib
)