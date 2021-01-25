#include "Data.h"
#include "DataStruct.h"

using namespace std;
using namespace TradeBase;

TBIndicator::TBIndicator( int bufferSize )
{
    buffLength = bufferSize;
    buffer = vector<DataStruct>( bufferSize );
    buffPos = 0;
    init = 0;
    valid = false;
}

std::unique_ptr<TBIndicator> TBIndicator::clone() const
{
    return make_unique<TBIndicator>( *this );
}

void TBIndicator::update( const DataStruct& newPoint )
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

bool TBIndicator::isValid() const
{
    return valid;
}

double TBIndicator::getIndicator() const
{
    return currentValue;
}