#include "HalvedPositionSMA.h"
#include "Account.h"
#include "Broker.h"
#include "Data.h"
#include "DataStruct.h"
#include "Execution.h"
#include "Order.h"
#include "SMA.h"
#include <spdlog/spdlog.h>

using namespace std;

constexpr int POSITIONSIZE = 1;

void HPSMA::ProcessNextTick( const shared_ptr<BTAccount>& A, const shared_ptr<BTBroker>& B, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
{
    auto time = D->getCurrentTime();
    for( const auto& point : D->getCurrentPoint().Points )
    {
        bool buySignal = false;
        bool sellSignal = false;

        // check if there is already an open order for this contract
        auto comp = pair( point.p_Vector->contract, Order() );
        if( B->openOrders.find( comp ) != B->openOrders.end() )
        {
            continue;
        }
        for( const auto& pa : B->openOrders )
        {
            spdlog::info( "Open order for symbol " + pa.first.symbol + ", secType " + pa.first.secType + ", conId " + to_string( pa.first.conId ) + " right " + pa.first.right + " strike " + to_string( pa.first.strike ) + " expiry " + pa.first.lastTradeDateOrContractMonth );
            if( pa.first.conId == point.p_Vector->contract.conId )
            {
                continue;
            }
        }

        // Find a position in the account that already exists for this contract
        Position* existingPos = nullptr;
        string    side = "NEUTRAL";
        for( const auto& pos : A->getPositions() )
        {
            if( pos->getContract()->conId == point.p_Vector->contract.conId )
            {
                existingPos = pos;
                side = pos->getFirstExecution().side;
                break;
            }
        }

        if( point.p_Vector->validIndicators() )
        {
            buySignal = point.p_Vector->Inds[0]->currentValue > point.p_Vector->Inds[1]->currentValue;
        }
        auto tradeContract = point.p_Vector->contract;
        // if our signals are buy and we have either no position or a short position on this contract, buy back all the shorts, if any, and go long 5 shares
        if( buySignal && side != "BOT" )
        {
            Order tradeOrder;
            if( side == "SLD" )
            {
                tradeOrder = getOrder( true, POSITIONSIZE * 2 );
                spdlog::info( "Buying " + to_string( POSITIONSIZE * 2 ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
            else if( side == "NEUTRAL" )
            {
                tradeOrder = getOrder( true, POSITIONSIZE );
                spdlog::info( "Buying " + to_string( POSITIONSIZE ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
        }
        if( existingPos != nullptr )
        {
            if( !buySignal )
            {
                sellSignal = ( point.p_Vector->Inds[0]->currentValue < point.p_Vector->Inds[1]->currentValue ) || ( existingPos->getAvgPrice() < point.p_Point->get() * 0.9 ) || ( existingPos->getAvgPrice() * 1.2 > point.p_Point->get() );
            }
            if( sellSignal && side == "BOT" )
            {
                spdlog::info( "Selling " + to_string( POSITIONSIZE * 2 ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) + " when the avg price is " + to_string( existingPos->getAvgPrice() ) );
                auto tradeOrder = getOrder( false, POSITIONSIZE * 2 );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
            else if( sellSignal && side == "NEUTRAL" )
            {
                spdlog::info( "Selling " + to_string( POSITIONSIZE ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) + " when the avg price is " + to_string( existingPos->getAvgPrice() ) );
                auto tradeOrder = getOrder( false, POSITIONSIZE );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
        }
    }
}

Order HPSMA::getOrder( bool buy, int quantity )
{
    // generate the most generic order for the contract as possible
    auto newOrder = Order();
    if( buy )
    {
        newOrder.action = "BUY";
    }
    else
    {
        newOrder.action = "SELL";
    }
    newOrder.orderType = "MKT";
    /// When submitting to IB, this quantity cannot be negative
    newOrder.totalQuantity = quantity;
    newOrder.tif = "DAY"; // only valid on this trading day
    newOrder.orderId = getNextOrderId();
    return newOrder;
}
