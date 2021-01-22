#pragma once

namespace TradeBase
{
    class BackTrader
    {
    public:
        static inline long getNextConId()
        {
            return conId++;
        }
        static inline long getNextOrderId()
        {
            return orderId++;
        }
        static inline long getNextVectorId()
        {
            return vectorId++;
        }
        int timestamp;
        int ClientID;

    private:
        static long conId;
        static long orderId;
        static long vectorId;
    };
} // namespace TradeBase