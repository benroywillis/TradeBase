#include "Brain.h"
#include "Data.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"
#include "Util.hpp"
#include <spdlog/spdlog.h>

using namespace std;

class LastClose : public BTStrategy
{
public:
    LastClose() = default;
    /// @brief Runs private methods on new data
    void ProcessNextTick( const set<Position, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
    {
        for( const auto& point : D->getCurrentPoint().Points )
        {
            auto prevPoint = D->getPreviousVectorPoint( point.p_Vector->vectorId );
            if( !prevPoint )
            {
                return;
            }
            double price = point.p_Point->get();
            double prevPrice = prevPoint->p_Point->get();
            bool   buySignal = price > prevPrice;
            bool   sellSignal = price < prevPrice;
            auto   tradeContract = point.p_Vector->contract;
            if( buySignal )
            {
                auto tradeOrder = getOrder( true );
                spdlog::info( "Buying 1 share at " + tradeContract.symbol );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
            if( sellSignal )
            {
                auto tradeOrder = getOrder( false );
                spdlog::info( "Selling 1 share " + tradeContract.symbol );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
        }
    }
};

int main( int argc, char* argv[] )
{
    unique_ptr<IBCommissionScheme> morphedScheme = make_unique<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           conf = Configure( morphedScheme.get(), true );
    auto                           Data = make_shared<BTData>( ReadInputs( argc, argv ) );
    shared_ptr<BTStrategy>         Strategy = make_shared<LastClose>();
    auto                           Brain = make_unique<BTBrain>( Data, Strategy, conf );
    Brain->run();
    PrintResults( Brain );
    return 0;
}