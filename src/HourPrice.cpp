//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//


#include <ElPricesCollector/PriceObjects/HourPrice.h>

HourPrice::HourPrice(int priceWithoutFees, int fees) : priceWithoutFees_(priceWithoutFees), fees_(fees)
{
}

int HourPrice::getPriceWithoutFees() const
{
    return priceWithoutFees_;
}

int HourPrice::getFees() const
{
    return fees_;
}

int HourPrice::getTotalPrice() const
{
    return priceWithoutFees_ + fees_;
}
