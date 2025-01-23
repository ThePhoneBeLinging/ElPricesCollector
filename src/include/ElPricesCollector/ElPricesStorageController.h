//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ELPRICESSTORAGECONTROLLER_H
#define ELPRICESSTORAGECONTROLLER_H

#include "PriceObjects/Date.h"
#include "../../FeeController.h"
#include "SQLiteCpp/Database.h"


class ElPricesStorageController
{
public:
    ElPricesStorageController();
    ~ElPricesStorageController() = default;
    void insertHourPriceToDB(const std::string& dateStringWithHour, int hour, const std::shared_ptr<HourPrice>& hourPrice);
    std::shared_ptr<HourPrice> getHourPriceFromMemoryDB(const std::string& dateString, int hour) const;
    std::vector<std::shared_ptr<HourPrice>> getCurrentAndFuturePrices();
    void handleParsedData(const std::string& parsedData);
    void copyToFileDataBase() const;
    void initMemoryDBFromFile() const;
    void reloadFees();
private:
    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;
    std::unique_ptr<FeeController> feeController_;
};



#endif //ELPRICESSTORAGECONTROLLER_H
