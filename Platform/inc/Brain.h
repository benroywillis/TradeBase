#pragma once
#include "BackTrader.h"
#include "CommissionSchemes.h"
#include "Order.h"
#include "Position.h"

namespace TradeBase
{
    class TBStrategy;
    class TBData;

    /// Structure only considers conId because orderIds will always be unique
    struct OrderCompare
    {
        using is_transparent = void;
        bool operator()( const Order& lhs, const Order& rhs ) const
        {
            return lhs.conId < rhs.conId;
        }
        bool operator()( const Order& lhs, const Contract& rhs ) const
        {
            return lhs.conId < rhs.conId;
        }
        bool operator()( const Contract& lhs, const Order& rhs ) const
        {
            return lhs.conId < rhs.conId;
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

    class TBBrain : public BackTrader
    {
    public:
        /// Constructor for most use cases
        TBBrain( const std::shared_ptr<TBData>&, const std::shared_ptr<TBStrategy>&, const Configure& );
        ~TBBrain() = default;
        /// @brief  Facilitates backtesting
        void   run();
        double getTotalCommission() const;
        /// @brief  Processes the input order
        ///
        /// Checks cash, exchange, margin requirements, etc. If the order is filled, the returned pointer is valid. Else the pointer is Null
        std::vector<Position>          processTrade( const std::pair<Contract, Order>& );
        std::vector<Position*>         getPositions() const;
        double                         getPnL() const;
        double                         getUPnL() const;
        double                         getMTMChange() const;
        double                         getMaximumGain() const;
        double                         getMaximumDrawdown() const;
        const std::vector<double>&     getMTMHistory() const;
        const std::vector<std::string> getTimeVector() const;

        std::shared_ptr<TBData> Data;
        /// @brief  Strategy that will be implemented in the backtest.
        /// @todo   Needs to be vectorized.
        std::shared_ptr<TBStrategy> Strategy;

    private:
        void                                 update( const Position& );
        std::optional<double>                getPrice( const Contract& );
        std::optional<Execution>             Execute( const Contract&, const Order&, double );
        void                                 MarkToMarket( const GlobalTimePoint& );
        std::set<Position*, positionCompare> positions;
        const CommissionScheme*              scheme;
        double                               maxGain;
        double                               maxDrawdown;
        double                               totalCommission;
        /// PnL = cash + CostBases - startCash
        double PnL;
        /// UPnL = PositionValues - CostBases
        double UPnL;
        /// MarkedToMarket Change
        double MTMChange;
        /// MarkedToMarket History
        std::vector<double> MTMHistory;
    };
} // namespace TradeBase