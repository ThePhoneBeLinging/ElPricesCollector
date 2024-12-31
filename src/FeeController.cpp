//
// Created by eal on 12/31/24.
//

#include "FeeController.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include "Utility/TimeUtil.h"
#include "Utility/Utility.h"

FeeController::FeeController() : summerFees_(std::make_unique<Fee>()), winterFees_(std::make_unique<Fee>())
{
}

int FeeController::getCurrentFees()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

int FeeController::getFeesFromDate(int month, int hour)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (monthsWithSummerFees_.contains(month))
    {
        return summerFees_->getCurrentFee(hour);
    }
    if (monthsWithWinterFees_.contains(month))
    {
        return winterFees_->getCurrentFee(hour);
    }
    throw std::invalid_argument("FeeController::getCurrentFee() failed");
}

void FeeController::loadFeesFromFile()
{
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string file = "../../Resources/CeriusFees.json";

    std::string loadedFile = Utility::readFromFile(file);

    auto jsonObject = nlohmann::json::parse(loadedFile);

    auto prices = jsonObject["Prices"];
    auto lowLoadPrices = prices["LowLoad"];
    auto highLoadPrices = prices["HighLoad"];
    auto peakLoadPrices = prices["PeakLoad"];

    winterFees_->setLowLoadFee(lowLoadPrices["WinterTariff"].get<int>() + lowLoadPrices["ElTax"].get<int>());
    summerFees_->setLowLoadFee(lowLoadPrices["SummerTariff"].get<int>() + lowLoadPrices["ElTax"].get<int>());

    winterFees_->setHighLoadFee(highLoadPrices["WinterTariff"].get<int>() + highLoadPrices["ElTax"].get<int>());
    summerFees_->setHighLoadFee(highLoadPrices["SummerTariff"].get<int>() + highLoadPrices["ElTax"].get<int>());

    winterFees_->setPeakLoadFee(peakLoadPrices["WinterTariff"].get<int>() + peakLoadPrices["ElTax"].get<int>());
    summerFees_->setPeakLoadFee(peakLoadPrices["SummerTariff"].get<int>() + peakLoadPrices["ElTax"].get<int>());

    for (auto sub : jsonObject["WinterPeriod"]["ActiveMonths"])
    {
        monthsWithWinterFees_.insert(sub.get<int>());
    }
    for (auto sub : jsonObject["SummerPeriod"]["ActiveMonths"])
    {
        monthsWithSummerFees_.insert(sub.get<int>());
    }

    std::set<int> lowLoadHours;
    std::set<int> highLoadHours;
    std::set<int> peakLoadHours;

    for (auto sub : jsonObject["WinterPeriod"]["HoursLoadLevel"]["LowLoad"])
    {
        lowLoadHours.insert(sub.get<int>());
    }
    for (auto sub : jsonObject["WinterPeriod"]["HoursLoadLevel"]["HighLoad"])
    {
        highLoadHours.insert(sub.get<int>());
    }
    for (auto sub : jsonObject["WinterPeriod"]["HoursLoadLevel"]["PeakLoad"])
    {
        peakLoadHours.insert(sub.get<int>());
    }
    winterFees_->setLowLoadHours(lowLoadHours);
    winterFees_->setHighLoadHours(highLoadHours);
    winterFees_->setPeakLoadHours(peakLoadHours);

    lowLoadHours.clear();
    highLoadHours.clear();
    peakLoadHours.clear();

    for (auto sub : jsonObject["SummerPeriod"]["HoursLoadLevel"]["LowLoad"])
    {
        lowLoadHours.insert(sub.get<int>());
    }
    for (auto sub : jsonObject["SummerPeriod"]["HoursLoadLevel"]["HighLoad"])
    {
        highLoadHours.insert(sub.get<int>());
    }
    for (auto sub : jsonObject["SummerPeriod"]["HoursLoadLevel"]["PeakLoad"])
    {
        peakLoadHours.insert(sub.get<int>());
    }
    summerFees_->setLowLoadHours(lowLoadHours);
    summerFees_->setHighLoadHours(highLoadHours);
    summerFees_->setPeakLoadHours(peakLoadHours);

}
