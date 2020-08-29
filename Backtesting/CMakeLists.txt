add_executable(HPSMA "HPSMABacktest.cpp")
set_target_properties(HPSMA
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(HPSMA PRIVATE ${Util_Inc} ${TMBackend_Inc} ${IBBackend_Inc} )
target_link_libraries(HPSMA PRIVATE "-lpthread" IBBackend TMBackend)

add_executable(LastClose "LastCloseBacktest.cpp")
set_target_properties(LastClose
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(LastClose PRIVATE ${Util_Inc} ${TMBackend_Inc} ${IBBackend_Inc})
target_link_libraries(LastClose PRIVATE "-lpthread" IBBackend TMBackend)

add_executable(SMACrossover "SMACrossBacktest.cpp")
set_target_properties(SMACrossover
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(SMACrossover PRIVATE ${Util_Inc} ${TMBackend_Inc} ${IBBackend_Inc})
target_link_libraries(SMACrossover PRIVATE "-lpthread" IBBackend TMBackend)
