#ifndef __TOOLS__
#define __TOOLS__

#include "types.hpp"

// string helpers
uint64_t bytesToU64(std::string ts);
std::string stringToHexString(std::string ts);
bool isIntString(std::string ts);
bool isFloatString(std::string ts);

// video time helpers
VidTime getVidTime(UTCus starttime, UTCus curtime);
uint64_t getVidTimeSeconds(VidTime tv);
bool isValidVidTime(VidTime tv);
bool timeStringToVidTime(std::string ts, VidTime *tv);

// klv
UTCus getPacketTime(KLV *packet);

#endif // __TOOLS__
