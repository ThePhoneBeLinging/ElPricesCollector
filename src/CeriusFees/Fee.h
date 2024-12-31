//
// Created by eal on 12/31/24.
//

#ifndef FEE_H
#define FEE_H
#include <set>
#include <string>
#include <vector>


class Fee
{
public:
    Fee() = default;
    ~Fee() = default;
    [[nodiscard]] int getCurrentFee(int hour) const;

    void setLowLoadFee(int fee);
    void setHighLoadFee(int fee);
    void setPeakLoadFee(int fee);
    void setLowLoadHours(std::set<int> hours);
    void setHighLoadHours(std::set<int> hours);
    void setPeakLoadHours(std::set<int> hours);

private:
    std::set<int> lowLoadHours_;
    std::set<int> highLoadHours_;
    std::set<int> peakLoadHours_;
    int lowLoadFee_;
    int highLoadFee_;
    int peakLoadFee_;
};



#endif //FEE_H
