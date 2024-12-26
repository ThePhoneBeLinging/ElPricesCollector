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
    void storeDate(const std::string& dateKey, const std::shared_ptr<Date>& date);
    std::shared_ptr<Date> getDate(const std::string& dateKey);
    void handleParsedData(const std::string& parsedData);
private:
    std::unordered_map<std::string,std::shared_ptr<Date>> datesMap_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
