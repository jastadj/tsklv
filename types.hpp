#ifndef __TYPES__
#define __TYPES__

#include <cstdlib>
#include <string>

#define METERS_TO_FEET 3.28084

// UTC time in microseconds
typedef uint64_t UTCus;

// video time (hh::mm::ss.ss)
struct VidTime
{
    int hh;
    int mm;
    float ss;

    bool operator<(const VidTime &tv) const
    {
        if(hh != tv.hh) return hh < tv.hh;
        if(mm != tv.mm) return mm < tv.mm;
        return ss < tv.ss;
    }

    bool operator<=(const VidTime &tv) const
    {
        if(hh != tv.hh) return hh <= tv.hh;
        if(mm != tv.mm) return mm <= tv.mm;
        return ss <= tv.ss;
    }

    bool operator>(const VidTime &tv) const
    {
        if(hh != tv.hh) return hh > tv.hh;
        if(mm != tv.mm) return mm > tv.mm;
        return ss > tv.ss;
    }

    bool operator>=(const VidTime &tv) const
    {
        if(hh != tv.hh) return hh >= tv.hh;
        if(mm != tv.mm) return mm >= tv.mm;
        return ss >= tv.ss;
    }
};

// KLV packet container
struct KLV
{
    std::string key;
    int len;
    std::string val;
};

#endif // __TYPES__
