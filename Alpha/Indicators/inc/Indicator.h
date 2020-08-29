#pragma once
#include "BackTrader.h"
#include <list>
#include <memory>
#include <vector>

struct DataStruct;
class DataArray;

class BTIndicator : public BackTrader
{
    friend class DataArray;

public:
    BTIndicator( int bufferSize );
    virtual ~BTIndicator() = default;
    /// Updates the indicator with a new value
    virtual void update( const DataStruct& );
    /// Returns the current value of the indicator
    double getIndicator() const;
    /// @brief Indicates whether the indicator contains a valid value
    ///
    /// An indicator is not valid when its buffer has not been fully initialized yet
    bool   isValid() const;
    double currentValue;

    /// Cloning methods to preserve polymorphism
    virtual std::unique_ptr<BTIndicator> clone() const;

protected:
    /// Most recent update of the underlying contract price
    std::vector<DataStruct> buffer;
    /// Length of memory for this indicator
    int buffLength;
    /// @brief  Index for the most outdated value
    ///
    /// The buffer is treated like a circular buffer, and therefore the line changes position, but those in line don't.
    /// Instead, they are replaced, and the next one to be replaced is pointed to by this index.
    int buffPos;
    /// Tracker of the amount of spaces in buffLength have been written to this far. Controls the valid flag.
    int init;
    /// Indicates whether the buffer has been fully initialized yet
    bool valid;
};