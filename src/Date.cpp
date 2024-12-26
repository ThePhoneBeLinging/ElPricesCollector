//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include <ElPricesCollector/PriceObjects/Date.h>
#include <memory>
#include <ElPricesCollector/PriceObjects/HourPrice.h>

Date::Date()
{
    hourPrices_.resize(24);
}

void Date::setPriceAtPoint(int hour, const std::shared_ptr<HourPrice>& hourPrice)
{
    hourPrices_[hour] = std::make_unique<HourPrice>(*hourPrice);
}

std::unique_ptr<HourPrice> Date::getPriceAtPoint(int hour) const
{
    return std::make_unique<HourPrice>(hourPrices_[hour]->getPriceWithoutFees(),hourPrices_[hour]->getFees());
}

bool Date::isDateComplete() const
{
    for (const auto& hourPrice : hourPrices_)
    {
        if (hourPrice == nullptr)
        {
            return false;
        }
    }
    return true;
}
