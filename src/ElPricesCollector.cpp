//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include <iostream>

#include "TimeUtil.h"
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

    return storageController_->getDate(timeString)->getPriceAtPoint(currentTime.tm_hour);
}

void ElPricesCollector::keepUpdated()
{
    std::mutex mutex_;
    std::unique_lock lock(mutex_);
    while (keepRunningBool_)
    {
        auto currentTime = TimeUtil::getCurrentTime();
        std::string currentTimeLookupString = TimeUtil::timeToStringForLookup(currentTime);
        auto tmrwTime = TimeUtil::getTommorowTime();
        std::string tmrwLookUpString = TimeUtil::timeToStringForLookup(tmrwTime);
        if (storageController_->getDate(currentTimeLookupString) != nullptr && storageController_->getDate(currentTimeLookupString)->isDateComplete())
        {
            if (storageController_->getDate(tmrwLookUpString) != nullptr && storageController_->getDate(tmrwLookUpString)->isDateComplete())
            {
                conditionVariable_.wait_for(lock, std::chrono::hours(1));
                continue;
            }

            int timeOfTmrwPriceRelease = 13;
            if (currentTime.tm_hour < timeOfTmrwPriceRelease)
            {
                conditionVariable_.wait_for(lock, std::chrono::hours(1));
                continue;
            }
        }

        std::string currentTimeAPIString = TimeUtil::timeToStringForAPI(currentTime);
        std::string tmrwTimeString = TimeUtil::timeToStringForAPI(TimeUtil::getTommorowTime());
        cpr::Response r = cpr::Get(cpr::Url{"https://andelenergi.dk/?obexport_format=csv&obexport_start=" + currentTimeAPIString + "&obexport_end=" + tmrwTimeString + "&obexport_region=east&obexport_tax=0&obexport_product_id=1%231%23TIMEENERGI"});
        if (r.status_code != 200)
        {
            throw std::invalid_argument("Status code was not 200, it was: " + std::to_string(r.status_code));
        }
        std::cout << r.text << std::endl;
        storageController_->handleParsedData(r.text);

        conditionVariable_.wait_for(lock, std::chrono::hours(1));
    }
}
