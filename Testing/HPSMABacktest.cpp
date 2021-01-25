#include "Brain.h"
#include "Data.h"
#include "Execution.h"
#include "Indicator.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"
#include "Util.hpp"
#include <set>
#include <spdlog/spdlog.h>

using namespace std;
using namespace TradeBase;

constexpr int fast = 5;
constexpr int slow = 20;
constexpr int POSITIONSIZE = 1;

class SMA : public TBIndicator
{
public:
    SMA( int bufferSize ) : TBIndicator( bufferSize )
    {
    }

    unique_ptr<TBIndicator> clone() const override
    {
        return make_unique<SMA>( *this );
    }

    void update( const DataStruct& newPoint ) override
    {
        buffPos = buffPos % buffLength;
        buffer[buffPos] = newPoint;
        buffPos++;
        if( !valid )
        {
            init++;
            if( init == buffLength )
            {
                valid = true;
            }
        }
        currentValue = 0;
        for( auto& index : buffer )
        {
            currentValue += index.get() / buffLength;
        }
    }
};

class HPSMA : public TBStrategy
{
public:
    void ProcessNextTick( const set<Position*, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<TBData>& D, vector<pair<Contract, Order>>& trades )
    {
        auto time = D->getCurrentTime();
        for( const auto& point : D->getCurrentPoint().Points )
        {
            bool buySignal = false;
            bool sellSignal = false;

            // check if there is already an open order for this contract
            if( openOrders.find( point.p_Vector->contract ) != openOrders.end() )
            {
                continue;
            }

            // Find a position in the account that already exists for this contract
            Position* existingPos = nullptr;
            string    side = "NEUTRAL";
            for( const auto& pos : positions )
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
            if( !buySignal )
            {
                if( existingPos != nullptr )
                {
                    sellSignal = ( point.p_Vector->Inds[0]->currentValue < point.p_Vector->Inds[1]->currentValue ) || ( existingPos->getAvgPrice() < point.p_Point->get() * 0.9 ) || ( existingPos->getAvgPrice() * 1.2 > point.p_Point->get() );
                }
                else
                {
                    sellSignal = point.p_Vector->Inds[0]->currentValue < point.p_Vector->Inds[1]->currentValue;
                }
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
            else
            {
                if( sellSignal && side == "BOT" )
                {
                    spdlog::info( "Selling " + to_string( POSITIONSIZE * 2 ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) + " when the avg price is " + to_string( existingPos->getAvgPrice() ) );
                    auto tradeOrder = getOrder( false, POSITIONSIZE * 2 );
                    trades.emplace_back( pair( tradeContract, tradeOrder ) );
                }
                else if( sellSignal && side == "NEUTRAL" )
                {
                    spdlog::info( "Selling " + to_string( POSITIONSIZE ) + " shares at time " + time.toString() + " at price " + to_string( point.p_Point->get() ) + " when there is no pre-existing position." );
                    auto tradeOrder = getOrder( false, POSITIONSIZE );
                    trades.emplace_back( pair( tradeContract, tradeOrder ) );
                }
            }
        }
    }
};

int main( int argc, char* argv[] )
{
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 0, 0 );
    auto                           conf = Configure( morphedScheme.get(), false );
    auto                           Indicators = vector<TBIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<TBData>( ReadInputs( argc, argv ), Indicators );
    shared_ptr<TBStrategy> Strategy = make_shared<HPSMA>();
    auto                   Brain = make_unique<TBBrain>( Data, Strategy, conf );
    Brain->run();
    PrintResults( Brain );
    // compare results to those known to be correct
    bool error = false;
    if( Brain->getPositions()[0]->getPositionSize() != 1 )
    {
        spdlog::error( "Ending position size was " + to_string( Brain->getPositions()[0]->getPositionSize() ) + " and the correct answer is 1!" );
        error = true;
    }
    if( fabs( Brain->getMaximumGain() - 1.1705 ) >= 0.01 )
    {
        spdlog::error( "Maximum gain was " + to_string( Brain->getMaximumGain() ) + " and the correct answer is 1.1705!" );
        error = true;
    }
    if( fabs( Brain->getMaximumDrawdown() + 20.2995 ) >= 0.01 )
    {
        spdlog::error( "Maximum drawdown was " + to_string( Brain->getMaximumDrawdown() ) + " and the correct answer is -20.2995!" );
        error = true;
    }
    if( fabs( Brain->getMTMChange() - 1.1705 ) >= 0.01 )
    {
        spdlog::error( "Ending Marked To Market change was " + to_string( Brain->getMTMChange() ) + " and the correct answer is 1.1705!" );
        error = true;
    }
    if( error )
    {
        return 1;
    }
    return 0;
}