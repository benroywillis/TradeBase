#pragma once
#include "Data.h"
#include <deque>
#include <memory>
#include <variant>
#include <vector>

struct Order;
struct Execution;
struct Contract;

class Position
{
public:
    Position( const Contract&, const Order&, const Execution& );
    Position() = default;
    ~Position() = default;
    bool             longPosition() const;
    const Contract*  getContract() const;
    Order&           getLastOrder();
    const Order&     getLastOrder() const;
    const Execution& getFirstExecution() const;
    Execution&       getLastExecution();
    const Execution& getLastExecution() const;
    double           getAvgPrice() const;
    double           getPositionSize() const;
    double           getPnL() const;
    double           getCashValue() const;
    double           getCashValue( const GlobalTimePoint& );

    void addOrder( const Order& );
    void addExecution( const Execution& );
    void addData( const std::vector<std::shared_ptr<DataArray>>& );
    void addData( const std::shared_ptr<DataArray>& );

    /// Updates both realized and unrealized PnL
    double updatePnL( double );
    double updateUPnL( const GlobalTimePoint& );

    /// @brief Update the queue of executions
    ///
    /// When transacting shares on a position, this method keeps track of the order of executions for calculation of the PnL of this position.
    /// The policy when calculating PnL is to match sell orders with the oldest buy order, "First In First Out".
    /// Since the execution queue cannot be iterated through, the avgPrice and positionSize parameters are updated every time the queue is updated.
    void updateQueue( Execution& );

private:
    std::shared_ptr<Contract> contract;
    void                      calcAvgPrice();
    /// Queue to hold executions of shared for this position. Used to calculate realized PnL
    std::deque<Execution> positionExecutions;
    std::vector<Order>    orderHistory;
    double                positionSize;
    double                avgPrice;
    double                currentPrice;
    double                PnL;
    double                UPnL;
};
