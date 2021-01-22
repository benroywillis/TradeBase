#pragma once
#include <string>

namespace TradeBase
{
    struct Execution
    {
        Execution()
        {
            shares = 0;
            price = 0;
            permId = 0;
            clientId = 0;
            orderId = 0;
            cumQty = 0;
            avgPrice = 0;
            evMultiplier = 0;
            lastLiquidity = 0;
        }

        std::string execId;
        std::string time;
        std::string acctNumber;
        std::string exchange;
        std::string side;
        double      shares;
        double      price;
        int         permId;
        long        clientId;
        long        orderId;
        int         liquidation;
        double      cumQty;
        double      avgPrice;
        std::string orderRef;
        std::string evRule;
        double      evMultiplier;
        std::string modelCode;
        int         lastLiquidity;
    };
} // namespace TradeBase