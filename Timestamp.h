#pragma once
#include <iostream>
#include <string>
#include <cstdint>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t SecondsSinceEpoch);

    static Timestamp now();
    std::string toString() const;

private:
    int64_t SecondsSinceEpoch_;
};