//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include <iostream>

#include "Utility/TimeUtil.h"
#include "cpr/response.h"
#include "cpr/api.h"

ElPricesCollector::ElPricesCollector() : keepRunningBool_(true), storageController_(std::make_shared<ElPricesStorageController>())
{
    updatingThread_ = std::thread(&ElPricesCollector::keepUpdated,this);
}

ElPricesCollector::~ElPricesCollector()
{
    keepRunningBool_ = false;
    conditionVariable_.notify_all();
    updatingThread_.join();
}

std::shared_ptr<HourPrice> ElPricesCollector::getCurrentPrice()
{
    auto currentTime = TimeUtil::getCurrentTime();
    auto timeString = TimeUtil::timeToStringForLookup(currentTime);

    return storageController_->getHourPriceFromMemoryDB(timeString,currentTime.tm_hour);
}

std::vector<std::shared_ptr<HourPrice>> ElPricesCollector::getPricesAroundCurrentTime()
{
    std::vector<std::shared_ptr<HourPrice>> backwardsPrices;
    std::vector<std::shared_ptr<HourPrice>> forwardsPrices;
    auto backwardsTimePtr = std::chrono::system_clock::now();
    auto forwardsTimePtr = std::chrono::system_clock::now();
    int hoursPastAndPresent = 7;
    for (int i = 0; i < hoursPastAndPresent; i++)
    {
        backwardsTimePtr -= std::chrono::hours(1);
        forwardsTimePtr += std::chrono::hours(1);
        tm backwardsTime = TimeUtil::timeToTM(backwardsTimePtr);
        tm forwardsTime = TimeUtil::timeToTM(forwardsTimePtr);
        std::string backwardsTimeString = TimeUtil::timeToStringForLookup(backwardsTime);
        std::string forwardsTimeString = TimeUtil::timeToStringForLookup(forwardsTime);
        backwardsPrices.push_back(storageController_->getHourPriceFromMemoryDB(backwardsTimeString,backwardsTime.tm_hour));
        forwardsPrices.push_back(storageController_->getHourPriceFromMemoryDB(forwardsTimeString, forwardsTime.tm_hour));
    }
    std::vector<std::shared_ptr<HourPrice>> currentPrices;

    std::reverse(backwardsPrices.begin(), backwardsPrices.end());
    std::reverse(forwardsPrices.begin(), forwardsPrices.end());

    for (const auto& backwardsPrice : backwardsPrices)
    {
        currentPrices.push_back(backwardsPrice);
    }
    currentPrices.push_back(getCurrentPrice());
    for (const auto& forwardsPrice : forwardsPrices)
    {
        currentPrices.push_back(forwardsPrice);
    }

    return currentPrices;
}

void ElPricesCollector::keepUpdated()
{
    std::mutex mutex_;
    std::unique_lock lock(mutex_);
    while (keepRunningBool_)
    {
        auto currentTime = TimeUtil::getCurrentTime();
        std::string currentTimeLookupString = TimeUtil::timeToStringForLookup(currentTime);
        std::string currentTimeAPIString = TimeUtil::timeToStringForAPI(currentTime);
        auto tmrwTime = TimeUtil::getTommorowTime();
        std::string tmrwTimeString = TimeUtil::timeToStringForAPI(tmrwTime);
        cpr::Response r = cpr::Get(cpr::Url{"https://andelenergi.dk/?obexport_format=csv&obexport_start=" + currentTimeAPIString + "&obexport_end=" + currentTimeAPIString + "&obexport_region=east&obexport_tax=0&obexport_product_id=1%231%23TIMEENERGI"});
        if (r.status_code != 200)
        {
            throw std::invalid_argument("Status code was not 200, it was: " + std::to_string(r.status_code));
        }
        std::cout << r.text << std::endl;
        storageController_->handleParsedData(r.text);

        conditionVariable_.wait_for(lock, std::chrono::hours(1));
    }
}
