//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesCollector/ElPricesStorageController.h"

#include <iostream>
#include <sstream>

#include "Utility//TimeUtil.h"
#include "Utility/Utility.h"

ElPricesStorageController::ElPricesStorageController()
{

    std::vector<std::string> datesToRead = TimeUtil::getStringsForTmrwAnd14DaysBack();
    std::vector<std::vector<std::string>> stringMatrix;

    for (const auto& fileName : datesToRead)
    {
        std::string properFileName = "../../HistoricData/Prices/" + fileName + ".csv";
        std::string fileContent = Utility::readFromFile(properFileName);
        std::stringstream outerStream(fileContent);
        std::string parsedString;
        while (getline(outerStream,parsedString,'\n'))
        {
            std::stringstream innerStream(parsedString);
            std::string innerParsed;
            std::vector<std::string> stringVector;
            stringVector.push_back(fileName);
            while (getline(innerStream,innerParsed,','))
            {
                stringVector.push_back(innerParsed);
            }
            stringMatrix.push_back(stringVector);
        }
    }

    for (const auto& vector : stringMatrix)
    {
        std::string dateString = vector[0];
        int hour = std::stoi(vector[1]);
        int priceWithoutFees = std::stoi(vector[2]);
        int fees = std::stoi(vector[3]);
        if (datesMap_[dateString] == nullptr)
        {
            auto date = std::make_shared<Date>();
            datesMap_[dateString] = date;
        }
        auto hourPrice = std::make_shared<HourPrice>(priceWithoutFees,fees);
        datesMap_[dateString]->setPriceAtPoint(hour,hourPrice);
    }
}

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
        int ceriusFees = 0;
        int total = std::stoi(priceLine[3]);
        std::string dateString = priceLine[4];
        int hour = std::stoi(priceLine[5]);
        // TODO Take Cerius prices into account :(
        auto hourPrice = std::make_shared<HourPrice>(priceWithoutTransport,ceriusFees);
        if (datesMap_[dateString] == nullptr)
        {
            auto date = std::make_shared<Date>();
            datesMap_[dateString] = date;
        }
        if (datesMap_[dateString]->getPriceAtPoint(hour) == nullptr)
        {
            std::string savePrice = std::to_string(hour) + "," + std::to_string(priceWithoutTransport) + "," + std::to_string(ceriusFees) + "\n";
            Utility::appendToFile("../../HistoricData/Prices/" + dateString + ".csv",savePrice);
            datesMap_[dateString]->setPriceAtPoint(hour,hourPrice);
        }
    }
}
