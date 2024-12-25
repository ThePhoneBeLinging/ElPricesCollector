//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESCOLLECTOR_H
#define ELPRICESCOLLECTOR_H

#include <atomic>
#include <thread>

class ElPricesCollector
{
public:
    ElPricesCollector();
    ~ElPricesCollector();

private:
    void keepUpdated();
    std::atomic<bool> keepRunningBool_;
    std::thread updatingThread_;
};



#endif //ELPRICESCOLLECTOR_H
