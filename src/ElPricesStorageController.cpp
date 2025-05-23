//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>
#include "DatabaseAccessController/DatabaseAccessController.h"
#include "Utility/ConfigController.h"
#include "Utility/DebugController.h"
#include "Utility/TimeUtil.h"
#include "Utility/Utility.h"


ElPricesStorageController::ElPricesStorageController() : db_(std::make_shared<SQLite::Database>("../../HistoricData/Prices.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, memoryDB_(std::make_shared<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, feeController_(std::make_unique<FeeController>())
{
    std::cout << "ElPricesStorageController::ElPricesStorageController()" << std::endl;

    DatabaseAccessController::addDatabase(db_,"PRICEDB");
    DatabaseAccessController::addDatabase(memoryDB_,"priceMemoryDB");

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
        auto memoryDB = DatabaseAccessController::getDatabase("priceMemoryDB");
        std::string query = "INSERT INTO Prices (Raw, Fee, Date, Hour) "
                  "VALUES (?, ?, ?, ?) "
                  "ON CONFLICT(Date, Hour) DO UPDATE SET "
                  "Fee = excluded.Fee;";
        SQLite::Statement sqlInsertStatement(*memoryDB->getDatabase(),query);
        sqlInsertStatement.bind(1,hourPrice->getPriceWithoutFees());
        sqlInsertStatement.bind(2,hourPrice->getFees());
        sqlInsertStatement.bind(3,dateStringWithHour);
        sqlInsertStatement.bind(4,hour);
        sqlInsertStatement.exec();
    }
    catch (const std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("Error occured in: InsertHourPriceToDB " + debugText);
    }
}

std::shared_ptr<HourPrice> ElPricesStorageController::getHourPriceFromMemoryDB(const std::string& dateString, int hour) const
{
    try
    {
        auto memoryDB = DatabaseAccessController::getDatabase("priceMemoryDB");
        SQLite::Statement selectStatement(*memoryDB->getDatabase(), "SELECT * FROM Prices WHERE Date == ? AND Hour == ?");
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
        std::cout << "GetHourPriceFromMemoryDB " << e.what() << std::endl;
    }
    return std::make_shared<HourPrice>(0,0);
}

std::vector<std::shared_ptr<HourPrice>> ElPricesStorageController::getCurrentAndFuturePrices()
{
    try
    {
        auto dateString = TimeUtil::timeToStringForLookup(TimeUtil::getCurrentTime());
        auto tmrwDateString = TimeUtil::timeToStringForLookup(TimeUtil::getTommorowTime());
        auto currentTime = TimeUtil::getCurrentTime();
        std::vector<std::shared_ptr<HourPrice>> hourPrices;
        hourPrices.resize(48);

        auto memoryDB = DatabaseAccessController::getDatabase("priceMemoryDB");
        SQLite::Statement selectStatement(*memoryDB->getDatabase(), "SELECT * FROM Prices WHERE Date == ? OR Date == ?");
        selectStatement.bind(1,dateString);
        selectStatement.bind(2,tmrwDateString);

        while (selectStatement.executeStep())
        {
            int id = selectStatement.getColumn(0).getInt();
            int rawPrice = selectStatement.getColumn(1).getInt();
            int fee = selectStatement.getColumn(2).getInt();
            std::string date = selectStatement.getColumn(3).getString();
            int hour = selectStatement.getColumn(4).getInt();

            auto price = std::make_shared<HourPrice>(rawPrice,fee);
            int multiplier = 0;
            if (date == tmrwDateString)
            {
                multiplier = 1;
            }
            hourPrices[hour + (multiplier * 24)] = price;
        }
        return hourPrices;
    }
    catch (const std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("GetCurrentAndFuturePrices: " + debugText);
    }
    return {};
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
        int ceriusFees = feeController_->getFeesFromDate(month,hour);
        std::shared_ptr<HourPrice> hourPrice;
        if (ConfigController::getConfigBool("UseRadiusToCeriusPriceDifference"))
        {
            std::string keyForFeeLookup = "RadiusToCerius_Hour:" + std::to_string(hour);
            int currentFees = currentPriceOfTransport + ConfigController::getConfigInt(keyForFeeLookup);
            currentFees *= 100;
            hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,currentFees);
        }
        else
        {
            hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,ceriusFees);
        }

        insertHourPriceToDB(dateString, hour, hourPrice);
    }
    copyToFileDataBase();
}

void ElPricesStorageController::copyToFileDataBase() const
{
    auto dbLock = DatabaseAccessController::getDatabase("PRICEDB");
    try
    {
        std::string todayLookupString = TimeUtil::getCurrentTimeAsDateString();
        std::string tmrwLookupString = TimeUtil::timeToStringForLookup(TimeUtil::getTommorowTime());

        auto memoryDB = DatabaseAccessController::getDatabase("priceMemoryDB");
        SQLite::Statement selectionQuery(*memoryDB->getDatabase(), "SELECT * FROM Prices");

        while (selectionQuery.executeStep())
        {
            int id = selectionQuery.getColumn(0).getInt();
            int priceWithoutFees = selectionQuery.getColumn(1).getInt();
            int fee = selectionQuery.getColumn(2).getInt();
            std::string dateString = selectionQuery.getColumn(3).getString();
            int hour = selectionQuery.getColumn(4).getInt();
            std::string query = "INSERT INTO Prices (Raw, Fee, Date, Hour) "
                  "VALUES (?, ?, ?, ?) "
                  "ON CONFLICT(Date, Hour) DO UPDATE SET "
                  "Fee = excluded.Fee;";
            SQLite::Statement sqlInsertStatement(*dbLock->getDatabase(),query);
            sqlInsertStatement.bind(1,priceWithoutFees);
            sqlInsertStatement.bind(2,fee);
            sqlInsertStatement.bind(3,dateString);
            sqlInsertStatement.bind(4,hour);
            sqlInsertStatement.exec();
        }

        SQLite::Statement deleteStatement(*memoryDB->getDatabase(),"DELETE FROM Prices WHERE Date != ? AND Date != ?");
        deleteStatement.bind(1,todayLookupString);
        deleteStatement.bind(2,tmrwLookupString);
        deleteStatement.exec();

    }
    catch (std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("CopyToFileDataBase(): " + debugText);
    }
}

void ElPricesStorageController::initMemoryDBFromFile() const
{
    auto dbLock = DatabaseAccessController::getDatabase("PRICEDB");
    auto memoryDB = DatabaseAccessController::getDatabase("priceMemoryDB");
    try
    {

        std::string todayLookupString = TimeUtil::getCurrentTimeAsDateString();
        std::string tmrwLookupString = TimeUtil::timeToStringForLookup(TimeUtil::getTommorowTime());

        SQLite::Statement selectionQuery(*dbLock->getDatabase(), "SELECT * FROM Prices WHERE Date == ? OR Date == ?");
        selectionQuery.bind(1,todayLookupString);
        selectionQuery.bind(2,tmrwLookupString);

        while (selectionQuery.executeStep())
        {
            int id = selectionQuery.getColumn(0).getInt();
            int priceWithoutFees = selectionQuery.getColumn(1).getInt();
            int fee = selectionQuery.getColumn(2).getInt();
            std::string dateString = selectionQuery.getColumn(3).getString();
            int hour = selectionQuery.getColumn(4).getInt();

            SQLite::Statement sqlInsertStatement(*memoryDB->getDatabase(),"INSERT OR IGNORE INTO Prices(Raw,Fee,Date,Hour) VALUES (?,?,?,?);");
            sqlInsertStatement.bind(1,priceWithoutFees);
            sqlInsertStatement.bind(2,fee);
            sqlInsertStatement.bind(3,dateString);
            sqlInsertStatement.bind(4,hour);
            sqlInsertStatement.exec();
        }
    }
    catch (std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("InitMemoryDBFromFile(): " + debugText);
    }
}

void ElPricesStorageController::reloadFees()
{
    feeController_->loadFeesFromFile();
}
