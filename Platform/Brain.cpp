#include "Brain.h"
#include "Account.h"
#include "Broker.h"
#include "Data.h"
#include "Execution.h"
#include "Order.h"
#include "Position.h"
#include "Strategy.h"

using namespace std;

long BackTrader::conId = 0;
long BackTrader::orderId = 0;
long BackTrader::vectorId = 9000;

BTBrain::BTBrain( double startCash, const shared_ptr<BTData>& newData, const shared_ptr<BTStrategy>& newStrategy )
{
    Strategy = newStrategy;
    Data = newData;
    Account = make_shared<BTAccount>( startCash, true );
    Broker = make_shared<BTBroker>();
}

BTBrain::BTBrain( double startCash, const shared_ptr<BTBroker>& newBroker, const shared_ptr<BTData>& newData, const shared_ptr<BTStrategy>& newStrategy )
{
    Account = make_shared<BTAccount>( startCash, true );
    Data = newData;
    Strategy = newStrategy;
    Broker = newBroker;
}

BTBrain::BTBrain( const shared_ptr<BTAccount>& newAccount, const shared_ptr<BTData>& newData, const shared_ptr<BTStrategy>& newStrategy )
{
    Account = newAccount;
    Data = newData;
    Strategy = newStrategy;
    Broker = make_shared<BTBroker>();
}

BTBrain::BTBrain( const shared_ptr<BTAccount>& newAccount, const shared_ptr<BTBroker>& newBroker, const shared_ptr<BTData>& newData, const shared_ptr<BTStrategy>& newStrategy )
{
    Account = newAccount;
    Data = newData;
    Strategy = newStrategy;
    Broker = newBroker;
}

void BTBrain::run()
{
    auto trades = vector<pair<Contract, Order>>();
    do
    {
        Strategy->ProcessNextTick( Account, Broker, Data, trades );
        if( !trades.empty() )
        {
            for( const auto& trade : trades )
            {
                auto tradeExec = Broker->processTrade( Account, Data, trade );
                if( !tradeExec.empty() )
                {
                    for( const auto& pos : tradeExec )
                    {
                        Account->update( pos );
                    }
                }
            }
        }
        trades.clear();
        Account->updatePnLs( Data->getCurrentPoint() );
    } while( Data->incrementTime() );
}
