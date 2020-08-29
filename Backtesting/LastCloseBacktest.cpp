#include "CommissionSchemes.h"
#include "Data.h"
#include "Execution.h"
#include "LastClose.h"
#include "Order.h"
#include "Position.h"
#include "Util.hpp"

using namespace std;

constexpr long STARTINGCASH = 100000;

int main( int argc, char* argv[] )
{
    shared_ptr<IBCommissionScheme> morphedScheme = make_shared<IBCommissionScheme>( 0, 0, 1, 1 );
    auto                           Account = make_shared<BTAccount>( STARTINGCASH, true );
    auto                           Broker = make_shared<BTBroker>( morphedScheme );
    auto                           Data = make_shared<BTData>( ReadInputs( argc, argv ) );
    shared_ptr<BTStrategy>         Strategy = make_shared<LastClose>();
    auto                           Brain = make_unique<BTBrain>( Account, Broker, Data, Strategy );
    Brain->run();
    PrintResults( Brain );
    return 0;
}