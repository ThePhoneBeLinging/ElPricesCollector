//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESCOLLECTOR_H
#define ELPRICESCOLLECTOR_H

#include <atomic>
#include <condition_variable>
#include <thread>

#include "ElPricesCollector/ElPricesStorageController.h"
#include "PriceObjects/HourPrice.h"

class ElPricesCollector
{
public:
    ElPricesCollector();
    ~ElPricesCollector();

    std::shared_ptr<HourPrice> getCurrentPrice();
    std::shared_ptr<HourPrice> getPriceBasedOnSecondsAgo(int seconds);

private:
    void keepUpdated();
    std::atomic<bool> keepRunningBool_;
    std::thread updatingThread_;
    std::condition_variable conditionVariable_;
    std::shared_ptr<ElPricesStorageController> storageController_;
};



#endif //ELPRICESCOLLECTOR_H
