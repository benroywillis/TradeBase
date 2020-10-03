#include "Brain.h"
#include "Data.h"
#include "Indicator.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"
#include "Util.hpp"
#include <spdlog/spdlog.h>

using namespace std;

constexpr long STARTINGCASH = 100000;
constexpr int  fast = 50;
constexpr int  slow = 200;

class SMA : public BTIndicator
{
public:
    SMA( int bufferSize ) : BTIndicator( bufferSize )
    {
    }

    unique_ptr<BTIndicator> clone() const override
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

class SMACrossover : public BTStrategy
{
public:
    /// In this strategy, the short SMA is the first indicator index
    /// So the one whose valid needs to be checked is index 1
    void ProcessNextTick( const set<Position*, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
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
};

int main( int argc, char* argv[] )
{
    auto                           csv = InputFile( "", "AMZN", "CONCYC", "STK", "SMART", "USD" );
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           conf = Configure( morphedScheme.get(), false );
    auto                           Indicators = vector<BTIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<BTData>( ReadInputs( argc, argv, csv ) );
    shared_ptr<BTStrategy> Strategy = make_shared<SMACrossover>();
    auto                   Brain = make_unique<BTBrain>( Data, Strategy, conf );
    Brain->run();
    PrintResults( Brain );
    return 0;
}