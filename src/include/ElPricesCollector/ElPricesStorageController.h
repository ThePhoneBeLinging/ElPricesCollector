//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESSTORAGECONTROLLER_H
#define ELPRICESSTORAGECONTROLLER_H
#include <sqlite3.h>

#include "PriceObjects/Date.h"


class ElPricesStorageController
{
public:
    ElPricesStorageController();
    ~ElPricesStorageController();
    void insertHourPriceToDB(const std::string& dateStringWithHour, const std::shared_ptr<HourPrice>& hourPrice);
    void handleParsedData(const std::string& parsedData);
private:
    sqlite3* db_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
