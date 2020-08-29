#pragma once
#include "SMACrossover.h"

class BTAccount;
class BTBroker;

/// @brief  Half priced simple moving average
///
/// Buys a position (of size 1) whenever the 5-day SMA is greater than the 20-day SMA.
/// Sells all shared of a position when the total value of the position is 90% of its cost basis or 120% of its cost basis.
class HPSMA : public SMACrossover
{
private:
    /// Used to access the price of positions
    static Order getOrder( bool, int );

public:
    HPSMA() = default;
    ~HPSMA() = default;

    /// Overrides ProcessNextTick in SMACrossover
    void ProcessNextTick( const std::shared_ptr<BTAccount>& A, const std::shared_ptr<BTBroker>& B, const std::shared_ptr<BTData>& D, std::vector<std::pair<Contract, Order>>& );
};