//
// Created by eal on 12/31/24.
//

#include "Fee.h"

#include <stdexcept>
#include <utility>

int Fee::getCurrentFee(const int hour) const
{
    if (lowLoadHours_.contains(hour))
    {
        return lowLoadFee_;
    }
    if (highLoadHours_.contains(hour))
    {
        return highLoadFee_;
    }
    if (peakLoadHours_.contains(hour))
    {
        return peakLoadFee_;
    }
    throw std::out_of_range("Fee::getCurrentFee");
}

void Fee::setLowLoadFee(int fee)
{
    lowLoadFee_ = fee;
}

void Fee::setHighLoadFee(int fee)
{
    highLoadFee_ = fee;
}

void Fee::setPeakLoadFee(int fee)
{
    peakLoadFee_ = fee;
}

void Fee::setLowLoadHours(std::set<int> hours)
{
    lowLoadHours_ = std::move(hours);
}

void Fee::setHighLoadHours(std::set<int> hours)
{
    highLoadHours_ = std::move(hours);
}

void Fee::setPeakLoadHours(std::set<int> hours)
{
    peakLoadHours_ = std::move(hours);
}
