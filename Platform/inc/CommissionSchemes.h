#pragma once
#include <iostream>

/// The default commission scheme is that everything is free. Only STK and OPT are supported
struct CommissionScheme
{
    CommissionScheme() : taxes( 0 ), fees( 0 ), pershare( 0 ), percontract( 0 )
    {
    }
    double         taxes;
    double         fees;
    double         pershare;
    double         percontract;
    virtual double perShare( int shares )
    {
        return pershare;
    }
    virtual double perContract( double premium, int contracts )
    {
        return percontract;
    }
};

/// @brief Commission scheme resembling IBs scheme for stocks and ETF on american exchanges, and options
///
/// This is an abridged version and only supports STK and OPT secType.
/// The threshold volume for the first tier is 10,001 or more, so I didn't even bother programming that into the scheme.
struct IBCommissionScheme : public CommissionScheme
{
    IBCommissionScheme()
    {
    }
    IBCommissionScheme( double newTax, double newFee, double newShare, double newContract )
    {
        taxes = newTax;
        fees = newFee;
        pershare = newShare;
        percontract = newContract;
    }
    double perShare( int shares ) override
    {
        return pershare;
    }
    double perContract( double premium, int contracts ) override
    {
        if( contracts == 1 )
        {
            return 1.00;
        }
        if( 0.1 <= premium )
        {
            return contracts * 0.65;
        }
        if( 0.05 <= premium < 0.1 )
        {
            return contracts * 0.5;
        }
        if( premium < 0.05 )
        {
            return contracts * 0.25;
        }
        return 0.0;
    }
};