#pragma once
#include "TimeStamp.h"

/// Structure for holding data
struct Data
{
    TimeStamp time;
    double    price;
    int       size;
    Data() = default;
};

/// Structure for holding candle data
struct CandleStruct : Data
{
    double      open;
    double      high;
    double      low;
    double      close;
    int         volume;
    std::string toString() const
    {
        return time.toString() + "," + std::to_string( open ) + "," + std::to_string( high ) + "," + std::to_string( low ) + "," + std::to_string( close ) + "," + std::to_string( volume );
    }
};

/// @brief Structure for holding snapshot data
///
/// There are two flavors of this structure: ask and bid stats are different and ask and bid stats are the same
/// If they are different, this snapshot represented a true bid/ask spread
/// If they are the same, this snapshot represented a trade execution
struct SnapStruct : Data
{
    int    bidSize;
    int    askSize;
    double bidPrice;
    double askPrice;

    SnapStruct()
    {
        time = TimeStamp();
        bidSize = -1;
        askSize = -1;
        bidPrice = -1;
        askPrice = -1;
    }
    SnapStruct( int newSize, bool bid = true )
    {
        if( bid )
        {
            bidSize = newSize;
            askSize = -1;
        }
        else
        {
            askSize = newSize;
            bidSize = -1;
        }
        time = TimeStamp();
        bidPrice = -1;
        askPrice = -1;
    }
    SnapStruct( double newPrice, bool bid = true )
    {
        if( bid )
        {
            bidPrice = newPrice;
            askPrice = -1;
        }
        else
        {
            bidPrice = -1;
            askPrice = newPrice;
        }
        time = TimeStamp();
        bidSize = -1;
        askSize = -1;
    }

    bool valid()
    {
        if( bidSize != -1 && bidPrice != -1 && askSize != -1 && askPrice != -1 )
        {
            time = TimeStamp( true );
            price = ( askPrice + bidPrice ) / 2;
            return true;
        }
        return false;
    }

    void clear()
    {
        time.clear();
        bidSize = -1;
        askSize = -1;
        bidPrice = -1;
        askPrice = -1;
    }

    std::string toString() const
    {
        return time.toString() + "," + std::to_string( bidPrice ) + "," + std::to_string( bidSize ) + "," + std::to_string( askPrice ) + "," + std::to_string( askSize );
    }
};

/// @brief Structure for option contract data
///
/// There are two flavors of this structure: ask and bid stats and greeks are different; ask and bid stats and greeks are the same
/// If they are different, this snapshot represented a true bid/ask spread
/// If they are the sa, this snapshot represented a trade execution
struct OptionStruct : Data
{
    int    bidSize;
    double bidPrice;
    double bidUndPrice;
    double bidImpliedVol;
    double bidDelta;
    double bidGamma;
    double bidVega;
    double bidTheta;
    double bidPvDividend;
    int    askSize;
    double askPrice;
    double askUndPrice;
    double askImpliedVol;
    double askDelta;
    double askGamma;
    double askVega;
    double askTheta;
    double askPvDividend;
    OptionStruct()
    {
        time = TimeStamp();
        bidSize = -1;
        bidPrice = -1;
        bidUndPrice = -1;
        bidImpliedVol = -1;
        bidDelta = -1;
        bidGamma = -1;
        bidVega = -1;
        bidTheta = -1;
        bidPvDividend = -1;
        askSize = -1;
        askPrice = -1;
        askUndPrice = -1;
        askImpliedVol = -1;
        askDelta = -1;
        askGamma = -1;
        askVega = -1;
        askTheta = -1;
        askPvDividend = -1;
    }
    bool valid()
    {
        if( bidSize != -1 && bidPrice != -1 && bidImpliedVol != -1 && bidDelta != -1 && bidGamma != -1 && bidVega != -1 && bidTheta != -1 && bidPvDividend != -1 && askPrice != -1 && askSize != -1 && askImpliedVol != -1 && askDelta != -1 && askGamma != -1 && askVega != -1 && askTheta != -1 && askPvDividend != -1 )
        {
            time = TimeStamp( true );
            price = ( askPrice + bidPrice ) / 2;
            return true;
        }
        return false;
    }
    void clear()
    {
        time.clear();
        bidSize = -1;
        bidPrice = -1;
        bidUndPrice = -1;
        bidImpliedVol = -1;
        bidDelta = -1;
        bidGamma = -1;
        bidVega = -1;
        bidTheta = -1;
        bidPvDividend = -1;
        askSize = -1;
        askPrice = -1;
        askUndPrice = -1;
        askImpliedVol = -1;
        askDelta = -1;
        askGamma = -1;
        askVega = -1;
        askTheta = -1;
        askPvDividend = -1;
    }
    std::string toString() const
    {
        return time.toString() + "," + std::to_string( bidImpliedVol ) + "," + std::to_string( bidDelta ) + "," + std::to_string( bidPrice ) + "," + std::to_string( bidSize ) + "," + std::to_string( bidPvDividend ) + "," + std::to_string( bidGamma ) + "," + std::to_string( bidVega ) + "," + std::to_string( bidTheta ) + "," + std::to_string( askImpliedVol ) + "," + std::to_string( askDelta ) + "," + std::to_string( askPrice ) + "," + std::to_string( askSize ) + "," + std::to_string( askPvDividend ) + "," + std::to_string( askGamma ) + "," + std::to_string( askVega ) + "," + std::to_string( askTheta );
    }
};