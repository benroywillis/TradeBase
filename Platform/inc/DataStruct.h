#pragma once
#include "DataTypes.h"
#include <variant>

namespace TradeBase
{
    typedef std::variant<CandleStruct, SnapStruct, OptionStruct> DataPoint;

    class DataStruct
    {
    public:
        DataStruct() = default;
        DataStruct( CandleStruct candle );
        DataStruct( SnapStruct snap );
        DataStruct( OptionStruct option );
        bool             candles() const;
        bool             snaps() const;
        bool             options() const;
        double           get() const;
        const DataPoint& getPoint() const;
        const TimeStamp& getTime() const;
        std::string      toString() const;

    private:
        DataPoint point;
    };
} // namespace TradeBase