//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESCOLLECTOR_H
#define ELPRICESCOLLECTOR_H

#include <atomic>
#include <thread>

#include "../../ElPricesStorageController.h"

class ElPricesCollector
{
public:
    ElPricesCollector();
    ~ElPricesCollector();

    std::shared_ptr<ElPricesStorageController>& getStorageController();

private:
    void keepUpdated();
    std::atomic<bool> keepRunningBool_;
    std::thread updatingThread_;
    std::shared_ptr<ElPricesStorageController> storageController_;
};



#endif //ELPRICESCOLLECTOR_H
