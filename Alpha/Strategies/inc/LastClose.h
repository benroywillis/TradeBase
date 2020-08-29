#pragma once
#include "Strategy.h"

/// @brief Buys if the last close was greater than the current, and sells if the previous close was less than the current
///
/// Interestingly, this strategy is surprisingly hard to beat (if there are no commission fees).
/// If you can consistently beat this strategy in a backtest, you have a decent strategy
class LastClose : public BTStrategy
{
public:
    LastClose() = default;
    /// @brief Runs private methods on new data
    void ProcessNextTick( const std::shared_ptr<BTAccount>& A, const std::shared_ptr<BTBroker>& B, const std::shared_ptr<BTData>& D, std::vector<std::pair<Contract, Order>>& ) override;
};