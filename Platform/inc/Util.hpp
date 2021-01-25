#pragma once
#include "BackTrader.h"
#include "Brain.h"
#include "matplotlibcpp.h"
#include <iomanip>
#include <iostream>
#include <memory>

namespace TradeBase
{
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

    inline void PrintResults( const std::unique_ptr<TBBrain>& brain )
    {
        std::cout << std::fixed << std::setprecision( 2 );
        std::cout << "Total commission was " << brain->getTotalCommission() << std::endl;
        std::cout << "Maximum gain was " << brain->getMaximumGain() << std::endl;
        std::cout << "Maximum drawdown was " << brain->getMaximumDrawdown() << std::endl;
        std::cout << "Ending marked-to-market change was " << brain->getMTMChange() << std::endl;
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

    inline void PlotResults( const std::unique_ptr<TBBrain>& brain )
    {
        matplotlibcpp::plot( brain->getTimeVector(), brain->getMTMHistory() );
        matplotlibcpp::show();
    }
} // namespace TradeBase