#include "Brain.h"
#include "CommissionSchemes.h"
#include "Data.h"
#include "DataStruct.h"
#include "Execution.h"
#include "HalvedPositionSMA.h"
#include "Order.h"
#include "Position.h"
#include "SMA.h"
#include "Util.hpp"

using namespace std;

constexpr long STARTINGCASH = 100000;
constexpr int  fast = 50;
constexpr int  slow = 200;

int main( int argc, char* argv[] )
{
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           Account = make_shared<BTAccount>( STARTINGCASH, true );
    auto                           Broker = make_shared<BTBroker>( morphedScheme );
    auto                           Indicators = vector<BTIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<BTData>( ReadInputs( argc, argv ), Indicators );
    shared_ptr<BTStrategy> Strategy = make_shared<HPSMA>();
    auto                   Brain = make_unique<BTBrain>( Account, Broker, Data, Strategy );
    Brain->run();
    PrintResults( Brain );
    return 0;
}