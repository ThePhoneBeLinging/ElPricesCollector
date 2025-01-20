//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include "Utility/TimeUtil.h"
#include "cpr/response.h"
#include "cpr/api.h"
#include "Utility/ConfigController.h"

ElPricesCollector::ElPricesCollector() : keepRunningBool_(true), storageController_(std::make_shared<ElPricesStorageController>())
, constructorReadyForCompletion_(false)
{
    updatingThread_ = std::thread(&ElPricesCollector::keepUpdated,this);
    while (not constructorReadyForCompletion_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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

std::shared_ptr<HourPrice> ElPricesCollector::getPriceBasedOnSecondsAgo(int seconds)
{
    auto now = std::chrono::system_clock::now();
    now -= std::chrono::seconds(seconds);
    auto timeTM = TimeUtil::timeToTM(now);
    auto timeString = TimeUtil::timeToStringForLookup(timeTM);

    return storageController_->getHourPriceFromMemoryDB(timeString,timeTM.tm_hour);
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
        storageController_->handleParsedData(r.text);
        constructorReadyForCompletion_ = true;
        int secondsToWait = ConfigController::getConfigInt("ElPricesCollectorUsualSecondsDelay");
        conditionVariable_.wait_for(lock, std::chrono::seconds(TimeUtil::secondsToNextHour()));
    }
}
