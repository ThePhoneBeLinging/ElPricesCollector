//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector.h"

#include <iostream>

ElPricesCollector::ElPricesCollector() : keepRunningBool_(true)
{
    auto lambda = [this]()
    {
        this->keepUpdated();
    };
    updatingThread_ = std::thread(lambda);
}

ElPricesCollector::~ElPricesCollector()
{
    keepRunningBool_ = false;
    updatingThread_.join();
}

void ElPricesCollector::keepUpdated()
{
    while (keepRunningBool_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "HELLO WORLD \n";
    }
}
