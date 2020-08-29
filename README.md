# TradeBase
TradeBase is a C++ backtester for algorithmic trading systems.
It is built to provide the base of a C++ algorithmic trader with IB C++ API. 

## Building
The C++ projects are built using CMake with LLVM 10.
A C++ linter is baked into the build flow and can be turned off when configuring CMake.
This system has been developed and tested on Ubuntu 19.10 and 20.04 only. 
It does not work on Windows and will require some porting (OS interrupt handling and other stuff).

## Dependencies
1. spdlog
2. pthreads