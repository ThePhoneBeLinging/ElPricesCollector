//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include <iostream>

#include "Utility/TimeUtil.h"
#include "cpr/response.h"
#include "cpr/api.h"
#include "Utility/ConfigController.h"
#include "Utility/DebugController.h"

ElPricesCollector::ElPricesCollector() : storageController_(std::make_shared<ElPricesStorageController>())
                                         , constructorReadyForCompletion_(false)
{
    storageController_->initMemoryDBFromFile();
    while (not constructorReadyForCompletion_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

std::shared_ptr<HourPrice> ElPricesCollector::getCurrentPrice()
{
    auto currentTime = TimeUtil::getCurrentTime();
    auto timeString = TimeUtil::timeToStringForLookup(currentTime);

    return storageController_->getHourPriceFromMemoryDB(timeString,currentTime.tm_hour);
}

std::shared_ptr<HourPrice> ElPricesCollector::getPriceBasedOnSecondsAgo(int seconds)
{
    auto now = std::chrono::system_clock::now();
    now -= std::chrono::seconds(seconds);
    auto timeTM = TimeUtil::timeToTM(now);
    auto timeString = TimeUtil::timeToStringForLookup(timeTM);

    return storageController_->getHourPriceFromMemoryDB(timeString,timeTM.tm_hour);
}

std::vector<std::shared_ptr<HourPrice>> ElPricesCollector::getCurrentAndFuturePrices()
{
    return storageController_->getCurrentAndFuturePrices();
}


void ElPricesCollector::update()
{
    std::mutex mutex_;
    std::unique_lock lock(mutex_);
    auto currentTime = TimeUtil::getCurrentTime();
    auto firstDayInWeek = std::chrono::system_clock::now() - std::chrono::days(4);
    auto firstDayTime = TimeUtil::timeToTM(firstDayInWeek);
    auto firstDayInWeekString = TimeUtil::timeToStringForAPI(firstDayTime);

    auto lastDayInWeek = std::chrono::system_clock::now() + std::chrono::days(4);
    auto lastDayTime = TimeUtil::timeToTM(lastDayInWeek);
    auto lastDayInWeekString = TimeUtil::timeToStringForAPI(lastDayTime);

    std::string currentTimeLookupString = TimeUtil::timeToStringForLookup(currentTime);
    std::string currentTimeAPIString = TimeUtil::timeToStringForAPI(currentTime);
    auto tmrwTime = TimeUtil::getTommorowTime();
    std::string tmrwTimeString = TimeUtil::timeToStringForAPI(tmrwTime);
    cpr::Response r = cpr::Get(cpr::Url{"https://andelenergi.dk/?obexport_format=csv&obexport_start=" + firstDayInWeekString + "&obexport_end=" + lastDayInWeekString + "&obexport_region=east&obexport_tax=0&obexport_product_id=1%231%23TIMEENERGI"});
    if (r.status_code != 200)
    {
        DebugController::debugWrite("Status code was not 200, it was: " + std::to_string(r.status_code));
    }
    storageController_->handleParsedData(r.text);
    constructorReadyForCompletion_ = true;
}
