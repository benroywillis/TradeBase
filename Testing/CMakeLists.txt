add_executable(LastClose "LastCloseBacktest.cpp")
set_target_properties(LastClose
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(LastClose PRIVATE ${Platform_Inc})
target_link_libraries(LastClose PRIVATE TBPlatform)
add_test(NAME LC COMMAND LastClose ${CMAKE_SOURCE_DIR}/Testing/TestDataShort.csv)

add_executable(SMACrossover "SMACrossBacktest.cpp")
set_target_properties(SMACrossover
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(SMACrossover PRIVATE ${Platform_Inc} ${Indicators_Inc} ${Strategies_Inc})
target_link_libraries(SMACrossover PRIVATE TBPlatform)
add_test(NAME SMAC COMMAND SMACrossover ${CMAKE_SOURCE_DIR}/Testing/TestDataShort.csv)

add_executable(HPSMA "HPSMABacktest.cpp")
set_target_properties(HPSMA
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)
target_include_directories(HPSMA PRIVATE ${Platform_Inc} ${Indicators_Inc} ${Strategies_Inc})
target_link_libraries(HPSMA PRIVATE TBPlatform)
add_test(NAME HPSMAT COMMAND HPSMA ${CMAKE_SOURCE_DIR}/Testing/TestDataShort.csv)