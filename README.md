# TradeBase
TradeBase is a C++ backtester for algorithmic trading systems.

## Building
This project has been developed and tested on Ubuntu 19.10 and 20.04 with LLVM 10 only. 
It does not work on Windows and will require some porting (OS interrupt handling and other stuff).

## Dependencies
1. spdlog

## Usage
There are three general steps to backtest your strategy
1. Create a strategy class that inherits from BTStrategy
   - The only method you should have to worry about is ProcessNextTick()
   - Use the strategies in the test programs to get started
2. Construct a driver program
   - Use any of the test programs as an example to start with
3. Run the resulting program inputting data to the program via csv file
   - The program can support multiple csv inputs. Each csv argument is positional, order does not matter, and a position will be allocated for each
   - The csv file format should be exactly like Testing/TestDataShort.csv (or TestDataLong.csv, they're the same). For more detail, see documentation in Platform/Data.h
   - Visual results not supported
