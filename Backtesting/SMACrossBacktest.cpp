#include "Brain.h"
#include "CommissionSchemes.h"
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
    auto csv = InputFile( "", "AMZN", "CONCYC", "STK", "SMART", "USD" );
    ReadInputs( argc, argv, csv );
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           Broker = make_shared<BTBroker>( morphedScheme );
    auto                           Indicators = vector<BTIndicator*>();
    auto                           SMAS = make_unique<SMA>( fast );
    auto                           SMAL = make_unique<SMA>( slow );
    Indicators.push_back( SMAS.get() );
    Indicators.push_back( SMAL.get() );
    auto                   Data = make_shared<BTData>( csv, Indicators );
    shared_ptr<BTStrategy> Strategy = make_shared<SMACrossover>();
    auto                   Brain = make_unique<BTBrain>( STARTINGCASH, Broker, Data, Strategy );
    Brain->run();
    PrintResults( Brain );
    return 0;
}