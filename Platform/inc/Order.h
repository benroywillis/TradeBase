#pragma once
#include <limits>
#include <memory>
#include <vector>

struct Order
{
    /// order identifier
    long orderId;
    long clientId;
    long conId;
    int  permId;

    // main order fields
    std::string action;
    double      totalQuantity;
    std::string orderType;
    double      lmtPrice;
    double      auxPrice;
    std::string tif;
    int         minQty;

    Order()
    {
        orderId = 0;
        clientId = 0;
        permId = 0;
        conId = 0;
        totalQuantity = 0;
        lmtPrice = 0;
        auxPrice = 0;
        minQty = 0;
    }
};