#include "Timestamp.h"

#include <time.h>
#include <string>
// struct tm {
//     int tm_sec;    // 秒：0~60
//     int tm_min;    // 分：0~59
//     int tm_hour;   // 时：0~23
//     int tm_mday;   // 一个月中的第几天：1~31
//     int tm_mon;    // 月份：0~11
//     int tm_year;   // 从1900年开始经过的年数
//     int tm_wday;   // 星期：0~6，0表示星期日
//     int tm_yday;   // 一年中的第几天：0~365
//     int tm_isdst;  // 夏令时标志
// };

Timestamp::Timestamp() 
: SecondsSinceEpoch_(0) 
{

}

Timestamp::Timestamp(int64_t SecondsSinceEpoch) 
: SecondsSinceEpoch_(SecondsSinceEpoch) 
{

}

Timestamp Timestamp::now() { return Timestamp(time(nullptr)); }

std::string Timestamp::toString() const {
    tm tm_time{};
    localtime_r(&SecondsSinceEpoch_, &tm_time);

    //格式化时间字符串
    char buf[128] = {0};
    snprintf(buf,
        sizeof(buf),
        "%04d-%02d-%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900,
        tm_time.tm_mon + 1,
        tm_time.tm_mday,
        tm_time.tm_hour,
        tm_time.tm_min,
        tm_time.tm_sec);
    return buf;
}

// int main() 
// {
//     std::cout << "现在的时间是:" << Timestamp::now().toString() << std::endl;
//     return 0;
// }
