#include "Data.h"
#include "DataStruct.h"

using namespace std;
using namespace TradeBase;

BTIndicator::BTIndicator( int bufferSize )
{
    buffLength = bufferSize;
    buffer = vector<DataStruct>( bufferSize );
    buffPos = 0;
    init = 0;
    valid = false;
}

std::unique_ptr<BTIndicator> BTIndicator::clone() const
{
    return make_unique<BTIndicator>( *this );
}

void BTIndicator::update( const DataStruct& newPoint )
{
    buffPos = buffPos % buffLength;
    buffer[buffPos] = newPoint;
    buffPos++;
    if( !valid )
    {
        init++;
        if( init == buffLength )
        {
            valid = true;
        }
    }
}

bool BTIndicator::isValid() const
{
    return valid;
}

double BTIndicator::getIndicator() const
{
    return currentValue;
}