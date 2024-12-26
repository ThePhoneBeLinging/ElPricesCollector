//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>

void ElPricesStorageController::storeDate(const std::string& dateKey, const std::shared_ptr<Date>& date)
{
    datesMap_[dateKey] = date;
}

std::shared_ptr<Date> ElPricesStorageController::getDate(const std::string& dateKey)
{
    std::lock_guard lockGuard(mutex_);
    return datesMap_[dateKey];
}

void ElPricesStorageController::handleParsedData(const std::string& parsedData)
{
    std::lock_guard lockGuard(mutex_);
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
        int total = std::stoi(priceLine[3]);
        std::string dateString = priceLine[4];
        int hour = std::stoi(priceLine[5]);
        // TODO Take Cerius prices into account :(
        auto hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,0);
        if (datesMap_[dateString] == nullptr)
        {
            auto date = std::make_shared<Date>();
            datesMap_[dateString] = date;
        }
        datesMap_[dateString]->setPriceAtPoint(hour,hourPrice);
    }
}
