#pragma once
#include "Strategy.h"
#include <memory>

class SMA;

/// @brief Buys when the SMAS is greater than the SMAL and sells when the SMAS is less than the SMAL
///
/// By default the long SMA (SMAL) is 20 units and the short (SMAS) is 5 days.
/// Each order is for 1 share.
class SMACrossover : public BTStrategy
{
public:
    SMACrossover() = default;
    ~SMACrossover() = default;
    void ProcessNextTick( const std::shared_ptr<BTAccount>& A, const std::shared_ptr<BTBroker>& B, const std::shared_ptr<BTData>& D, std::vector<std::pair<Contract, Order>>& );
};