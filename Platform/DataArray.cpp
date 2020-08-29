#include "DataArray.h"
#include "DataStruct.h"
#include <cmath>
#include <iterator>

using namespace std;

DataArray::DataArray( long newVecId, long newConId, const string& newSymbol, const string& newSecId, const string& newSecType, const string& newExchange, const string& newCurrency )
{
    vectorId = newVecId;
    contract = Contract();
    contract.conId = newConId;
    contract.symbol = newSymbol;
    contract.secId = newSecId;
    contract.secType = newSecType;
    contract.exchange = newExchange;
    contract.currency = newCurrency;
    array = list<DataStruct>();
    Inds = vector<unique_ptr<BTIndicator>>();
    length = array.size();
}

void DataArray::addPoint( CandleStruct& newPoint )
{
    // check for a time clash with the last data point
    // solves the precision problem brought about by ctime
    if( !array.empty() )
    {
        TimeStamp before = prev( array.end() )->getTime();
        if( before.year == newPoint.time.year && before.month == newPoint.time.month && before.day == newPoint.time.day && before.hour == newPoint.time.hour && before.minute == newPoint.time.minute && floor( before.second ) == floor( newPoint.time.second ) )
        {
            newPoint.time.second = before.second + 0.1;
        }
    }
    array.emplace_back( DataStruct( newPoint ) );
    length = array.size();
    for( auto& ind : Inds )
    {
        ind->update( array.back() );
    }
}

void DataArray::addPoint( SnapStruct& newPoint )
{
    // check for a time clash with the last data point
    // solves the precision problem brought about by ctime
    if( !array.empty() )
    {
        TimeStamp before = prev( array.end() )->getTime();
        if( before.year == newPoint.time.year && before.month == newPoint.time.month && before.day == newPoint.time.day && before.hour == newPoint.time.hour && before.minute == newPoint.time.minute && floor( before.second ) == floor( newPoint.time.second ) )
        {
            newPoint.time.second = before.second + 0.01;
        }
    }
    array.emplace_back( DataStruct( newPoint ) );
    length = array.size();
    for( auto& ind : Inds )
    {
        ind->update( array.back() );
    }
}

void DataArray::addPoint( OptionStruct& newPoint )
{
    // check for a time clash with the last data point
    // solves the precision problem brought about by ctime
    if( !array.empty() )
    {
        TimeStamp before = prev( array.end() )->getTime();
        if( before.year == newPoint.time.year && before.month == newPoint.time.month && before.day == newPoint.time.day && before.hour == newPoint.time.hour && before.minute == newPoint.time.minute && floor( before.second ) == floor( newPoint.time.second ) )
        {
            newPoint.time.second = before.second + 0.1;
        }
    }
    array.emplace_back( DataStruct( newPoint ) );
    length = array.size();
    for( auto& ind : Inds )
    {
        ind->update( array.back() );
    }
}

optional<list<DataStruct>::const_iterator> DataArray::getLastPoint() const
{
    if( !array.empty() )
    {
        auto it = prev( array.cend() );
        auto ret = make_optional( it );
        return ret;
    }
    return nullopt;
}

void DataArray::addIndicator( BTIndicator* newInd )
{
    Inds.push_back( newInd->clone() );
}

bool DataArray::validIndicators() const
{
    for( const auto& ind : Inds )
    {
        if( !ind->isValid() )
        {
            return false;
        }
    }
    return true;
}

string DataArray::getStart( bool fileOutput ) const
{
    if( !array.empty() )
    {
        return array.begin()->getTime().toString( fileOutput );
    }
    return "";
}

TimeStamp DataArray::getStartingTime() const
{
    if( !array.empty() )
    {
        return array.begin()->getTime();
    }
    return TimeStamp();
}

string DataArray::getEnd( bool fileOutput ) const
{
    if( !array.empty() )
    {
        return prev( array.end() )->getTime().toString( fileOutput );
    }
    return "";
}

TimeStamp DataArray::getEndingTime( bool fileOutput ) const
{
    if( !array.empty() )
    {
        return prev( array.end() )->getTime();
    }
    return TimeStamp();
}