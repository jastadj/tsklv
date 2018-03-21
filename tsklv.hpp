#ifndef CLASS_TSKLV
#define CLASS_TSKLV

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include <map>

#include "types.hpp"
#include "tools.hpp"
#include "tagitem.hpp"
#include "kml.hpp"

#ifndef VER
#define VER "dbg"
#endif

// version history
// v1.0.0 initial
// v1.0.1 parse security tags
// v1.0.2 fix version option
// v1.0.3 handle case where packets are split up
// v1.0.4 increase nav angle precision


enum _PRATEFILTER{P_ALL, P_SLOW, P_FAST};

class TSKLV
{
private:

    std::string getVersionString();
    void printAbout() {std::cout << "tsklv " << getVersionString() << std::endl;}

    // input
    std::string m_TargetFile;
    VidTime m_TargetTime;
    VidTime m_TargetTimeEnd; // when using a start and stop time
    bool m_UseStartAndStopTime;
    bool m_DoGenerateGoogleMapsLink;
    bool m_ShowTagsRaw;
    bool m_ShowKLVRaw;
    bool m_DoScreenshot;
    _PRATEFILTER m_PacketRateFilter;
    bool m_DoGenerateCSV;
    bool m_DoGenerateKML;
    bool m_QuietMode;


    // output
    std::string m_ScreenshotName;
    std::string m_TargetCSVFilename;
    std::ofstream m_CSVFile;
    KML m_KML;

    // initial timestamps
    UTCus m_UTCusStartTime;
    UTCus m_UTCusMISMStartTime;

    // klv functions
    void printKLV(KLV tklv);
    std::map<int, TagItem*> m_Taglist;
    std::map<int,int> m_TaglistIndex;


    void takeScreenshotAtVideoTime(VidTime tt);

    // command line feedback
    void showHelp();
    void invalidParameter();
    bool isFFMPEGPresent();

    void done();

public:
    TSKLV(int argc, char *argv[]);
    ~TSKLV() {};


};
#endif // CLASS_TSKLV
