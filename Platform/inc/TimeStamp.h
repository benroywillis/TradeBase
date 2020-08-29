#pragma once
#include <string>

/// Structure for holding time information
class TimeStamp
{
public:
    int    year;
    int    month;
    int    day;
    int    hour;
    int    minute;
    double second;
    TimeStamp();
    /// TimeStamp object for the time of construction
    TimeStamp( bool );
    TimeStamp( const std::string&, bool exprDate = false );
    bool        operator==( const TimeStamp& ) const;
    bool        operator!=( const TimeStamp& ) const;
    bool        operator<( const TimeStamp& ) const;
    bool        operator>( const TimeStamp& ) const;
    bool        valid() const;
    void        clear();
    std::string toString( bool fileOutput = false, bool dateOnly = false ) const;
};