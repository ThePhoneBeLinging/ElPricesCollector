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
    ~ElPricesStorageController() = default;
    void insertHourPriceToDB(const std::string& dateStringWithHour, int hour, const std::shared_ptr<HourPrice>& hourPrice);
    void handleParsedData(const std::string& parsedData);
    void copyToFileDataBase() const;
private:
    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
