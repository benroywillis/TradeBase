#include "Brain.h"
#include "Data.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"
#include "Util.hpp"
#include "matplotlibcpp.h"
#include <spdlog/spdlog.h>

using namespace std;
namespace plt = matplotlibcpp;

class LastClose : public BTStrategy
{
public:
    LastClose() = default;
    /// @brief Runs private methods on new data
    void ProcessNextTick( const set<Position*, positionCompare>& positions, const set<Order, OrderCompare>& openOrders, const shared_ptr<BTData>& D, vector<pair<Contract, Order>>& trades )
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
            auto   tradeContract = point.p_Vector->contract;
            if( price > prevPrice )
            {
                auto tradeOrder = getOrder( true );
                spdlog::info( "Buying 1 share of " + tradeContract.symbol + " at price " + to_string( price ) + "." );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
            else
            {
                auto tradeOrder = getOrder( false );
                spdlog::info( "Selling 1 share of " + tradeContract.symbol + " at price " + to_string( price ) + "." );
                trades.emplace_back( pair( tradeContract, tradeOrder ) );
            }
        }
    }
};

int main( int argc, char* argv[] )
{
    unique_ptr<IBCommissionScheme> morphedScheme = make_unique<IBCommissionScheme>( 0, 0, 0, 0 );
    auto                           conf = Configure( morphedScheme.get(), true );
    auto                           Data = make_shared<BTData>( ReadInputs( argc, argv ) );
    shared_ptr<BTStrategy>         Strategy = make_shared<LastClose>();
    auto                           Brain = make_unique<BTBrain>( Data, Strategy, conf );
    Brain->run();
    // compare results to those known to be correct
    bool error = false;
    if( Brain->getPositions()[0]->getPositionSize() != 15 )
    {
        spdlog::error( "Ending position size was " + to_string( Brain->getPositions()[0]->getPositionSize() ) + " and the correct answer is 15!" );
        error = true;
    }
    if( fabs( Brain->getMaximumGain() - 177.298096 ) >= 0.01 )
    {
        spdlog::error( "Maximum gain was " + to_string( Brain->getMaximumGain() ) + " and the correct answer is 177.298096!" );
        error = true;
    }
    if( fabs( Brain->getMaximumDrawdown() + 55.553223 ) >= 0.01 )
    {
        spdlog::error( "Maximum drawdown was " + to_string( Brain->getMaximumDrawdown() ) + " and the correct answer is -55.553223!" );
        error = true;
    }
    if( fabs( Brain->getMTMChange() - 177.298096 ) >= 0.01 )
    {
        spdlog::error( "Ending Marked To Market change was " + to_string( Brain->getMTMChange() ) + " and the correct answer is 177.298096!" );
        error = true;
    }
    if( error )
    {
        return 1;
    }
    plt::plot( Brain->getMTMHistory() );
    plt::show();
    return 0;
}