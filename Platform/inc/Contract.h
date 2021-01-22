#pragma once
#include <memory>
#include <string>
#include <vector>

namespace TradeBase
{
    struct Contract
    {
        Contract() : conId( 0 ), strike( 0 ), includeExpired( false )
        {
        }

        long        conId;
        std::string symbol;
        std::string secType;
        std::string lastTradeDateOrContractMonth;
        double      strike;
        std::string right;
        std::string multiplier;
        std::string exchange;
        std::string primaryExchange;
        std::string currency;
        std::string localSymbol;
        std::string tradingClass;
        bool        includeExpired;
        std::string secIdType; // CUSIP;SEDOL;ISIN;RIC
        std::string secId;
    };
} // namespace TradeBase