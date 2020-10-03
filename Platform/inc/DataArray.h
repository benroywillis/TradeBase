#pragma once
#include "Contract.h"
#include "DataStruct.h"
#include "Indicator.h"
#include <optional>

/// Structure for holding each unique data array
class DataArray : public BackTrader
{
public:
    int                                       length;
    long                                      vectorId;
    Contract                                  contract;
    TimeStamp                                 exprDate;
    std::string                               interval;
    std::list<DataStruct>                     array;
    std::vector<std::unique_ptr<BTIndicator>> Inds;
    DataArray() = default;
    DataArray( long, long, const std::string&, const std::string&, const std::string&, const std::string&, const std::string& );
    DataArray( const DataArray& ) = delete;
    ~DataArray() = default;
    bool operator<( const DataArray& rhs ) const
    {
        return ( contract.conId < rhs.contract.conId ) && ( interval < rhs.interval );
    }
    bool operator==( const DataArray& rhs ) const
    {
        return ( contract.conId == rhs.contract.conId ) && ( interval == rhs.interval );
    }
    void                                                 addPoint( CandleStruct& );
    void                                                 addPoint( SnapStruct& );
    void                                                 addPoint( OptionStruct& );
    std::optional<std::list<DataStruct>::const_iterator> getLastPoint() const;

    void        addIndicator( BTIndicator* );
    bool        validIndicators() const;
    std::string getStart( bool fileOutput = false ) const;
    TimeStamp   getStartingTime() const;
    std::string getEnd( bool fileOutput = false ) const;
    TimeStamp   getEndingTime( bool fileOutput = false ) const;
};