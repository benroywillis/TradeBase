#include "Data.h"
#include "DataStruct.h"
#include "Indicator.h"
#include "Order.h"
#include "Util.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

using namespace std;

BTData::BTData()
{
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
}

BTData::BTData( InputFile& csv )
{
    auto newVec = readVector( csv );
    DataArrays.insert( newVec );
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    indicators = vector<BTIndicator*>();
    processData();
    initTimeLine();
}

BTData::BTData( InputFile& csv, vector<BTIndicator*>& newIndicators )
{
    auto newVec = readVector( csv );
    indicators = newIndicators;
    for( const auto& ind : indicators )
    {
        newVec->addIndicator( ind );
    }
    DataArrays.insert( newVec );
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    processData();
    initTimeLine();
}

BTData::BTData( vector<InputFile>& csvs )
{
    for( auto& csv : csvs )
    {
        auto newVec = readVector( csv );
        DataArrays.insert( newVec );
    }
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    indicators = vector<BTIndicator*>();
    processData();
    initTimeLine();
}

BTData::BTData( const string& filepath )
{
    auto newVec = readVector( filepath );
    newVec->vectorId = getNextVectorId();
    newVec->contract.conId = getNextConId();
    DataArrays.insert( newVec );
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    indicators = vector<BTIndicator*>();
    processData();
    initTimeLine();
}

BTData::BTData( const vector<string>& filepaths, const vector<BTIndicator*>& newIndicators )
{
    indicators = newIndicators;
    for( const auto& path : filepaths )
    {
        auto newVec = readVector( path );
        newVec->vectorId = getNextVectorId();
        newVec->contract.conId = getNextConId();
        DataArrays.insert( newVec );
    }
    for( const auto& ind : indicators )
    {
        for( const auto& vec : DataArrays )
        {
            vec->addIndicator( ind );
        }
    }
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    processData();
    initTimeLine();
}

BTData::BTData( const vector<string>& filepaths )
{
    for( const auto& path : filepaths )
    {
        auto newVec = readVector( path );
        newVec->vectorId = getNextVectorId();
        newVec->contract.conId = getNextConId();
        DataArrays.insert( newVec );
    }
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    indicators = vector<BTIndicator*>();
    processData();
    initTimeLine();
}

BTData::BTData( shared_ptr<DataArray> newVector )
{
    DataArrays.insert( move( newVector ) );
    indicators = vector<BTIndicator*>();
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    initTimeLine();
}

BTData::BTData( const vector<shared_ptr<DataArray>>& newData )
{
    DataArrays.insert( begin( newData ), end( newData ) );
    TimeLine = TimeMap();
    currentTime = TimeLine.begin();
    indicators = vector<BTIndicator*>();
    initTimeLine();
}

shared_ptr<DataArray> BTData::readVector( const InputFile& csv )
{
    ifstream csvFile( csv.filepath, ifstream::in );
    if( !csvFile.is_open() )
    {
        spdlog::error( "The csv file " + csv.filepath + " could not be found." );
        exit( 1 );
    }
    string headerLine;
    getline( csvFile, headerLine, '\n' );
    auto newVec = make_shared<DataArray>( getNextVectorId(), getNextConId(), csv.symbol, csv.secId, csv.secType, csv.exchange, csv.currency );
    if( csv.candles )
    {
        ReadCandleData( csvFile, headerLine, newVec->array );
    }
    else if( csv.snaps )
    {
        ReadSnapData( csvFile, headerLine, newVec->array );
    }
    else if( csv.options )
    {
        ReadOptionData( csvFile, headerLine, newVec->array );
    }
    return newVec;
}

void BTData::addData( const shared_ptr<DataArray>& newData )
{
    DataArrays.insert( newData );
    checkData( newData );
}

void BTData::checkData( const shared_ptr<DataArray>& newData )
{
    if( newData->getStartingTime() != globalStartingTime )
    {
        // have to shave the new vector to fit this starting time
    }
    if( newData->getEndingTime() != globalEndingTime )
    {
        // have to shave the new vector to fit this ending time
    }
    // evaluate the timestamps of the new vector and see how it fits into global time
    // idea: create a global vector of TimeStamp objects with pointers to all data vectors that have data at that time. Fit the new data into this global vector
}

void BTData::initTimeLine()
{
    // evaluate the boundaries of each vector
    int maxlength = 0;
    for( const auto& vec : DataArrays )
    {
        globalStartingTime = vec->getStartingTime() < globalStartingTime ? vec->getStartingTime() : globalStartingTime;
        globalEndingTime = vec->getEndingTime() > globalEndingTime ? vec->getEndingTime() : globalEndingTime;
    }
    // construct global time vector
    int vectorIndex;
    for( const auto& vec : DataArrays )
    {
        for( auto point = vec->array.cbegin(); point != vec->array.cend(); point++ )
        {
            if( TimeLine.find( point->getTime() ) == TimeLine.end() )
            {
                TimeLine[point->getTime()] = GlobalTimePoint( vec, point );
            }
            else
            {
                TimeLine[point->getTime()].addVector( vec, point );
            }
        }
    }
    currentTime = TimeLine.begin();
}

void BTData::processData()
{
    // check the order of each vector and reverse if necessary
    // first index should be the oldest timepoint, last index most recent timepoint
    for( const auto& entry : DataArrays )
    {
        bool reversed = false;
        for( auto it = entry->array.begin(); it != prev( entry->array.end() ); it++ )
        {
            if( it->getTime() > next( it )->getTime() )
            {
                reversed = true;
                break;
            }
        }
        if( reversed )
        {
            reverse( entry->array.begin(), entry->array.end() );
        }
    }
    // parse time interval
    findTimeInterval();
    // parse each price, volume, make sure they are not 0
    // parse data into separate days?
}

void BTData::findTimeInterval()
{
    for( const auto& entry : DataArrays )
    {
        // check each clock point and if we get a consistent, non-zero interval for one of the stats, thats the interval to choose
        double seconds = 0;
        double minutes = 0;
        double hours = 0;
        double days = 0;
        double months = 0;
        double years = 0;
        for( auto it = entry->array.begin(); it != next( entry->array.begin(), 10 ); it++ )
        {
            seconds += ( next( it )->getTime().second - it->getTime().second );
            minutes += ( next( it )->getTime().minute - it->getTime().minute );
            hours += ( next( it )->getTime().hour - it->getTime().hour );
            days += ( next( it )->getTime().day - it->getTime().day );
            months += ( next( it )->getTime().month - it->getTime().month );
            years += ( next( it )->getTime().year - it->getTime().year );
        }
        double secondInterval = seconds / 10;
        double minuteInterval = minutes / 10;
        double hourInterval = hours / 10;
        double dayInterval = seconds / 10;
        double monthInterval = minutes / 10;
        double yearInterval = hours / 10;
        if( secondInterval > 0 )
        {
            entry->interval = to_string( (int)secondInterval ) + "s";
            continue;
        }
        if( minuteInterval > 0 )
        {
            entry->interval = to_string( (int)minuteInterval ) + "m";
            continue;
        }
        if( hourInterval > 0 )
        {
            entry->interval = to_string( (int)hourInterval ) + "h";
            continue;
        }
        if( dayInterval > 0 )
        {
            entry->interval = to_string( (int)secondInterval ) + "d";
            continue;
        }
        if( monthInterval > 0 )
        {
            entry->interval = to_string( (int)minuteInterval ) + "M";
            continue;
        }
        if( yearInterval > 0 )
        {
            entry->interval = to_string( (int)hourInterval ) + "y";
        }
    }
}

bool BTData::incrementTime()
{
    // increment global time vector
    currentTime = next( currentTime );
    bool more = currentTime != TimeLine.end();
    if( more )
    {
        for( const auto& point : currentTime->second.Points )
        {
            for( const auto& vec : DataArrays )
            {
                if( vec->contract.conId == point.p_Vector->contract.conId )
                {
                    for( auto& ind : point.p_Vector->Inds )
                    {
                        ind->update( *( point.p_Point ) );
                    }
                }
            }
        }
    }
    return more;
}

optional<VectorPoint> BTData::lookup( long conId, bool current )
{
    if( current )
    {
        for( const auto& point : currentTime->second.Points )
        {
            if( point.p_Vector->contract.conId == conId )
            {
                return make_optional<VectorPoint>( point );
            }
        }
        return nullopt;
    }
    // look up the last time this contract ID appeared in the data vector
    for( auto it = currentTime; it != TimeLine.begin(); it-- )
    {
        for( const auto& point : it->second.Points )
        {
            if( point.p_Vector->contract.conId == conId )
            {
                return make_optional<VectorPoint>( point );
            }
        }
    }
    return nullopt;
}

const TimeStamp& BTData::getCurrentTime() const
{
    return currentTime->first;
}

const GlobalTimePoint& BTData::getCurrentPoint() const
{
    return currentTime->second;
}

optional<const VectorPoint> BTData::getCurrentVectorPoint( long vecId ) const
{
    for( const auto& point : currentTime->second.Points )
    {
        if( point.p_Vector->vectorId == vecId )
        {
            return make_optional<const VectorPoint>( point );
        }
    }
    return nullopt;
}

optional<const TimeStamp> BTData::getPreviousTime() const
{
    if( currentTime != TimeLine.begin() )
    {
        return make_optional<const TimeStamp>( prev( currentTime )->first );
    }
    return nullopt;
}

optional<const GlobalTimePoint> BTData::getPreviousPoint() const
{
    if( currentTime == TimeLine.begin() )
    {
        return nullopt;
    }
    return make_optional<const GlobalTimePoint>( prev( currentTime )->second );
}

optional<const VectorPoint> BTData::getPreviousVectorPoint( long vecId ) const
{
    if( currentTime == TimeLine.begin() )
    {
        return nullopt;
    }
    auto prevPoint = prev( currentTime );
    do
    {
        for( const auto& vec : prevPoint->second.Points )
        {
            if( vec.p_Vector->vectorId == vecId )
            {
                return make_optional<const VectorPoint>( vec );
            }
        }
        prevPoint = prev( prevPoint );
    } while( prevPoint != TimeLine.begin() );
    return nullopt;
}

shared_ptr<DataArray> BTData::readVector( const string& filepath )
{
    ifstream csvFile( filepath, ifstream::in );
    if( !csvFile.is_open() )
    {
        spdlog::error( "The csv file " + filepath + " could not be found." );
        exit( 1 );
    }
    auto newVector = make_shared<DataArray>();
    // Stores the line that holds column names
    // We have to read this from the ifstream before we know what it is, and it must be passed to the data parsers, and this stores it
    string headerLine;
    // read the file until we hit the column names
    string lineStr;
    while( getline( csvFile, lineStr, '\n' ) )
    {
        size_t colon = lineStr.find( ':' );
        // if there is no colon in this line, we have hit the column names
        if( colon == string::npos )
        {
            headerLine = lineStr;
            break;
        }
        string name = lineStr.substr( 0, colon );
        // increment past colon
        string value = lineStr.substr( colon + 1, string::npos );
        if( !value.empty() )
        {
            if( name == "symbol" )
            {
                newVector->contract.symbol = value;
            }
            else if( name == "secId" )
            {
                newVector->contract.secId = value;
            }
            else if( name == "secType" )
            {
                newVector->contract.secType = value;
            }
            else if( name == "exchange" )
            {
                newVector->contract.exchange = value;
            }
            else if( name == "currency" )
            {
                newVector->contract.currency = value;
            }
            else if( name == "conId" )
            {
                auto sz = string::size_type();
                newVector->contract.conId = stol( value, &sz );
            }
            else if( name == "interval" )
            {
                newVector->interval = value;
            }
            else if( name == "strike" )
            {
                newVector->contract.strike = stof( value );
            }
            else if( name == "exprDate" )
            {
                newVector->exprDate = TimeStamp( value, true );
                newVector->contract.lastTradeDateOrContractMonth = newVector->exprDate.toString( false, true );
            }
            else if( name == "right" )
            {
                newVector->contract.right = value;
            }
        }
    }
    // read the data
    if( newVector->contract.strike != 0 )
    {
        ReadOptionData( csvFile, headerLine, newVector->array );
    }
    else if( newVector->interval.empty() )
    {
        ReadSnapData( csvFile, headerLine, newVector->array );
    }
    else
    {
        ReadCandleData( csvFile, headerLine, newVector->array );
    }
    return newVector;
}

void BTData::ReadCandleData( ifstream& csvFile, string& headerLine, list<DataStruct>& data )
{
    // evaluate column header names
    int openCol = -1;
    int highCol = -1;
    int lowCol = -1;
    int closeCol = -1;
    int volCol = -1;
    int timeCol = -1;

    size_t         hpos = 0;
    size_t         hnextpos = headerLine.find( ',' );
    string         htoken;
    vector<string> hvalues;
    while( hnextpos != string::npos )
    {
        htoken = headerLine.substr( hpos, hnextpos - hpos );
        hvalues.push_back( htoken );
        hpos = hnextpos + 1;
        hnextpos = headerLine.find( ',', hpos );
    }
    // push the last entry, volume, that is not ended with a comma
    hvalues.push_back( headerLine.substr( hpos, string::npos ) );
    // lower their case first
    vector<string> headerNames;
    headerNames.reserve( hvalues.size() );
    for( const auto& index : hvalues )
    {
        headerNames.emplace_back( index );
    }
    for( int i = 0; i < headerNames.size(); i++ )
    {
        if( headerNames[i] == "open" )
        {
            openCol = i;
        }
        else if( headerNames[i] == "high" )
        {
            highCol = i;
        }
        else if( headerNames[i] == "low" )
        {
            lowCol = i;
        }
        else if( headerNames[i] == "close" )
        {
            closeCol = i;
        }
        else if( headerNames[i] == "vol" || headerNames[i] == "volume" )
        {
            volCol = i;
        }
        else if( headerNames[i] == "timestamp" )
        {
            timeCol = i;
        }
    }

    string lineStr;
    while( getline( csvFile, lineStr, '\n' ) )
    {
        size_t         pos = 0;
        size_t         nextpos = lineStr.find( ',' );
        string         token;
        vector<string> values;
        CandleStruct   newCandle;
        while( nextpos != string::npos )
        {
            token = lineStr.substr( pos, nextpos - pos );
            values.push_back( token );
            pos = nextpos + 1;
            nextpos = lineStr.find( ',', pos );
        }
        for( int i = 0; i < values.size(); i++ )
        {
            if( i == openCol )
            {
                newCandle.open = stof( values[i] );
            }
            else if( i == highCol )
            {
                newCandle.high = stof( values[i] );
            }
            else if( i == lowCol )
            {
                newCandle.low = stof( values[i] );
            }
            else if( i == closeCol )
            {
                newCandle.close = stof( values[i] );
            }
            else if( i == volCol )
            {
                newCandle.volume = stoi( values[i] );
            }
            else if( i == timeCol )
            {
                newCandle.time = TimeStamp( values[i] );
            }
        }
        data.emplace_back( DataStruct( newCandle ) );
    }
}

void BTData::ReadSnapData( ifstream& csvFile, string& headerLine, list<DataStruct>& data )
{
    // evaluate column headers first
    int            timeCol = -1;
    int            bidPriceCol = -1;
    int            askPriceCol = -1;
    int            bidSizeCol = -1;
    int            askSizeCol = -1;
    size_t         hpos = 0;
    size_t         hnextpos = headerLine.find( ',' );
    string         htoken;
    vector<string> hvalues;
    while( hnextpos != string::npos )
    {
        htoken = headerLine.substr( hpos, hnextpos - hpos );
        hvalues.push_back( htoken );
        hpos = hnextpos + 1;
        hnextpos = headerLine.find( ',', hpos );
    }
    // push the last entry, volume, that is not ended with a comma
    hvalues.push_back( headerLine.substr( hpos, string::npos ) );
    // lower their case first
    vector<string> headerNames;
    headerNames.reserve( hvalues.size() );
    for( const auto& index : hvalues )
    {
        headerNames.emplace_back( index );
    }
    // timestamp, price, size
    for( int i = 0; i < headerNames.size(); i++ )
    {
        if( headerNames[i] == "timestamp" )
        {
            timeCol = i;
        }
        else if( headerNames[i] == "bidPrice" )
        {
            bidPriceCol = i;
        }
        else if( headerNames[i] == "askPrice" )
        {
            askPriceCol = i;
        }
        else if( headerNames[i] == "bidSize" )
        {
            bidSizeCol = i;
        }
        else if( headerNames[i] == "askSize" )
        {
            askSizeCol = i;
        }
    }

    string lineStr;
    while( getline( csvFile, lineStr, '\n' ) )
    {
        size_t         pos = 0;
        size_t         nextpos = lineStr.find( ',' );
        string         token;
        vector<string> values;
        SnapStruct     newSnap;
        while( nextpos != string::npos )
        {
            token = lineStr.substr( pos, nextpos - pos );
            values.push_back( token );
            pos = nextpos + 1;
            nextpos = lineStr.find( ',', pos );
        }
        for( int i = 0; i < values.size(); i++ )
        {
            if( i == bidPriceCol )
            {
                newSnap.bidPrice = stof( values[i] );
            }
            else if( i == askPriceCol )
            {
                newSnap.askPrice = stof( values[i] );
            }
            else if( i == bidSizeCol )
            {
                newSnap.bidSize = stoi( values[i] );
            }
            else if( i == askSizeCol )
            {
                newSnap.askSize = stoi( values[i] );
            }
            else if( i == timeCol )
            {
                newSnap.time = TimeStamp( values[i] );
            }
        }
        newSnap.price = ( newSnap.bidPrice + newSnap.askPrice ) / 2;
        newSnap.size = newSnap.askSize;
        data.emplace_back( DataStruct( newSnap ) );
    }
}

void BTData::ReadOptionData( ifstream& csvFile, string& headerLine, list<DataStruct>& data )
{
    // evaluate column headers
    int timeCol = -1;
    int bidImpVolCol = -1;
    int bidDelCol = -1;
    int bidPriceCol = -1;
    int bidSizeCol = -1;
    int bidPvDivCol = -1;
    int bidGamCol = -1;
    int bidVegCol = -1;
    int bidThetCol = -1;
    int bidUndCol = -1;
    int askImpVolCol = -1;
    int askDelCol = -1;
    int askPriceCol = -1;
    int askSizeCol = -1;
    int askPvDivCol = -1;
    int askGamCol = -1;
    int askVegCol = -1;
    int askThetCol = -1;
    int askUndCol = -1;

    size_t         hpos = 0;
    size_t         hnextpos = headerLine.find( ',' );
    string         htoken;
    vector<string> hvalues;
    while( hnextpos != string::npos )
    {
        htoken = headerLine.substr( hpos, hnextpos - hpos );
        hvalues.push_back( htoken );
        hpos = hnextpos + 1;
        hnextpos = headerLine.find( ',', hpos );
    }
    // push the last entry, volume, that is not ended with a comma
    hvalues.push_back( headerLine.substr( hpos, string::npos ) );
    // lower their case first
    vector<string> headerNames;
    headerNames.reserve( hvalues.size() );
    for( const auto& index : hvalues )
    {
        headerNames.emplace_back( index );
    }
    // timestamp, bidImpVol, bidDel, bidPrice, bidPvDiv, bidGam, bidVeg, bidThet, bidUnd, askImpVol, askDel, askPrice, askPvDiv, askGam, askVeg, askThet, askUnd, vol
    for( int i = 0; i < headerNames.size(); i++ )
    {
        if( headerNames[i] == "timestamp" )
        {
            timeCol = i;
        }
        else if( headerNames[i] == "bidImpVol" )
        {
            bidImpVolCol = i;
        }
        else if( headerNames[i] == "bidDel" )
        {
            bidDelCol = i;
        }
        else if( headerNames[i] == "bidPrice" )
        {
            bidPriceCol = i;
        }
        else if( headerNames[i] == "bidSize" )
        {
            bidSizeCol = i;
        }
        else if( headerNames[i] == "bidPvDiv" )
        {
            bidPvDivCol = i;
        }
        else if( headerNames[i] == "bidGam" )
        {
            bidGamCol = i;
        }
        else if( headerNames[i] == "bidVeg" )
        {
            bidVegCol = i;
        }
        else if( headerNames[i] == "bidThet" )
        {
            bidThetCol = i;
        }
        else if( headerNames[i] == "bidUnd" )
        {
            bidUndCol = i;
        }
        else if( headerNames[i] == "askImpVol" )
        {
            askImpVolCol = i;
        }
        else if( headerNames[i] == "askDel" )
        {
            askDelCol = i;
        }
        else if( headerNames[i] == "askPrice" )
        {
            askPriceCol = i;
        }
        else if( headerNames[i] == "askSize" )
        {
            askSizeCol = i;
        }
        else if( headerNames[i] == "askPvDiv" )
        {
            askPvDivCol = i;
        }
        else if( headerNames[i] == "askGam" )
        {
            askGamCol = i;
        }
        else if( headerNames[i] == "askVeg" )
        {
            askVegCol = i;
        }
        else if( headerNames[i] == "askThet" )
        {
            askThetCol = i;
        }
        else if( headerNames[i] == "askUnd" )
        {
            askUndCol = i;
        }
    }
    string lineStr;
    while( getline( csvFile, lineStr, '\n' ) )
    {
        size_t         pos = 0;
        size_t         nextpos = lineStr.find( ',' );
        string         token;
        vector<string> values;
        OptionStruct   newOption;
        while( nextpos != string::npos )
        {
            token = lineStr.substr( pos, nextpos - pos );
            values.push_back( token );
            pos = nextpos + 1;
            nextpos = lineStr.find( ',', pos );
        }
        // push the last entry, volume, that is not ended with a comma
        values.push_back( lineStr.substr( pos, string::npos ) );
        for( int i = 0; i < values.size(); i++ )
        {
            // bid computations
            if( i == bidImpVolCol )
            {
                newOption.bidImpliedVol = stof( values[i] );
            }
            else if( i == bidDelCol )
            {
                newOption.bidDelta = stof( values[i] );
            }
            else if( i == bidPriceCol )
            {
                newOption.bidPrice = stof( values[i] );
            }
            else if( i == bidSizeCol )
            {
                newOption.bidSize = stof( values[i] );
            }
            else if( i == bidPvDivCol )
            {
                newOption.bidPvDividend = stof( values[i] );
            }
            else if( i == bidGamCol )
            {
                newOption.bidGamma = stof( values[i] );
            }
            else if( i == bidVegCol )
            {
                newOption.bidVega = stof( values[i] );
            }
            else if( i == bidThetCol )
            {
                newOption.bidTheta = stof( values[i] );
            }
            else if( i == bidUndCol )
            {
                newOption.bidUndPrice = stof( values[i] );
            }
            // ask computations
            else if( i == askImpVolCol )
            {
                newOption.askImpliedVol = stof( values[i] );
            }
            else if( i == askDelCol )
            {
                newOption.askDelta = stof( values[i] );
            }
            else if( i == askPriceCol )
            {
                newOption.askPrice = stof( values[i] );
            }
            else if( i == askSizeCol )
            {
                newOption.askSize = stof( values[i] );
            }
            else if( i == askPvDivCol )
            {
                newOption.askPvDividend = stof( values[i] );
            }
            else if( i == askGamCol )
            {
                newOption.askGamma = stof( values[i] );
            }
            else if( i == askVegCol )
            {
                newOption.askVega = stof( values[i] );
            }
            else if( i == askThetCol )
            {
                newOption.askTheta = stof( values[i] );
            }
            else if( i == askUndCol )
            {
                newOption.askUndPrice = stof( values[i] );
            }
            else if( i == timeCol )
            {
                newOption.time = TimeStamp( values[i] );
            }
        }
        newOption.price = ( newOption.bidPrice + newOption.askPrice ) / 2;
        newOption.size = newOption.askSize;
        data.emplace_back( DataStruct( newOption ) );
    }
}

void BTData::printCSVs()
{
    for( const auto& vec : DataArrays )
    {
        // print each array to CSV
        ofstream output;
        string   fileString = "symbol:" + vec->contract.symbol + "\n";
        fileString += "secId:" + vec->contract.secId + "\n";
        fileString += "secType:" + vec->contract.secType + "\n";
        fileString += "exchange:" + vec->contract.exchange + "\n";
        fileString += "currency:" + vec->contract.currency + "\n";
        fileString += "conId:" + to_string( vec->contract.conId ) + "\n";
        if( vec->array.begin()->candles() )
        {
            fileString += "interval: " + vec->interval + "\n";
            fileString += "timestamp,open,high,low,close,volume\n";
            for( const auto& p : vec->array )
            {
                fileString += p.toString() + "\n";
            }
            if( vec->array.size() > 1 )
            {
                output.open( vec->contract.symbol + "_candle_" + vec->getStart( true ) + "_" + vec->getEnd( true ) + "_" + vec->interval + "_" + ".csv" );
                output.write( fileString.data(), fileString.size() );
            }
        }
        else if( vec->array.begin()->snaps() )
        {

            fileString += "timestamp,askPrice,askSize,bidPrice,bidSize\n";
            for( const auto& p : vec->array )
            {
                fileString += p.toString() + "\n";
            }
            if( vec->array.size() > 1 )
            {
                output.open( vec->contract.symbol + "_snap_" + vec->getStart( true ) + "_" + vec->getEnd( true ) + ".csv" );
                output.write( fileString.data(), fileString.size() );
            }
        }
        else if( vec->array.begin()->options() )
        {
            fileString += "strike:" + to_string( vec->contract.strike ) + "\n";
            fileString += "exprDate:" + vec->exprDate.toString() + "\n";
            fileString += "right:" + vec->contract.right + "\n";
            fileString += "timestamp,bidImpVol,bidDel,bidPrice,bidSize,bidPvDiv,bidGam,bidVeg,bidThet,askImpVol,askDel,askPrice,askSize,askPvDiv,askGam,askVeg,askThet\n";

            for( const auto& p : vec->array )
            {
                fileString += p.toString() + "\n";
            }
            if( vec->array.size() > 1 )
            {
                output.open( vec->contract.symbol + "_option_" + vec->getStart( true ) + "_" + vec->getEnd( true ) + "_" + vec->exprDate.toString( true ) + "_" + to_string( vec->contract.strike ) + "_" + vec->contract.right + ".csv" );
                output.write( fileString.data(), fileString.size() );
            }
        }
    }
}