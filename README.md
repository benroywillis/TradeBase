# TradeBase
TradeBase is a C++ backtester for algorithmic trading systems.
A platform suitable for HFT backtesting, it is also built to provide the base of a C++ algorithmic trader with IB C++ API. 

## Building
The CMake script is straightforward, building should be simple on linux.
It does not work on Windows and will require some porting (OS interrupt handling and other stuff).
A C++ linter is baked into the build flow and can be turned off when configuring CMake.
This system has been developed and tested on Ubuntu 19.10 and 20.04 with LLVM 10 only. 

## Dependencies
1. spdlog