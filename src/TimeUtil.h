//
// Created by Elias Aggergaard Larsen on 20/11/2024.
//

#ifndef TIMEUTIL_H
#define TIMEUTIL_H
#include <chrono>
#include <ctime>


class TimeUtil
{
public:
    static std::string getCurrentTimeAsDateString();
    static tm getCurrentTime();
    static tm getTommorowTime();
    static std::string timeToStringForAPI(const tm& time);
    static std::string timeToStringForLookup(const tm& time);
private:
    static tm timeToTM(std::chrono::time_point<std::chrono::system_clock> time);
};



#endif //TIMEUTIL_H
