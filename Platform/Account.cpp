#include "Account.h"
#include "Execution.h"
#include "Order.h"
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std;

BTAccount::BTAccount()
{
    startEquity = 0;
    cash = startEquity;
    accountEquity = cash;
    margin = true;
    maintenanceMargin = 0.25;
    marginLoan = 0.0;
    longMarginLoan = 0.0;
    shortMarginLoan = 0.0;
    positionsValue = 0.0;
}

BTAccount::BTAccount( double startingCash, bool Margin )
{
    startEquity = startingCash;
    cash = startEquity;
    accountEquity = cash;
    margin = Margin;
    maintenanceMargin = 0.25;
    marginLoan = 0.0;
    longMarginLoan = 0.0;
    shortMarginLoan = 0.0;
    positionsValue = 0.0;
}

BTAccount::~BTAccount()
{
    for( auto* pos : positions )
    {
        delete pos;
    }
}

void BTAccount::update( const Position& newPosition )
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
        // now check if the position has closed
        if( pos->getPositionSize() == 0 )
        {
            positions.erase( pos );
        }
    }
    else
    {
        positions.insert( new Position( newPosition ) );
    }
}

bool BTAccount::updateCash( const Contract& cont, const Order& ord, double cost )
{
    // 0 order size check
    if( ord.totalQuantity == 0 )
    {
        spdlog::warn( "Cannot accept order size of 0" );
        return false;
    }
    // cost per share
    double perShareCost = cost / ord.totalQuantity;
    // different kinds of costs to finance
    double buyingLongSharesValue = 0;
    double sellingLongSharesValue = 0;
    double sellingShortSharesValue = 0;
    double buyingShortSharesValue = 0;
    // Find existing shares, if any
    double positionShares = 0;
    if( positions.find( cont.conId ) != positions.end() )
    {
        auto* pos = *( positions.find( cont.conId ) );
        if( pos->longPosition() )
        {
            positionShares = pos->getPositionSize();
        }
        else
        {
            positionShares = -1 * pos->getPositionSize();
        }
    }
    // Order quantity
    double shares;
    if( ord.action == "BUY" )
    {
        shares = ord.totalQuantity;
    }
    else if( ord.action == "SELL" )
    {
        shares = -1 * ord.totalQuantity;
    }
    else
    {
        spdlog::critical( "Only order actions BUY and SELL are supported." );
        return false;
    }
    double shareSum = positionShares + shares;
    // now calculate value types
    if( positionShares >= 0 && shareSum >= 0 && shares > 0 )
    {
        // only buying long positions
        buyingLongSharesValue = cost;
    }
    if( positionShares >= 0 && shareSum >= 0 && shares < 0 )
    {
        // only closing long positions
        sellingLongSharesValue = cost;
    }
    if( positionShares >= 0 && shareSum <= 0 && shares > 0 )
    {
        // not possible
        spdlog::critical( "Something is wrong with order quantity logic in Accout::updateCash()." );
    }
    if( positionShares >= 0 && shareSum <= 0 && shares < 0 )
    {
        // may be both closing long positions and opening short ones (if positionShares > 0) or just selling short shares
        // positionShares is the number of closed long positions
        sellingLongSharesValue = positionShares * perShareCost;
        sellingShortSharesValue = abs( shareSum ) * perShareCost;
    }
    if( positionShares < 0 && shareSum >= 0 && shares > 0 )
    {
        // buying back short shares and maybe buying long shares
        buyingLongSharesValue = shareSum * perShareCost;
        buyingShortSharesValue = abs( positionShares ) * perShareCost;
    }
    if( positionShares < 0 && shareSum >= 0 && shares < 0 )
    {
        // not possible
        spdlog::critical( "Something is wrong with order quantity logic in Accout::updateCash()." );
    }
    if( positionShares < 0 && shareSum <= 0 && shares > 0 )
    {
        // buying back some short positions only
        buyingShortSharesValue = cost;
    }
    if( positionShares < 0 && shareSum < 0 && shares < 0 )
    {
        // selling short shares only
        sellingShortSharesValue = cost;
    }

    // enforce margin rules
    if( ( buyingShortSharesValue > 0 ) && !margin )
    {
        spdlog::critical( "Buying back short shares in a non-margin account!" );
        return false;
    }
    if( ( sellingShortSharesValue > 0 ) && !margin )
    {
        spdlog::error( "Shortselling is not allowed in a non-margin account." );
        return false;
    }

    // Now see if we have the power to cover the transaction values
    double tmpLongML = longMarginLoan;
    double tmpShortML = shortMarginLoan;
    double tmpCash = cash;
    // margin for long shares
    if( tmpLongML > 0 )
    {
        tmpLongML -= sellingLongSharesValue;
    }
    else
    {
        tmpCash -= sellingLongSharesValue;
    }
    if( tmpCash == 0 )
    {
        tmpLongML += buyingLongSharesValue;
    }
    else
    {
        tmpCash -= buyingLongSharesValue;
    }
    if( tmpCash < 0 )
    {
        tmpLongML -= tmpCash;
        tmpCash = 0;
    }
    if( tmpLongML < 0 )
    {
        tmpCash -= tmpLongML;
        tmpLongML = 0;
    }

    // margin for short shares
    // closing short positions
    tmpCash -= buyingShortSharesValue;
    tmpShortML -= buyingShortSharesValue;
    // opening new short positions
    tmpShortML += sellingShortSharesValue;
    tmpCash += sellingShortSharesValue;
    if( tmpCash < 0 )
    {
        tmpShortML -= tmpCash;
        tmpCash = 0;
    }

    if( tmpLongML < 0 )
    {
        tmpCash -= tmpLongML;
        tmpLongML = 0;
    }
    if( tmpShortML < 0 )
    {
        tmpCash -= tmpShortML;
        tmpShortML = 0;
    }
    if( !checkMaintenanceMargin( tmpShortML+tmpLongML, tmpCash ) )
    {
        return false;
    }
    // finally, update the cash and margin
    // update global cash and margin amounts
    cash = tmpCash;
    shortMarginLoan = tmpShortML;
    longMarginLoan = tmpLongML;
    marginLoan = shortMarginLoan+longMarginLoan;
    return true;
}

void BTAccount::updatePnLs( const GlobalTimePoint& point )
{
    UPnL = 0.0;
    PnL = 0.0;
    positionsValue = 0.0;
    for( const auto& pos : positions )
    {
        UPnL += pos->updateUPnL( point );
        PnL += pos->getPnL();
        if( pos->longPosition() )
        {
            positionsValue += pos->getCashValue( point );
        }
        else
        {
            positionsValue -= pos->getCashValue( point );
        }
    }
}

std::vector<Position*> BTAccount::getPositions() const
{
    vector<Position*> returnVec( positions.begin(), positions.end() );
    return returnVec;
}

double BTAccount::getCash() const
{
    return cash;
}
double BTAccount::getMarginLoan() const
{
    return marginLoan;
}

bool BTAccount::checkMaintenanceMargin( double mL, double c ) const
{
    if( mL > ( ( ( 1 / maintenanceMargin ) ) - 1 ) * ( ( positionsValue + c ) ) )
    {
        spdlog::warn( "Not enough buying power to place order." );
        return false;
    }
    return true;
}

double BTAccount::getPnL() const
{
    return PnL;
}

double BTAccount::getUPnL() const
{
    return UPnL;
}