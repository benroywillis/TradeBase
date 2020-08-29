#pragma once
#include "BackTrader.h"
#include "Order.h"
#include <memory>
#include <optional>
#include <vector>

class Position;
class BTAccount;
class BTBroker;
class BTData;

struct CandleStruct;
class DataArray;

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
    virtual void ProcessNextTick( const std::shared_ptr<BTAccount>& A, const std::shared_ptr<BTBroker>& B, const std::shared_ptr<BTData>& D, std::vector<std::pair<Contract, Order>>& );

protected:
    /// @brief  Generates an order for the new trade
    ///
    /// This method will map the strategy being employed to an order the IB API can understand
    static Order getOrder( bool );
};