#pragma once
#include "BackTrader.h"
#include "CommissionSchemes.h"
#include "Order.h"
#include "Position.h"

class BTStrategy;
class BTData;

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

struct positionCompare
{
    using is_transparent = void;
    bool operator()( const Position* lhs, const Position* rhs ) const
    {
        return lhs->getContract()->conId < rhs->getContract()->conId;
    }
    bool operator()( const Position* lhs, long rhs ) const
    {
        return lhs->getContract()->conId < rhs;
    }
    bool operator()( long lhs, const Position* rhs ) const
    {
        return lhs < rhs->getContract()->conId;
    }
};

struct Configure
{
    const CommissionScheme* scheme;
    bool                    margin;
    Configure() = default;
    Configure( const CommissionScheme* newScheme, bool m )
    {
        scheme = newScheme;
        margin = m;
    }
    ~Configure() = default;
};

class BTBrain : public BackTrader
{
public:
    /// Constructor for most use cases
    BTBrain( const std::shared_ptr<BTData>&, const std::shared_ptr<BTStrategy>&, const Configure& );
    ~BTBrain() = default;
    /// @brief  Facilitates backtesting
    void   run();
    double getTotalCommission() const;
    /// @brief  Processes the input order
    ///
    /// Checks cash, exchange, margin requirements, etc. If the order is filled, the returned pointer is valid. Else the pointer is Null
    std::vector<Position>  processTrade( const std::pair<Contract, Order>& );
    std::vector<Position*> getPositions() const;
    double                 getCash() const;
    double                 getMarginLoan() const;
    double                 getPnL() const;
    double                 getUPnL() const;

    std::shared_ptr<BTData> Data;
    /// @brief  Strategy that will be implemented in the backtest.
    /// @todo   Needs to be vectorized.
    std::shared_ptr<BTStrategy> Strategy;

private:
    void                                 update( const Position& );
    std::optional<double>                getPrice( const Contract& );
    std::optional<Execution>             Execute( const Contract&, const Order&, double );
    bool                                 updateCash( const Contract&, const Order&, double );
    void                                 updatePnLs( const GlobalTimePoint& );
    bool                                 checkMaintenanceMargin( double, double ) const;
    std::set<Position*, positionCompare> positions;
    const CommissionScheme*              scheme;
    double                               totalCommission;
    /// @brief  Holds all positions in the account.
    bool margin;
    /// Equity within the account. Used for buying power calculations
    double accountEquity;
    /// Amount of equity at the start of the test
    double startEquity;
    /// @brief  Current cash amount in the account
    double cash;
    /// Amount of money our securities are worth, total
    double positionsValue;
    /// Amount of open margin loan in the account
    double marginLoan;
    /// Amount of borrowed money for opening long positions
    double longMarginLoan;
    /// Amount of margin for opening short positions
    double shortMarginLoan;
    /// Controls how much equity must be maintained in a margin account
    /// IB does not have a per-contract maintenance margin requirement, so only the account is necessary
    double maintenanceMargin;
    /// PnL = cash + CostBases - startCash
    double PnL;
    /// UPnL = PositionValues - CostBases
    double UPnL;
};