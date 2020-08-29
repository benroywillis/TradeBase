#include "Broker.h"
#include "Account.h"
#include "CommissionSchemes.h"
#include "Data.h"
#include "DataStruct.h"
#include "Execution.h"
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std;

/// By default the scheme is set to IBs "fixed price" options and "american exchange" discount for STKs and ETFs (NYSE, AMEX, NASDAQ)
BTBroker::BTBroker()
{
    scheme = make_shared<CommissionScheme>( CommissionScheme() );
    totalCommission = 0;
}

BTBroker::BTBroker( shared_ptr<CommissionScheme> newScheme ) : scheme( move( newScheme ) ), totalCommission( 0 )
{
}

double BTBroker::getTotalCommission() const
{
    return totalCommission;
}

vector<Position> BTBroker::processTrade( const shared_ptr<BTAccount>& A, const shared_ptr<BTData>& D, const pair<Contract, Order>& p )
{
    vector<Position> returnVec;
    auto             con = p.first;
    auto             order = p.second;
    auto             currentPrice = getPrice( D, con );
    if( currentPrice )
    {
        if( order.orderType == "LMT" )
        {
            openOrders.insert( p );
        }
        else if( order.orderType == "MKT" )
        {
            double costBasis = currentPrice.value() * order.totalQuantity;
            auto   newExec = Execute( A, con, order, costBasis );
            if( newExec )
            {
                returnVec.emplace_back( Position( con, order, newExec.value() ) );
            }
        }
        else
        {
            spdlog::warn( "Orders not of type MKT or LMT are not supported." );
        }
        for( const auto& pa : openOrders )
        {
            if( pa.second.lmtPrice < currentPrice )
            {
                double costBasis = currentPrice.value() * pa.second.totalQuantity;
                auto   newExec = Execute( A, pa.first, pa.second, costBasis );
                if( newExec )
                {
                    returnVec.emplace_back( Position( pa.first, pa.second, newExec.value() ) );
                }
            }
        }
        return returnVec;
    }
    // there is not current price for this contract
    openOrders.insert( p );
    return vector<Position>();
}

optional<Execution> BTBroker::Execute( const shared_ptr<BTAccount>& A, const Contract& cont, const Order& ord, double costBasis )
{
    double newCostBasis;
    double tradeCommission;
    if( cont.secType == "OPT" )
    {
        tradeCommission = scheme->perContract( costBasis / ord.totalQuantity, ord.totalQuantity );
        newCostBasis = costBasis + tradeCommission;
    }
    else if( cont.secType == "STK" )
    {
        tradeCommission = scheme->perShare( ord.totalQuantity );
        newCostBasis = costBasis + tradeCommission;
    }
    else
    {
        cout << cont.secType << endl;
        spdlog::error( "The broker can only accept secTypes of STK and OPT." );
        tradeCommission = 0.0;
        newCostBasis = costBasis;
    }
    // update total commission cost for the backtest
    totalCommission += tradeCommission;
    if( A->updateCash( cont, ord, newCostBasis ) )
    {
        auto newExec = make_optional<Execution>();
        newExec->avgPrice = newCostBasis / ord.totalQuantity;
        newExec->price = newExec->avgPrice;
        newExec->shares = ord.totalQuantity;
        newExec->orderId = ord.orderId;
        newExec->cumQty = ord.totalQuantity;
        newExec->clientId = ClientID;
        newExec->side = ord.action == "BUY" ? "BOT" : "SLD";
        return newExec;
    }
    return nullopt;
}

optional<double> BTBroker::getPrice( const shared_ptr<BTData>& D, const Contract& con )
{
    auto point = D->lookup( con.conId );
    if( point )
    {
        return point->p_Point->get();
    }
    return nullopt;
}