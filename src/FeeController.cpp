//
// Created by eal on 12/31/24.
//

#include "FeeController.h"

#include "Utility/TimeUtil.h"

FeeController::FeeController() : summerFees_(std::make_unique<Fee>()), winterFees_(std::make_unique<Fee>())
{
}

int FeeController::getCurrentFees() const
{
    const auto now = TimeUtil::getCurrentTime();
    auto currentMonth = now.tm_mon;
    auto currentHour = now.tm_hour;
    if (monthsWithSummerFees_.contains(currentMonth))
    {
        return summerFees_->getCurrentFee(currentHour);
    }
    if (monthsWithWinterFees_.contains(currentMonth))
    {
        return winterFees_->getCurrentFee(currentHour);
    }
    throw std::invalid_argument("FeeController::getCurrentFee() failed");
}
