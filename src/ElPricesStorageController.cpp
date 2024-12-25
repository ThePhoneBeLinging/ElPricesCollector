//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesStorageController.h"

void ElPricesStorageController::storeDate(const std::string& dateKey, const std::shared_ptr<Date>& date)
{
    datesMap_[dateKey] = date;
}

std::shared_ptr<Date> ElPricesStorageController::getDate(const std::string& dateKey)
{
    return datesMap_[dateKey];
}
