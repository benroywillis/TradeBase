#include "Brain.h"
#include "Data.h"
#include "Execution.h"
#include "Order.h"
#include "Strategy.h"
#include <spdlog/spdlog.h>
#include <utility>

using namespace std;

long BackTrader::conId = 0;
long BackTrader::orderId = 0;
long BackTrader::vectorId = 9000;

BTBrain::BTBrain( const shared_ptr<BTData>& newData, const shared_ptr<BTStrategy>& newStrategy, const Configure& conf )
{
    Strategy = newStrategy;
    Data = newData;
    scheme = conf.scheme;
}

void BTBrain::run()
{
    auto trades = vector<pair<Contract, Order>>();
    auto openOrders = set<Order, OrderCompare>();
    do
    {
        Strategy->ProcessNextTick( positions, openOrders, Data, trades );
        if( !trades.empty() )
        {
            for( const auto& trade : trades )
            {
                auto tradeExec = processTrade( trade );
                if( !tradeExec.empty() )
                {
                    for( const auto& pos : tradeExec )
                    {
                        update( pos );
                    }
                }
            }
        }
        MarkToMarket( Data->getCurrentPoint() );
        trades.clear();
    } while( Data->incrementTime() );
}

void BTBrain::update( const Position& newPosition )
{
    auto samePos = positions.find( newPosition.getContract()->conId );
    if( samePos != positions.end() )
    {
        auto* pos = *samePos;
        if( newPosition.getContract()->conId == pos->getContract()->conId )
        {
            auto newExec = newPosition.getLastExecution();
            pos->updateQueue( newExec );
        }
    }
    else
    {
        positions.insert( new Position( newPosition ) );
    }
}

vector<Position> BTBrain::processTrade( const pair<Contract, Order>& p )
{
    vector<Position> returnVec;
    auto             con = p.first;
    auto             order = p.second;
    auto             currentPrice = getPrice( con );
    if( currentPrice )
    {
        double costBasis = currentPrice.value() * order.totalQuantity;
        auto   newExec = Execute( con, order, costBasis );
        if( newExec )
        {
            returnVec.emplace_back( Position( con, order, newExec.value() ) );
        }
    }
    return returnVec;
}

optional<Execution> BTBrain::Execute( const Contract& cont, const Order& ord, double costBasis )
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
        spdlog::error( "The broker can only accept secTypes of STK and OPT." );
        tradeCommission = 0.0;
        newCostBasis = costBasis;
    }
    totalCommission += tradeCommission;
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

optional<double> BTBrain::getPrice( const Contract& con )
{
    auto point = Data->lookup( con.conId );
    if( point )
    {
        return point->p_Point->get();
    }
    return nullopt;
}

double BTBrain::getMaximumGain() const
{
    return maxGain;
}

double BTBrain::getMaximumDrawdown() const
{
    return maxDrawdown;
}

std::vector<Position*> BTBrain::getPositions() const
{
    vector<Position*> returnVec( positions.begin(), positions.end() );
    return returnVec;
}

double BTBrain::getMTMChange() const
{
    return MTMChange;
}

const vector<double>& BTBrain::getMTMHistory() const
{
    return MTMHistory;
}

double BTBrain::getTotalCommission() const
{
    return totalCommission;
}

void BTBrain::MarkToMarket( const GlobalTimePoint& point )
{
    PnL = 0.0;
    for( const auto& pos : positions )
    {
        PnL += pos->getPnL();
    }
    UPnL = 0.0;
    for( const auto& pos : positions )
    {
        UPnL += pos->MarkToMarket( point );
    }
    MTMChange = UPnL + PnL;
    MTMHistory.push_back( MTMChange );
    maxGain = MTMChange > maxGain ? MTMChange : maxGain;
    maxDrawdown = MTMChange < maxDrawdown ? MTMChange : maxDrawdown;
}