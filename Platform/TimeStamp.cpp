#include "TimeStamp.h"
#include <chrono>
#include <ctime>

using namespace std;
using namespace TradeBase;

TimeStamp::TimeStamp()
{
    year = -1;
    month = -1;
    day = -1;
    hour = -1;
    minute = -1;
    second = -1;
}

/// TimeStamp object for the time of construction
TimeStamp::TimeStamp( bool precise )
{
    auto   now = chrono::system_clock::now();
    auto   now_time = chrono::system_clock::to_time_t( now );
    string timePoint = ctime( &now_time );
    // string is in Www Mmm dd hh:mm:ss yyyy format
    // Www -> weekday ("Mon"), Mmm -> month ("Oct")
    string monthstr = timePoint.substr( 4, 3 );
    string daystr = timePoint.substr( 8, 2 );
    string timestr = timePoint.substr( 11, 8 );
    string yearstr = timePoint.substr( 20, 4 );

    year = stoi( yearstr );
    if( monthstr == "Jan" )
    {
        month = 1;
    }
    else if( monthstr == "Feb" )
    {
        month = 2;
    }
    else if( monthstr == "Mar" )
    {
        month = 3;
    }
    else if( monthstr == "Apr" )
    {
        month = 4;
    }
    else if( monthstr == "May" )
    {
        month = 5;
    }
    else if( monthstr == "Jun" )
    {
        month = 6;
    }
    else if( monthstr == "Jul" )
    {
        month = 7;
    }
    else if( monthstr == "Aug" )
    {
        month = 8;
    }
    else if( monthstr == "Sep" )
    {
        month = 9;
    }
    else if( monthstr == "Oct" )
    {
        month = 10;
    }
    else if( monthstr == "Nov" )
    {
        month = 11;
    }
    else if( monthstr == "Dec" )
    {
        month = 12;
    }
    else
    {
        month = 0;
    }
    day = stoi( daystr );

    size_t sep = timestr.find( ':' );
    string hourstr = timestr.substr( 0, sep );
    string minutestr = timestr.substr( sep + 1, 2 );
    sep = timestr.find( ':', sep + 1 );
    string secondstr = timestr.substr( sep + 1, 2 );
    hour = stoi( hourstr );
    minute = stoi( minutestr );
    second = stof( secondstr );
    /*if( precise )
    {
        // retrieve our seconds in double form
        // TODO: this doesn't work because 1.) the begin time is not the second in integer form and 2.) milliseconds is too large of a unit
        auto t_2 = chrono::system_clock::now();
        auto diff = chrono::duration_cast<chrono::milliseconds>( t_2 - now).count();
        second += (double);
    }*/

} /// IB historical data constructor

TimeStamp::TimeStamp( const string& barTime, bool exprDate )
{
    // TODO: futures expiration dates don't need to include the day, so this needs to be generalized for that
    /// time format is yyyymmdd  hh:mm:ss (one white space [in the case of input csv files] or two white spaces [in the case of historical bar data from IB])
    // find # of white spaces (can be 1 or two)
    size_t whitespace = barTime.find( ' ' );
    size_t whitespace2 = barTime.find( ' ', whitespace + 1 );
    size_t spaceOffset;
    if( whitespace2 == string::npos )
    {
        spaceOffset = 1;
    }
    else
    {
        spaceOffset = 2;
    }
    string date = barTime.substr( 0, whitespace );
    string yearstr = date.substr( 0, 4 );
    string monthstr = date.substr( 4, 2 );
    string daystr = date.substr( 6, 2 );
    year = stoi( yearstr );
    month = stoi( monthstr );
    day = stoi( daystr );

    if( exprDate )
    {
        // time information is not included
        hour = 16;
        minute = 0;
        second = 0.0;
    }
    else
    {
        string time = barTime.substr( whitespace + spaceOffset, string::npos );
        size_t sep = time.find( ':' );
        string hourstr = time.substr( 0, sep );
        string minutestr = time.substr( sep + 1, 2 );
        sep = time.find( ':', sep + 1 );
        string secondstr = time.substr( sep + 1, 2 );
        hour = stoi( hourstr );
        minute = stoi( minutestr );
        second = stof( secondstr );
    }
}

bool TimeStamp::operator==( const TimeStamp& rhs ) const
{
    return ( year == rhs.year ) && ( month == rhs.month ) && ( day == rhs.day ) && ( hour == rhs.hour ) && ( minute == rhs.minute ) && ( second == rhs.second );
}

bool TimeStamp::operator!=( const TimeStamp& rhs ) const
{
    return ( year != rhs.year ) || ( month != rhs.month ) || ( day != rhs.day ) || ( hour != rhs.hour ) || ( minute != rhs.minute ) || ( second != rhs.second );
}

bool TimeStamp::operator<( const TimeStamp& rhs ) const
{
    if( year < rhs.year )
    {
        return true;
    }
    if( year > rhs.year )
    {
        return false;
    }
    if( month < rhs.month )
    {
        return true;
    }
    if( month > rhs.month )
    {
        return false;
    }
    if( day < rhs.day )
    {
        return true;
    }
    if( day > rhs.day )
    {
        return false;
    }
    if( hour < rhs.hour )
    {
        return true;
    }
    if( hour > rhs.hour )
    {
        return false;
    }
    if( minute < rhs.minute )
    {
        return true;
    }
    if( minute > rhs.minute )
    {
        return false;
    }
    if( second < rhs.second )
    {
        return true;
    }
    return false;
}

bool TimeStamp::operator>( const TimeStamp& rhs ) const
{
    if( year < rhs.year )
    {
        return false;
    }
    if( year > rhs.year )
    {
        return true;
    }
    if( month < rhs.month )
    {
        return false;
    }
    if( month > rhs.month )
    {
        return true;
    }
    if( day < rhs.day )
    {
        return false;
    }
    if( day > rhs.day )
    {
        return true;
    }
    if( hour < rhs.hour )
    {
        return false;
    }
    if( hour > rhs.hour )
    {
        return true;
    }
    if( minute < rhs.minute )
    {
        return false;
    }
    if( minute > rhs.minute )
    {
        return true;
    }
    if( second < rhs.second )
    {
        return false;
    }
    return true;
}

bool TimeStamp::valid() const
{
    return ( year != -1 ) && ( month != 1 ) && ( day != -1 ) && ( hour != -1 ) && ( minute != -1 ) && ( second != -1 );
}

void TimeStamp::clear()
{
    year = -1;
    month = -1;
    day = -1;
    hour = -1;
    minute = -1;
    second = -1;
}

string TimeStamp::toString( bool fileOutput, bool dateOnly ) const
{
    string daystr = day < 10 ? "0" + to_string( day ) : to_string( day );
    string monthstr = month < 10 ? "0" + to_string( month ) : to_string( month );
    if( fileOutput )
    {
        return to_string( year ) + monthstr + daystr + "_" + to_string( hour ) + ":" + to_string( minute ) + ":" + to_string( (int)second );
    }
    if( dateOnly )
    {
        return to_string( year ) + monthstr + daystr;
    }
    return to_string( year ) + monthstr + daystr + " " + to_string( hour ) + ":" + to_string( minute ) + ":" + to_string( second );
}