#pragma once
#include "BackTrader.h"
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace TradeBase
{
    class Position;
    class TBData;
    struct Order;
    struct Contract;
    struct OrderCompare;
    struct positionCompare;
    /// @brief Parent class of all strategies
    ///
    /// Not meant to be constructed.
    /// Its template should be used for every strategy developed.
    class TBStrategy : public BackTrader
    {
    public:
        TBStrategy() = default;
        ~TBStrategy() = default;
        virtual void ProcessNextTick( const std::set<Position*, positionCompare>&, const std::set<Order, OrderCompare>&, const std::shared_ptr<TBData>&, std::vector<std::pair<Contract, Order>>& )
        {
        }
    };
} // namespace TradeBase