file(GLOB SOURCES "*.cpp" )
add_library(TMBackend SHARED ${SOURCES})
set_target_properties(TMBackend
    PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" 
)
target_compile_options(TMBackend PRIVATE -fPIC)
target_include_directories(TMBackend PRIVATE "inc/" ${Util_Inc} ${IBBackend_Inc})
target_link_libraries(TMBackend PRIVATE IBBackend spdlog::spdlog spdlog::spdlog_header_only)
install(TARGETS TMBackend
    LIBRARY DESTINATION lib
)