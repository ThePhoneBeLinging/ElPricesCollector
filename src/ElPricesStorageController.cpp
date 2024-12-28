//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>

#include "Utility//TimeUtil.h"
#include "Utility/Utility.h"
#include "sqlite3.h"

static int callback(void* data, int argc, char** argv, char** azColName)
{
    int i;
    fprintf(stderr, "%s: ", (const char*)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

ElPricesStorageController::ElPricesStorageController() : db_()
{
    int dataBaseLoadStatus = sqlite3_open("../../Resources/historicData.db",&db_);
    if(dataBaseLoadStatus != SQLITE_OK)
    {
        throw std::invalid_argument("Failed to open database");
    }
}

ElPricesStorageController::~ElPricesStorageController()
{
    sqlite3_close(db_);
}

void ElPricesStorageController::insertHourPriceToDB(const std::string& dateStringWithHour, const std::shared_ptr<HourPrice>& hourPrice)
{
    char * errorMessage;
    std::string sqlInsert = "INSERT OR IGNORE INTO Prices(Raw,Fee,Time) VALUES (" + std::to_string(hourPrice->getPriceWithoutFees()) +  ", " + std::to_string(hourPrice->getFees()) + ",'" + dateStringWithHour + "');";
    sqlite3_exec(db_, sqlInsert.c_str(), nullptr, nullptr, &errorMessage);
    std::cout << errorMessage << std::endl;
    free(errorMessage);
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
