//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef DATE_H
#define DATE_H
#include <memory>
#include <vector>

#include "HourPrice.h"


class Date
{
public:
    Date();
    void setPriceAtPoint(int hour, const std::shared_ptr<HourPrice>& hourPrice);
    [[nodiscard]] std::unique_ptr<HourPrice> getPriceAtPoint(int hour) const;
    bool isDateComplete() const;
private:
    std::vector<std::unique_ptr<HourPrice>> hourPrices_;
};



#endif //DATE_H
