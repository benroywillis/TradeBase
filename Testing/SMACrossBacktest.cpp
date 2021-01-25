#include "Brain.h"
#include "Data.h"
#include "Indicator.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"
#include "Util.hpp"
#include <spdlog/spdlog.h>

using namespace std;
using namespace TradeBase;

constexpr int fast = 5;
constexpr int slow = 20;

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

class SMACrossover : public TBStrategy
{
public:
    /// In this strategy, the short SMA is the first indicator index
    /// So the one whose valid needs to be checked is index 1
    void ProcessNextTick( const set<Position*, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<TBData>& D, vector<pair<Contract, Order>>& trades )
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
                spdlog::info( "Buying 1 share of " + tradeContract.symbol + " at price " + to_string( point.p_Point->get() ) + "." );
            }
            if( sellSignal )
            {
                auto tradeOrder = getOrder( false );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
                spdlog::info( "Selling 1 share of " + tradeContract.symbol + " at price " + to_string( point.p_Point->get() ) + "." );
            }
        }
    }
};

int main( int argc, char* argv[] )
{
    auto                           csv = InputFile( "", "AMZN", "CONCYC", "STK", "SMART", "USD" );
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 0, 0 );
    auto                           conf = Configure( morphedScheme.get(), false );
    auto                           Indicators = vector<TBIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<TBData>( ReadInputs( argc, argv ), Indicators );
    shared_ptr<TBStrategy> Strategy = make_shared<SMACrossover>();
    auto                   Brain = make_unique<TBBrain>( Data, Strategy, conf );
    Brain->run();
    // compare results to those known to be correct
    bool error = false;
    if( Brain->getPositions()[0]->getPositionSize() != 18 )
    {
        spdlog::error( "Ending position size was " + to_string( Brain->getPositions()[0]->getPositionSize() ) + " and the correct answer is 15!" );
        error = true;
    }
    if( fabs( Brain->getMaximumGain() - 165.48 ) >= 0.01 )
    {
        spdlog::error( "Maximum gain was " + to_string( Brain->getMaximumGain() ) + " and the correct answer is 165.48!" );
        error = true;
    }
    if( fabs( Brain->getMaximumDrawdown() + 215.307 ) >= 0.01 )
    {
        spdlog::error( "Maximum drawdown was " + to_string( Brain->getMaximumDrawdown() ) + " and the correct answer is -215.307!" );
        error = true;
    }
    if( fabs( Brain->getMTMChange() + 37.12 ) >= 0.01 )
    {
        spdlog::error( "Ending Marked To Market change was " + to_string( Brain->getMTMChange() ) + " and the correct answer is -37.12!" );
        error = true;
    }
    if( error )
    {
        return 1;
    }
    return 0;
}