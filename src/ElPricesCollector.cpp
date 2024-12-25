//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "include/ElPricesCollector/ElPricesCollector.h"

#include <iostream>

ElPricesCollector::ElPricesCollector() : keepRunningBool_(true)
{
    updatingThread_ = std::thread(&ElPricesCollector::keepUpdated,this);
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
