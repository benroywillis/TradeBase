#pragma once
#include "BackTrader.h"
#include "Contract.h"
#include "Data.h"
#include "Position.h"
#include <algorithm>
#include <set>

class BTAccount : BackTrader
{
public:
    /// Default constructor for derived classes
    BTAccount();
    /// Meant for backtesting
    BTAccount( double, bool Margin = false );
    ~BTAccount();
    void update( const Position& );
    /// @brief Updates the balances in the account.
    ///
    /// Primarily used by the broker.
    /// Returns true if the update to the cash results in valid balances of the account funds.
    /// Enforces margin requirements as well.
    bool                   updateCash( const Contract&, const Order&, double );
    double                 getCash() const;
    double                 getMarginLoan() const;
    std::vector<Position*> getPositions() const;
    double                 getPnL() const;
    double                 getUPnL() const;
    void                   updatePnLs( const GlobalTimePoint& );

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

    /// @brief  Holds all positions in the account.
    std::set<Position*, positionCompare> positions;
    bool                                 margin;

protected:
    bool checkMaintenanceMargin( double, double ) const;
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