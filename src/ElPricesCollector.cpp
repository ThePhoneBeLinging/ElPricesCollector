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
    updatingThread_.join();
}

std::shared_ptr<HourPrice> ElPricesCollector::getCurrentPrice()
{
    auto currentTime = TimeUtil::getCurrentTime();
    auto timeString = TimeUtil::timeToString(currentTime);

    return storageController_->getDate(timeString)->getPriceAtPoint(currentTime.tm_hour);
}

void ElPricesCollector::keepUpdated()
{
    while (keepRunningBool_)
    {
        std::string currentTimeString = TimeUtil::timeToString(TimeUtil::getCurrentTime());
        cpr::Response r = cpr::Get(cpr::Url{"https://andelenergi.dk/?obexport_format=csv&obexport_start=" + currentTimeString + "&obexport_end=" + currentTimeString + "&obexport_region=east&obexport_tax=0&obexport_product_id=1%231%23TIMEENERGI"});
        if (r.status_code != 200)
        {
            throw std::invalid_argument("Status code was not 200, it was: " + std::to_string(r.status_code));
        }
        storageController_->handleParsedData(r.text);
        keepRunningBool_ = false;
    }
}
