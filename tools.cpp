#include "tools.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

///////////////////////////////////////////////
//  STRING HELPERS

uint64_t bytesToU64(std::string ts)
{
    uint64_t result = 0;
    int len = int(ts.length());

    for(int n = len-1; n >= 0; n--)
        result = result | ( (uint64_t)((unsigned char)ts[n]) << ((len-1-n)*8) );

    return result;
}

std::string stringToHexString(std::string ts)
{
    std::stringstream hexss;

    for(int i = 0; i < int(ts.length()); i++)
    {
        unsigned char c = ts[i];

        hexss << std::hex << std::setw(2) << std::setfill('0') << int(c);
        if(i != int(ts.length()-1) ) hexss << " ";
    }

    return hexss.str();
}

bool isIntString(std::string ts)
{
    for(int i = 0; i < int(ts.length()); i++)
    {
        if( int(ts[i]) < int('0') || int(ts[i]) > int('9')  ) return false;
    }
    return true;
}

bool isFloatString(std::string ts)
{
    bool foundprecision = false;

    for(int i = 0; i < int(ts.length()); i++)
    {
        if( int(ts[i]) < int('0') || int(ts[i]) > int('9')  )
        {
            if(ts[i] == '.')
            {
                if(foundprecision) return false;
                else foundprecision = true;
            }
            else return false;
        }
    }
    return true;
}

///////////////////////////////////////////////
//  VIDEO TIME HELPERS

VidTime getVidTime(UTCus starttime, UTCus curtime)
{
    // goofy
    if(starttime > curtime)
    {
        UTCus t = starttime;
        starttime = curtime;
        curtime = t;
    }

    VidTime ttime;
    ttime.hh = 0;
    ttime.mm = 0;
    ttime.ss = 0.0;

    UTCus deltat = curtime - starttime;


    uint64_t totseconds = deltat/1000000;

    ttime.hh = totseconds / 3600;
    ttime.mm = (totseconds - ttime.hh*3600)/60;
    ttime.ss = (deltat * 0.000001) - ttime.hh*3600 - ttime.mm*60;


    return ttime;

}

uint64_t getVidTimeSeconds(VidTime tv)
{
    uint64_t result = int(tv.ss);
    result += tv.mm * 60;
    result += tv.hh * 60 * 60;
    return result;
}

bool isValidVidTime(VidTime tv)
{
    if(tv.ss < 0 || tv.ss > 59.9999) return false;
    if(tv.mm < 0 || tv.mm > 59) return false;

    return true;
}

bool timeStringToVidTime(std::string ts, VidTime *tv)
{
    VidTime tempvid;
    std::string hstr, mstr, sstr;
    int len;
    size_t pos = 0;

    if(!tv) return false;

    // get hours
    len = int(ts.length());
    pos = ts.find_first_of(':');
    if(pos == std::string::npos) return false;
    hstr = ts.substr(0, pos);
    if(!isIntString(hstr)) return false;
    if(int(pos+1) >= len) return false;
    ts.erase(0, pos+1);
    tempvid.hh = atoi(hstr.c_str());

    // get minutes
    len = int(ts.length());
    pos = ts.find_first_of(':');
    if(pos == std::string::npos) return false;
    mstr = ts.substr(0, pos);
    if(!isIntString(mstr)) return false;
    if(int(pos+1) >= len) return false;
    ts.erase(0, pos+1);
    tempvid.mm = atoi(mstr.c_str());

    // get seconds
    if(!isFloatString(ts)) return false;
    tempvid.ss = atof(ts.c_str());

    if( !isValidVidTime(tempvid)) return false;

    *tv = tempvid;

    return true;
}

///////////////////////////////////////////////
//  KLV HELPERS

UTCus getPacketTime(KLV *packet)
{
    int index = 0;

    if(!packet) return 0;

    // check the data
    while(index < int(packet->val.length()) )
    {
        int tag = int( (unsigned char)(packet->val[index]) );
        int tlen = int( (unsigned char)(packet->val[index+1]) );
        std::string tdat = packet->val.substr( index+2, tlen);
        std::string tstr = stringToHexString(tdat);

        // if timestamp tag is found, process packet/video time
        if(tag == 2) return bytesToU64(tdat);

        index += tlen + 2;
    }

    // timestamp not found?
    return 0;
}
