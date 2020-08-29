#pragma once
#include "BackTrader.h"
#include "Order.h"
#include <memory>

class BTAccount;
class BTBroker;
class BTStrategy;
class BTData;

class BTBrain : public BackTrader
{
public:
    /// Constructor for most use cases
    BTBrain( double startCash, const std::shared_ptr<BTData>& newData, const std::shared_ptr<BTStrategy>& newStrategy );
    /// Constructor for passing custom commission scheme
    BTBrain( double startCash, const std::shared_ptr<BTBroker>& newBroker, const std::shared_ptr<BTData>& newData, const std::shared_ptr<BTStrategy>& newStrategy );
    /// Constructor for passing a custom account
    BTBrain( const std::shared_ptr<BTAccount>& newAccount, const std::shared_ptr<BTData>& newData, const std::shared_ptr<BTStrategy>& newStrategy );
    /// Constructor for passing a broker with a custom commission scheme
    BTBrain( const std::shared_ptr<BTAccount>& newAccount, const std::shared_ptr<BTBroker>& newBroker, const std::shared_ptr<BTData>& newData, const std::shared_ptr<BTStrategy>& newStrategy );
    ~BTBrain() = default;
    /// @brief  Facilitates backtesting
    void run();
    /// @brief
    std::shared_ptr<BTAccount> Account;
    /// @brief
    std::shared_ptr<BTBroker> Broker;
    /// @brief  Data to be used in the backtest.
    /// @todo   Needs to be vectorized.
    std::shared_ptr<BTData> Data;
    /// @brief  Strategy that will be implemented in the backtest.
    /// @todo   Needs to be vectorized.
    std::shared_ptr<BTStrategy> Strategy;
};