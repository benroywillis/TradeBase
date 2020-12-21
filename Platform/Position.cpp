#include "Position.h"
#include "Contract.h"
#include "Data.h"
#include "DataStruct.h"
#include "Execution.h"
#include "Order.h"
#include <spdlog/spdlog.h>

using namespace std;

Position::Position( const Contract& newContract, const Order& newOrder, const Execution& newExecution ) : contract( make_shared<Contract>( newContract ) )
{
    orderHistory.push_back( newOrder );
    addExecution( newExecution );
    currentPrice = 0.0;
}

bool Position::longPosition() const
{
    return getFirstExecution().side == "BOT";
}

const Contract* Position::getContract() const
{
    return const_cast<Contract*>( contract.get() );
}

Order& Position::getLastOrder()
{
    return orderHistory.back();
}

const Order& Position::getLastOrder() const
{
    return orderHistory.back();
}

const Execution& Position::getFirstExecution() const
{
    return positionExecutions.front();
}

Execution& Position::getLastExecution()
{
    return positionExecutions.back();
}

const Execution& Position::getLastExecution() const
{
    return positionExecutions.back();
}

double Position::getAvgPrice() const
{
    return avgPrice;
}
double Position::getPositionSize() const
{
    return positionSize;
}

double Position::getPnL() const
{
    return PnL;
}

double Position::getCashValue() const
{
    return currentPrice * positionSize;
}

double Position::getCashValue( const GlobalTimePoint& point )
{
    MarkToMarket( point );
    return currentPrice * positionSize;
}

void Position::addExecution( const Execution& newExecution )
{
    Execution malleable = newExecution;
    updateQueue( malleable );
}

void Position::addOrder( const Order& newOrder )
{
    orderHistory.push_back( newOrder );
}

void Position::addData( const vector<shared_ptr<DataArray>>& newData )
{
}

double Position::updatePnL( double update )
{
    PnL += update;
    return PnL;
}

double Position::MarkToMarket( const GlobalTimePoint& point )
{
    bool found = false;
    for( const auto& point : point.Points )
    {
        if( point.p_Vector->contract.conId == contract->conId )
        {
            currentPrice = point.p_Point->get();
            found = true;
            break;
        }
    }
    if( !found )
    {
        return UPnL;
    }
    if( positionSize > 0 )
    {
        UPnL = positionSize * currentPrice - positionSize * avgPrice;
    }
    else
    {
        UPnL = ( -1 ) * positionSize * avgPrice - ( -1 ) * positionSize * currentPrice;
    }
    return UPnL;
}

void Position::calcAvgPrice()
{
    if( !positionExecutions.empty() )
    {
        double avgPricesum = 0.0;
        double shareSum = 0.0;
        for( const auto& ex : positionExecutions )
        {
            avgPricesum += ex.avgPrice;
            shareSum += ex.shares;
        }
        avgPrice = avgPricesum / positionExecutions.size();
        positionSize = shareSum;
    }
    else
    {
        avgPrice = 0;
        positionSize = 0;
    }
}

void Position::updateQueue( Execution& newExec )
{
    if( !positionExecutions.empty() )
    {
        if( newExec.side == "BOT" )
        {
            // if our queue contains offsetting shares
            if( positionExecutions.front().side == "SLD" )
            {
                // amount of money borrowed -> costbasis
                // amount of money received from buyback -> proceeds
                int transactionShares = newExec.shares;
                int diff = transactionShares - positionExecutions.front().shares;
                while( true )
                {
                    if( diff > 0 )
                    {
                        // we're not offset yet
                        updatePnL( positionExecutions.front().avgPrice * positionExecutions.front().shares - newExec.avgPrice * positionExecutions.front().shares );
                        positionExecutions.pop_front();
                        transactionShares = diff;
                        if( positionExecutions.empty() )
                        {
                            newExec.shares = transactionShares;
                            positionExecutions.push_back( newExec );
                            break;
                        }
                    }
                    else if( diff < 0 )
                    {
                        // we're more than offset
                        updatePnL( positionExecutions.front().avgPrice * transactionShares - newExec.avgPrice * transactionShares );
                        positionExecutions.front().shares = diff;
                        break;
                    }
                    else if( diff == 0 )
                    {
                        // we're on the money
                        updatePnL( positionExecutions.front().avgPrice * transactionShares - newExec.avgPrice * transactionShares );
                        positionExecutions.pop_front();
                        break;
                    }
                    diff = transactionShares - positionExecutions.front().shares;
                }
            }
            else
            {
                positionExecutions.push_back( newExec );
            }
        }
        else if( newExec.side == "SLD" )
        {
            // if our queue contains offsetting shares
            if( positionExecutions.front().side == "BOT" )
            {
                // amount of money spent buying long -> costbasis
                // amount of money received selling shares -> proceeds
                int transactionShares = newExec.shares;
                int diff = transactionShares - positionExecutions.front().shares;
                while( true )
                {
                    if( diff > 0 )
                    {
                        // we're not offset yet
                        updatePnL( newExec.avgPrice * positionExecutions.front().shares - positionExecutions.front().avgPrice * positionExecutions.front().shares );
                        transactionShares = diff;
                        positionExecutions.pop_front();
                        if( positionExecutions.empty() )
                        {
                            newExec.shares = transactionShares;
                            positionExecutions.push_back( newExec );
                            break;
                        }
                    }
                    if( diff < 0 )
                    {
                        // we're more than offset
                        updatePnL( newExec.avgPrice * transactionShares - positionExecutions.front().avgPrice * transactionShares );
                        positionExecutions.front().shares = diff;
                        break;
                    }
                    if( diff == 0 )
                    {
                        // we're right on the money
                        updatePnL( newExec.avgPrice * transactionShares - positionExecutions.front().avgPrice * transactionShares );
                        positionExecutions.pop_front();
                        break;
                    }
                    diff = transactionShares - positionExecutions.front().shares;
                }
            }
            else
            {
                positionExecutions.push_back( newExec );
            }
        } // if newExec.shares < 0
    }
    else
    {
        positionExecutions.push_back( newExec );
    }
    calcAvgPrice();
}