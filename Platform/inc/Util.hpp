#pragma once
#include "BackTrader.h"
#include "Brain.h"
#include <iomanip>
#include <iostream>
#include <memory>

/// @brief Structure for reading input data that does not have its information internal to it.
///
/// Use this input to the BTData class if you have to specify the data characteristics yourself.
struct InputFile
{
    std::string filepath;
    std::string symbol;
    std::string secId;
    std::string secType;
    std::string exchange;
    std::string currency;
    std::string exprDate;
    bool        candles;
    bool        snaps;
    bool        options;
    /// Candle or Snap constructor
    InputFile( std::string path, std::string newSym, std::string newSecId, std::string newSecType, std::string newEx, std::string newCur, bool candle = true )
    {
        filepath = std::move( path );
        symbol = std::move( newSym );
        secId = std::move( newSecId );
        secType = std::move( newSecType );
        exchange = std::move( newEx );
        currency = std::move( newCur );
        exprDate = "";
        candles = candle;
        snaps = !candle;
        options = false;
    }
    /// Options data constructor
    InputFile( std::string path, std::string newSym, std::string newSecId, std::string newSecType, std::string newEx, std::string newCur, std::string newExprDate )
    {
        filepath = path;
        symbol = newSym;
        secId = newSecId;
        secType = newSecType;
        exchange = newEx;
        currency = newCur;
        exprDate = newExprDate;
        candles = false;
        snaps = false;
        options = true;
    }
};

/// Helpful for shrinking strategy class codes.
inline Order getOrder( bool buy, double quant = 1, std::string timeForce = "DAY", std::string oT = "MKT" )
{
    auto newOrder = Order();
    newOrder.orderId = BackTrader::getNextOrderId();
    newOrder.totalQuantity = quant;
    newOrder.tif = timeForce;
    newOrder.orderType = oT;
    if( buy )
    {
        newOrder.action = "BUY";
    }
    else
    {
        newOrder.action = "SELL";
    }
    return newOrder;
}

/// @brief Parses positional input args into filepaths
///
/// Use this method if your input data file already has its information at the top.
/// This method must be used if your file does not have its data column headers on the first line of the file.
inline std::vector<std::string> ReadInputs( int argc, char* argv[] )
{
    if( ( argc - 1 ) == 0 )
    {
        std::cout << "You must provide an input file path as a positional argument!" << std::endl;
        exit( -1 );
    }
    auto retPaths = std::vector<std::string>();
    for( int i = 1; i < argc; i++ )
    {
        retPaths.push_back( argv[i] );
    }
    return retPaths;
}

/// @brief Reads input arguments for input data where the data info is not within the file itself
///
/// tickerInfos should contain the information about an input csv, but not the filepath.
inline void ReadInputs( int argc, char* argv[], InputFile& tickerInfo )
{
    if( ( argc - 1 ) == 0 )
    {
        std::cout << "You must provide an input file path as a positional argument!" << std::endl;
        exit( -1 );
    }
    tickerInfo.filepath = argv[1];
}

/// @brief Reads input arguments for input data where the data info is not within the file itself
///
/// tickerInfos should contain the information about an input csv, but not the filepath.
inline void ReadInputs( int argc, char* argv[], std::vector<InputFile>& tickerInfos )
{
    if( ( argc - 1 ) != tickerInfos.size() )
    {
        std::cout << "Input arguments and file information lengths do not match!" << std::endl;
        exit( -1 );
    }
    for( int i = 1; i < argc; i++ )
    {
        tickerInfos[i - 1].filepath = argv[i];
    }
}

inline void PrintResults( const std::unique_ptr<BTBrain>& brain )
{
    std::cout << std::fixed << std::setprecision( 2 );
    std::cout << "Ending marked-to-market change was " << brain->getMTMChange() << std::endl;
    std::cout << "Total commission was " << brain->getTotalCommission() << std::endl;
    //std::cout << "PnL was " << brain->getPnL() << std::endl;
    //std::cout << "UPnL was " << brain->getUPnL() << std::endl;
    //std::cout << "Total account change was " << brain->getUPnL() + brain->getPnL() << std::endl;
    std::cout << "Ending positions are " << std::endl;
    for( auto& index : brain->getPositions() )
    {
        std::string name = index->getContract()->symbol;
        if( index->getContract()->secType == "OPT" )
        {
            name += std::to_string( index->getContract()->strike );
            name += index->getContract()->right;
        }
        std::cout << index->getContract()->symbol << ", " << index->getAvgPrice() << ", " << index->getPositionSize() << std::endl;
    }
}
