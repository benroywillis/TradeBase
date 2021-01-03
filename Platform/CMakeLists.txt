file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "inc/*.h*")
add_library(TradeBase SHARED ${SOURCES})
set_target_properties(TradeBase
    PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PUBLIC_HEADER "${HEADERS}"
)
target_compile_options(TradeBase PRIVATE -fPIC)
target_include_directories(TradeBase PRIVATE "inc/")
target_link_libraries(TradeBase PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
install(TARGETS TradeBase EXPORT TradeBaseTargets LIBRARY DESTINATION "${INSTALL_LIB_DIR}" PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/TradeBase")