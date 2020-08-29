#include "SMA.h"
#include "DataStruct.h"

using namespace std;

SMA::SMA( int bufferSize ) : BTIndicator( bufferSize )
{
}

std::unique_ptr<BTIndicator> SMA::clone() const
{
    return make_unique<SMA>( *this );
}

void SMA::update( const DataStruct& newPoint )
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
    currentValue = 0;
    for( auto& index : buffer )
    {
        currentValue += index.get() / buffLength;
    }
}