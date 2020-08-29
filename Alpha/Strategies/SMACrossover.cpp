#include "SMACrossover.h"
#include "Contract.h"
#include "Data.h"
#include "DataTypes.h"
#include "Execution.h"
#include "Order.h"
#include "Position.h"
#include "SMA.h"
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std;

/// In this strategy, the short SMA is the first indicator index
/// So the one whose valid needs to be checked is index 1
void SMACrossover::ProcessNextTick( const shared_ptr<BTAccount>& A, const shared_ptr<BTBroker>& B, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
{
    for( const auto& point : D->getCurrentPoint().Points )
    {
        bool buySignal = false;
        bool sellSignal = false;

        if( point.p_Vector->validIndicators() )
        {
            buySignal = point.p_Vector->Inds[0]->currentValue > point.p_Vector->Inds[1]->currentValue;
            sellSignal = point.p_Vector->Inds[1]->currentValue > point.p_Vector->Inds[0]->currentValue;
        }
        auto tradeContract = point.p_Vector->contract;
        if( buySignal )
        {
            auto tradeOrder = getOrder( true );
            trades.emplace_back( pair( tradeContract, tradeOrder ) );
        }
        if( sellSignal )
        {
            auto tradeOrder = getOrder( false );
            trades.emplace_back( pair( tradeContract, tradeOrder ) );
        }
    }
}