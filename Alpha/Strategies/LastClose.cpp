#include "LastClose.h"
#include "Account.h"
#include "Broker.h"
#include "Contract.h"
#include "Data.h"
#include "DataStruct.h"
#include "DataTypes.h"
#include "Execution.h"
#include "Order.h"
#include "Position.h"
#include <spdlog/spdlog.h>

using namespace std;

void LastClose::ProcessNextTick( const shared_ptr<BTAccount>& A, const shared_ptr<BTBroker>& B, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
{
    for( const auto& point : D->getCurrentPoint().Points )
    {
        auto prevPoint = D->getPreviousVectorPoint( point.p_Vector->vectorId );
        if( !prevPoint )
        {
            return;
        }
        double price = point.p_Point->get();
        double prevPrice = prevPoint->p_Point->get();
        bool   buySignal = price > prevPrice;
        bool   sellSignal = price < prevPrice;
        auto   tradeContract = point.p_Vector->contract;
        if( buySignal )
        {
            auto tradeOrder = getOrder( true );
            spdlog::info( "Buying 1 share at " + tradeContract.symbol );
            trades.emplace_back( pair( tradeContract, tradeOrder ) );
        }
        if( sellSignal )
        {
            auto tradeOrder = getOrder( false );
            spdlog::info( "Selling 1 share " + tradeContract.symbol );
            trades.emplace_back( pair( tradeContract, tradeOrder ) );
        }
    }
}