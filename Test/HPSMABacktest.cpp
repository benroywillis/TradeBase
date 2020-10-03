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

constexpr int fast = 50;
constexpr int slow = 200;
constexpr int POSITIONSIZE = 1;

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

class HPSMA : public BTStrategy
{
public:
    void ProcessNextTick( const set<Position*, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
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
            /*for( const auto& pa : openOrders )
            {
                spdlog::info( "Open order for symbol " + pa.symbol + ", secType " + pa.secType + ", conId " + to_string( pa.conId ) + " right " + pa.right + " strike " + to_string( pa.strike ) + " expiry " + pa.lastTradeDateOrContractMonth );
                if( pa.conId == point.p_Vector->contract.conId )
                {
                    continue;
                }
            }*/

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
};

int main( int argc, char* argv[] )
{
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           conf = Configure( morphedScheme.get(), false );
    auto                           Indicators = vector<BTIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<BTData>( ReadInputs( argc, argv ), Indicators );
    shared_ptr<BTStrategy> Strategy = make_shared<HPSMA>();
    auto                   Brain = make_unique<BTBrain>( Data, Strategy, conf );
    Brain->run();
    PrintResults( Brain );
    return 0;
}