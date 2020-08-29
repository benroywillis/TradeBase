#pragma once
#include "Indicator.h"

struct CandleStruct;

/// @brief  Simple Moving Average indicator
///
/// Averages the past bufferSize data values together.
class SMA : public BTIndicator
{
    friend DataArray;

public:
    SMA( int );
    void                         update( const DataStruct& ) override;
    std::unique_ptr<BTIndicator> clone() const override;
};
