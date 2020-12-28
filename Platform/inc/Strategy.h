#pragma once
#include "BackTrader.h"
#include "Brain.h"
#include <memory>
#include <optional>
#include <vector>

class Position;
class BTData;
struct Order;
struct Contract;

/// @brief Parent class of all strategies
///
/// Not meant to be constructed.
/// Its template should be used for every strategy developed.
class BTStrategy : public BackTrader
{
public:
    BTStrategy() = default;
    ~BTStrategy() = default;
    virtual void ProcessNextTick( const std::set<Position*, positionCompare>&, const std::set<Order, OrderCompare>&, const std::shared_ptr<BTData>&, std::vector<std::pair<Contract, Order>>& )
    {
    }
};