//
// Created by eal on 12/31/24.
//

#ifndef FEECONTROLLER_H
#define FEECONTROLLER_H
#include <memory>

#include "CeriusFees/Fee.h"


class FeeController
{
public:
    FeeController();
    ~FeeController() = default;
    [[nodiscard]] int getCurrentFees();
    [[nodiscar]] int getFeesFromDate(int month, int hour);
    void loadFeesFromFile();

private:
    std::mutex mutex_;
    std::set<int> monthsWithSummerFees_;
    std::set<int> monthsWithWinterFees_;
    std::unique_ptr<Fee> summerFees_;
    std::unique_ptr<Fee> winterFees_;
};



#endif //FEECONTROLLER_H
