file(GLOB SOURCES "Indicators/*.cpp" "Strategies/*.cpp" )
add_library(TBAlpha SHARED ${SOURCES})
set_target_properties(TBAlpha
    PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" 
)
target_compile_options(TBAlpha PRIVATE -fPIC)
target_include_directories(TBAlpha PRIVATE "inc/" ${Platform_Inc} ${Indicators_Inc} ${Strategies_Inc})
target_link_libraries(TBAlpha PRIVATE TBPlatform spdlog::spdlog spdlog::spdlog_header_only)
install(TARGETS TBAlpha
    LIBRARY DESTINATION lib
)