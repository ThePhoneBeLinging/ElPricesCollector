//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESSTORAGECONTROLLER_H
#define ELPRICESSTORAGECONTROLLER_H

#include "PriceObjects/Date.h"
#include "SQLiteCpp/Column.h"
#include "SQLiteCpp/Database.h"


class ElPricesStorageController
{
public:
    ElPricesStorageController();
    ~ElPricesStorageController();
    void insertHourPriceToDB(const std::string& dateStringWithHour, const std::shared_ptr<HourPrice>& hourPrice);
    void handleParsedData(const std::string& parsedData);
private:
    SQLite::Database db_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
