#include "Strategy.h"
#include "Account.h"
#include "Broker.h"
#include "Contract.h"
#include "Data.h"
#include "Execution.h"
#include "Order.h"
#include "Position.h"

using namespace std;

void BTStrategy::ProcessNextTick( const shared_ptr<BTAccount>& A, const shared_ptr<BTBroker>& B, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
{
    for( const auto& vec : D->DataArrays )
    {
        auto tradeContract = vec->contract;
        auto tradeOrder = getOrder( true );
        trades.emplace_back( pair( tradeContract, tradeOrder ) );
    }
}

Order BTStrategy::getOrder( bool buy )
{
    // generate the most generic order for the contract as possible
    auto newOrder = Order();
    newOrder.orderType = "MKT";
    newOrder.totalQuantity = 1;
    if( buy )
    {
        newOrder.action = "BUY";
    }
    else
    {
        newOrder.action = "SELL";
    }
    newOrder.tif = "DAY"; // only valid on this trading day
    newOrder.orderId = getNextOrderId();
    return newOrder;
}