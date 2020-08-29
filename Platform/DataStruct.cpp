#include "DataStruct.h"

using namespace std;

DataStruct::DataStruct( CandleStruct candle )
{
    point = DataPoint( candle );
}

DataStruct::DataStruct( SnapStruct snap )
{
    point = DataPoint( snap );
}

DataStruct::DataStruct( OptionStruct option )
{
    point = DataPoint( option );
}

bool DataStruct::candles() const
{
    return get_if<CandleStruct>( &point ) != nullptr;
}

bool DataStruct::snaps() const
{
    return get_if<SnapStruct>( &point ) != nullptr;
}

bool DataStruct::options() const
{
    return get_if<OptionStruct>( &point ) != nullptr;
}

double DataStruct::get() const
{
    if( const auto* value = get_if<CandleStruct>( &point ) )
    {
        return value->close;
    }
    if( const auto* value = get_if<SnapStruct>( &point ) )
    {
        return ( value->askPrice + value->bidPrice ) / 2;
    }
    return std::get<OptionStruct>( point ).price;
}

const DataPoint& DataStruct::getPoint() const
{
    return point;
}

const TimeStamp& DataStruct::getTime() const
{
    if( const auto* candle = get_if<CandleStruct>( &point ) )
    {
        return candle->time;
    }
    if( const auto* snap = get_if<SnapStruct>( &point ) )
    {
        return snap->time;
    }
    return std::get<OptionStruct>( point ).time;
}

string DataStruct::toString() const
{
    if( candles() )
    {
        return std::get<CandleStruct>( point ).toString();
    }
    if( snaps() )
    {
        return std::get<SnapStruct>( point ).toString();
    }
    return std::get<OptionStruct>( point ).toString();
}