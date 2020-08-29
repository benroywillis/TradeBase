#pragma once
#include "BackTrader.h"
#include "Contract.h"
#include "Order.h"
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

struct Execution;
struct Order;
struct Contract;
struct CommissionScheme;
class BTData;
class BTAccount;
class Position;

/// Structure only considers conId because orderIds will always be unique
struct OrderCompare
{
    using is_transparent = void;
    bool operator()( const std::pair<Contract, Order>& lhs, const std::pair<Contract, Order>& rhs ) const
    {
        return lhs.first.conId < rhs.first.conId;
    }
    bool operator()( const std::pair<Contract, Order>& lhs, long rhs ) const
    {
        return lhs.first.conId < rhs;
    }
    bool operator()( long lhs, const std::pair<Contract, Order>& rhs ) const
    {
        return lhs < rhs.first.conId;
    }
};

class BTBroker : public BackTrader
{
    friend class BTBrain;

public:
    BTBroker();
    BTBroker( std::shared_ptr<CommissionScheme> );
    ~BTBroker() = default;
    double getTotalCommission() const;
    /// @brief  Processes the input order
    ///
    /// Checks cash, exchange, margin requirements, etc. If the order is filled, the returned pointer is valid. Else the pointer is Null
    std::vector<Position> processTrade( const std::shared_ptr<BTAccount>&, const std::shared_ptr<BTData>&, const std::pair<Contract, Order>& );
    /// Maps a conId to a vector of open orders that may need execution
    /// When servicing callbacks for this orderId, this maps those to a contract
    std::map<long, std::pair<Contract, Order>> orderMap;
    /// Contains all orders that have been accepted by the broker but have not filled completely
    std::set<std::pair<Contract, Order>, OrderCompare> openOrders;

private:
    static std::optional<double>             getPrice( const std::shared_ptr<BTData>&, const Contract& );
    std::optional<Execution>                 Execute( const std::shared_ptr<BTAccount>&, const Contract&, const Order&, double );
    struct std::shared_ptr<CommissionScheme> scheme;
    double                                   totalCommission;
};