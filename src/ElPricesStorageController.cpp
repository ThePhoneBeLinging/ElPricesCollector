//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>

#include "Utility/TimeUtil.h"
#include "Utility/Utility.h"


ElPricesStorageController::ElPricesStorageController() : db_(std::make_unique<SQLite::Database>("../../HistoricData/Prices.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, memoryDB_(std::make_unique<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, feeController_(std::make_unique<FeeController>())
{
    std::cout << "ElPricesStorageController::ElPricesStorageController()" << std::endl;
    // This part of the constructor creates a Table with the same specifications of the file-based DB
    std::string query = "SELECT sql FROM sqlite_master WHERE type='table' AND name='Prices';";

    SQLite::Statement queryStmt(*db_, query);
    if (queryStmt.executeStep())
    {
        std::string createTableSQL = queryStmt.getColumn(0).getString();
        memoryDB_->exec(createTableSQL);
    }
    feeController_->loadFeesFromFile();
}

void ElPricesStorageController::insertHourPriceToDB(const std::string& dateStringWithHour, int hour, const std::shared_ptr<HourPrice>& hourPrice)
{
    try
    {
        SQLite::Statement sqlInsertStatement(*memoryDB_,"INSERT OR IGNORE INTO Prices(Raw,Fee,Date,Hour) VALUES (?,?,?,?);");
        sqlInsertStatement.bind(1,hourPrice->getPriceWithoutFees());
        sqlInsertStatement.bind(2,hourPrice->getFees());
        sqlInsertStatement.bind(3,dateStringWithHour);
        sqlInsertStatement.bind(4,hour);
        sqlInsertStatement.exec();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

std::shared_ptr<HourPrice> ElPricesStorageController::getHourPriceFromMemoryDB(const std::string& dateString, int hour) const
{
    try
    {
        SQLite::Statement selectStatement(*memoryDB_, "SELECT * FROM Prices WHERE Date == ? AND Hour == ?");
        selectStatement.bind(1,dateString);
        selectStatement.bind(2,hour);

        while (selectStatement.executeStep())
        {
            int id = selectStatement.getColumn(0).getInt();
            int rawPrice = selectStatement.getColumn(1).getInt();
            int fee = selectStatement.getColumn(2).getInt();

            return std::make_shared<HourPrice>(rawPrice,fee);
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return nullptr;
}

void ElPricesStorageController::handleParsedData(const std::string& parsedData)
{
    // Split by \n, then by "
    std::string parsedString;
    std::stringstream stringStream(parsedData);
    std::vector<std::vector<std::string>> matrixStrings;

    while (getline(stringStream,parsedString,'\n'))
    {
        std::vector<std::string> stringList;
        std::stringstream innerStringStream(parsedString);
        std::string innerParsedString;

        while (getline(innerStringStream,innerParsedString,'"'))
        {
            if (innerParsedString.empty() || innerParsedString == ",")
            {
                continue;
            }
            stringList.push_back(innerParsedString);
        }
        matrixStrings.push_back(stringList);
    }

    bool first = true;
    for (auto& priceLine : matrixStrings)
    {
        // We skip the first line that describes the csv format.
        if (first)
        {
            first = false;
            continue;
        }
        std::erase(priceLine[0],' ');
        std::stringstream stream(priceLine[0]);
        std::string parsedDateStringLine;
        while (getline(stream,parsedDateStringLine,'-'))
        {
            priceLine.push_back(parsedDateStringLine);
        }

        stream = std::stringstream(priceLine[5]);
        bool innerFirst = true;
        while (getline(stream,parsedDateStringLine,':'))
        {
            if (innerFirst)
            {
                priceLine[5] = parsedDateStringLine;
                innerFirst = false;
            }
        }

        std::erase(priceLine[1],',');
        std::erase(priceLine[2],',');
        std::erase(priceLine[3],',');

        int priceWithoutTransport = std::stoi(priceLine[1]) * 100;
        int currentPriceOfTransport = std::stoi(priceLine[2]);
        int total = std::stoi(priceLine[3]);
        std::string dateString = priceLine[4];
        std::string monthString;
        std::stringstream monthStream(dateString);
        int index = 0;
        while (getline(monthStream,monthString,'.'))
        {
            if (index == 1)
            {
                break;
            }
            index++;
        }
        int month = std::stoi(monthString);
        int hour = std::stoi(priceLine[5]);
        // TODO Take Cerius prices into account :(
        int ceriusFees = feeController_->getFeesFromDate(month,hour);
        auto hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,ceriusFees);

        insertHourPriceToDB(dateString, hour, hourPrice);
    }
    copyToFileDataBase();
}

void ElPricesStorageController::copyToFileDataBase() const
{
    try
    {
        std::string todayLookupString = TimeUtil::getCurrentTimeAsDateString();
        std::string tmrwLookupString = TimeUtil::timeToStringForLookup(TimeUtil::getTommorowTime());

        SQLite::Statement selectionQuery(*memoryDB_, "SELECT * FROM Prices WHERE Date != ? AND Date != ?");
        selectionQuery.bind(1,todayLookupString);
        selectionQuery.bind(2,tmrwLookupString);

        while (selectionQuery.executeStep())
        {
            int id = selectionQuery.getColumn(0).getInt();
            int priceWithoutFees = selectionQuery.getColumn(1).getInt();
            int fee = selectionQuery.getColumn(2).getInt();
            std::string dateString = selectionQuery.getColumn(3).getString();
            int hour = selectionQuery.getColumn(4).getInt();

            SQLite::Statement sqlInsertStatement(*db_,"INSERT OR IGNORE INTO Prices(Raw,Fee,Date,Hour) VALUES (?,?,?,?);");
            sqlInsertStatement.bind(1,priceWithoutFees);
            sqlInsertStatement.bind(2,fee);
            sqlInsertStatement.bind(3,dateString);
            sqlInsertStatement.bind(4,hour);
            sqlInsertStatement.exec();

            SQLite::Statement memoryDeleteStatement(*memoryDB_,"DELETE FROM Prices WHERE Date == ?");
            memoryDeleteStatement.bind(1,id);
            memoryDeleteStatement.exec();
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void ElPricesStorageController::reloadFees()
{
    feeController_->loadFeesFromFile();
}
