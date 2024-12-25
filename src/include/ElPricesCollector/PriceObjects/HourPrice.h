//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef HOURPRICE_H
#define HOURPRICE_H



class HourPrice
{
public:
    HourPrice(int priceWithoutFees, int fees);
    [[nodiscard]] int getPriceWithoutFees() const;
    [[nodiscard]] int getFees() const;

private:
    int priceWithoutFees_;
    int fees_;
};



#endif //HOURPRICE_H
