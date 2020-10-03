#pragma once
#include "BackTrader.h"
#include "DataArray.h"
#include <functional>
#include <iterator>
#include <map>
#include <set>

struct InputFile;

struct VectorPoint
{
    std::shared_ptr<DataArray>            p_Vector;
    std::list<DataStruct>::const_iterator p_Point;
    VectorPoint( const std::shared_ptr<DataArray>& newData, const std::list<DataStruct>::const_iterator& newIndex )
    {
        p_Vector = newData;
        p_Point = newIndex;
    }
};

struct GlobalTimePoint
{
    std::vector<VectorPoint> Points;
    GlobalTimePoint() = default;
    GlobalTimePoint( const std::shared_ptr<DataArray>& data, const std::list<DataStruct>::const_iterator& index )
    {
        Points.push_back( VectorPoint( data, index ) );
    }
    GlobalTimePoint( const std::vector<std::shared_ptr<DataArray>>& data, const std::list<DataStruct>::const_iterator& index )
    {
        for( const auto& vec : data )
        {
            Points.push_back( VectorPoint( vec, index ) );
        }
    }
    void addVector( const std::shared_ptr<DataArray>& data, const std::list<DataStruct>::const_iterator& index )
    {
        Points.push_back( VectorPoint( data, index ) );
    }
};

typedef std::map<TimeStamp, GlobalTimePoint> TimeMap;

class BTData : public BackTrader
{
public:
    BTData();
    /// @brief Constructor for a filepath
    ///
    /// Use this constructor for input data files that have all their information within the file itself (before the data column headers).
    BTData( const std::string& );
    /// Vector version of above
    BTData( const std::vector<std::string>& );
    BTData( const std::vector<std::string>&, const std::vector<BTIndicator*>& );
    /// Use this constructor if you have a data file that does not have its information above the data column headers.
    BTData( InputFile& );
    BTData( InputFile&, std::vector<BTIndicator*>& );
    /// Vector version of above.
    BTData( std::vector<InputFile>& );
    /// Use this constructor for data vectors that are already being collected.
    BTData( std::shared_ptr<DataArray> );
    /// Vector version of above
    BTData( const std::vector<std::shared_ptr<DataArray>>& );

    ~BTData() = default;

    void addData( const std::shared_ptr<DataArray>& );

    void                                 initTimeLine();
    void                                 checkData( const std::shared_ptr<DataArray>& );
    bool                                 incrementTime();
    const TimeStamp&                     getCurrentTime() const;
    const GlobalTimePoint&               getCurrentPoint() const;
    std::optional<const VectorPoint>     getCurrentVectorPoint( long ) const;
    std::optional<const TimeStamp>       getPreviousTime() const;
    std::optional<const GlobalTimePoint> getPreviousPoint() const;
    std::optional<const VectorPoint>     getPreviousVectorPoint( long ) const;

    std::optional<VectorPoint> lookup( long, bool current = true );

    struct VectorCompare
    {
        using is_transparent = void;
        bool operator()( const std::shared_ptr<DataArray>& lhs, const std::shared_ptr<DataArray>& rhs ) const
        {
            return lhs->vectorId < rhs->vectorId;
        }
        bool operator()( const std::shared_ptr<DataArray>& lhs, long vecId ) const
        {
            return lhs->vectorId < vecId;
        }
        bool operator()( long vecId, const std::shared_ptr<DataArray>& rhs ) const
        {
            return vecId < rhs->vectorId;
        }
    };
    std::set<std::shared_ptr<DataArray>, VectorCompare> DataArrays;

protected:
    /// Holds every unique timepoint in the data. For each timepoint, vectors of pointers to each valid data vector is present.
    TimeMap           TimeLine;
    TimeMap::iterator currentTime;
    TimeStamp         globalStartingTime;
    TimeStamp         globalEndingTime;

    /// Vector of indicators to add to every data vector that may be created
    std::vector<BTIndicator*> indicators;
    int                       timeLength;

    void processData();
    void findTimeInterval();

    /// @brief  Reads OHLCV data in a csv
    ///
    /// Data must contain a column labeled open, high, low, close, vol (or volume).
    /// Not case sensitive.
    /// Without any one of these columns, undefined behavior will occur.
    static void ReadCandleData( std::ifstream&, std::string&, std::list<DataStruct>& );

    /// @brief Reads a csv of option data
    ///
    /// Columns should be named as follows:
    /// timestamp, impVol, del, price, pvDiv, gam, veg, thet, und
    static void ReadOptionData( std::ifstream&, std::string&, std::list<DataStruct>& );

    /// @brief Reads a csv of snap data
    ///
    /// Columns should be named as follows in no particular order:
    /// timestamp, price, size
    static void ReadSnapData( std::ifstream&, std::string&, std::list<DataStruct>& );

    static std::shared_ptr<DataArray> readVector( const InputFile& );
    static std::shared_ptr<DataArray> readVector( const std::string& filepath );

    /// Prints all available data
    void printCSVs();
};