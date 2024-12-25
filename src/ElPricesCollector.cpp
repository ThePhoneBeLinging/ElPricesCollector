//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include <iostream>

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
    return nullptr;
}

void ElPricesCollector::keepUpdated()
{
    while (keepRunningBool_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "HELLO WORLD \n";
    }
}
