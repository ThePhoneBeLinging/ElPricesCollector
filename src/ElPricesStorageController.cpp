//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>

#include "Utility//TimeUtil.h"
#include "Utility/Utility.h"


ElPricesStorageController::ElPricesStorageController() : db_(std::make_unique<SQLite::Database>("../../Resources/historicData.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, memoryDB_(std::make_unique<SQLite::Database>(":memory",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
{
    memoryDB_->exec("DROP TABLE IF EXISTS Prices");
    memoryDB_->exec("""CREATE TABLE \"Prices\" ("
    "\"ID\"	INTEGER NOT NULL UNIQUE,"
    "\"Raw\"	INTEGER NOT NULL,"
    "\"Fee\"INTEGER NOT NULL,"
    "\"Time\"	TEXT NOT NULL UNIQUE,"
    "PRIMARY KEY(\"ID\" AUTOINCREMENT)"
    ");"
    );
}

void ElPricesStorageController::insertHourPriceToDB(const std::string& dateStringWithHour, const std::shared_ptr<HourPrice>& hourPrice)
{
    try
    {
        std::string sqlInsert = "INSERT OR IGNORE INTO Prices(Raw,Fee,Time) VALUES (" + std::to_string(hourPrice->getPriceWithoutFees()) +  ", " + std::to_string(hourPrice->getFees()) + ",'" + dateStringWithHour + "');";
        memoryDB_->exec(sqlInsert);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    copyToFileDataBase();
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

        int priceWithoutTransport = std::stoi(priceLine[1]);
        int currentPriceOfTransport = std::stoi(priceLine[2]);
        int ceriusFees = 0;
        int total = std::stoi(priceLine[3]);
        std::string dateString = priceLine[4];
        int hour = std::stoi(priceLine[5]);
        dateString += "-" + std::to_string(hour);
        // TODO Take Cerius prices into account :(
        auto hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,ceriusFees);

        insertHourPriceToDB(dateString,hourPrice);
    }
}

void ElPricesStorageController::copyToFileDataBase()
{
    try
    {
        // TODO Update statement below to perhaps only attempt today.
        // This will require the program to run, at least once, per day. But will save a lot of read/write.
        // Before release, i should probably just keep the database in memory.
        SQLite::Statement   selectionQuery(*memoryDB_, "SELECT * FROM Prices");

        while (selectionQuery.executeStep())
        {
            int id = selectionQuery.getColumn(0).getInt();
            int priceWithoutFees = selectionQuery.getColumn(1).getInt();
            int fee = selectionQuery.getColumn(2).getInt();
            std::string time = selectionQuery.getColumn(3).getString();

            SQLite::Statement sqlInsertStatement(*db_,"INSERT OR IGNORE INTO Prices(Raw,Fee,Time) VALUES (?,?,?);");
            sqlInsertStatement.bind(1,priceWithoutFees);
            sqlInsertStatement.bind(2,fee);
            sqlInsertStatement.bind(3,time);
            sqlInsertStatement.exec();
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
