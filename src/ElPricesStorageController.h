//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESSTORAGECONTROLLER_H
#define ELPRICESSTORAGECONTROLLER_H
#include <unordered_map>

#include "PriceObjects/Date.h"


class ElPricesStorageController
{
public:
    ElPricesStorageController() = default;
private:
    std::unordered_map<std::string,std::shared_ptr<Date>> datesMap_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
