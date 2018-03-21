#include "tsklv.hpp"
#include "math.h"
#include <vector>

TSKLV::TSKLV(int argc, char *argv[])
{

    std::stringstream myss;
    std::ifstream ifile;

    // init vars
    m_ScreenshotName = "screenshot.jpg";
    m_ShowTagsRaw = false;
    m_ShowKLVRaw = false;
    m_UseStartAndStopTime = false;
    m_DoGenerateGoogleMapsLink = false;
    m_DoScreenshot = true;
    m_PacketRateFilter = P_ALL;
    m_DoGenerateCSV = false;
    m_DoGenerateKML = false;
    m_QuietMode = false;

    // zero initial timestamps
    m_UTCusStartTime = 0;
    m_UTCusMISMStartTime = 0;

    // flag VidTime vars as not set yet
    m_TargetTime.hh = -1;
    m_TargetTimeEnd.hh = -1;

    // slow packet variables
    std::vector<KLV> prev_slowpackets;
    std::vector<KLV> cur_slowpackets;
    UTCus prev_slowtime = 0;

    std::string binfile;

    // multiple MISM packets at same UTC
    std::vector<KLV> packetsAtTargetTime;


    // handle command line arguments
    for(int i = 1; i < argc; i++)
    {
        // input file parameter
        if(std::string(argv[i]) == "-i" || std::string(argv[i]) == "-i")
        {
            if( i >= argc-1 || !m_TargetFile.empty() ) invalidParameter();
            m_TargetFile = std::string( argv[i+1] );
            i++;
        }
        // target time for metadata
        else if( std::string(argv[i]).find("-t=") != std::string::npos )
        {
            if( m_TargetTime.hh != -1 || m_TargetTimeEnd.hh != -1) invalidParameter();
            if(m_UseStartAndStopTime) invalidParameter();

            std::string ttimestr = std::string(argv[i]);
            ttimestr.erase(0, 3);
            if(!timeStringToVidTime(ttimestr, &m_TargetTime)) invalidParameter();
        }
        // show help menu
        else if( std::string(argv[i]) == "-h" || std::string(argv[i]) == "-help" || std::string(argv[i]) == "-?")
        {
            showHelp();
            exit(1);
        }
        // option to display tags in raw form
        else if( std::string(argv[i]) == "-rt")
        {
            if(m_ShowKLVRaw) invalidParameter();
            m_ShowTagsRaw = true;
        }
        else if( std::string(argv[i]) == "-raw")
        {
            if(m_ShowTagsRaw) invalidParameter();
            m_ShowKLVRaw = true;
        }
        // use a start and stop time
        else if( std::string(argv[i]).find("-b=") != std::string::npos)
        {
            if(m_TargetTime.hh != -1 ) invalidParameter();
            if(m_UseStartAndStopTime) invalidParameter();

            m_UseStartAndStopTime = true;

            std::string ttimestr = std::string(argv[i]);
            ttimestr.erase(0, 3);
            if(!timeStringToVidTime(ttimestr, &m_TargetTime)) invalidParameter();
        }
        else if( std::string(argv[i]).find("-e=") != std::string::npos)
        {
            if(m_TargetTimeEnd.hh != -1 ) invalidParameter();
            if(!m_UseStartAndStopTime) invalidParameter();

            std::string ttimestr = std::string(argv[i]);
            ttimestr.erase(0, 3);
            if(!timeStringToVidTime(ttimestr, &m_TargetTimeEnd)) invalidParameter();
        }
        else if( std::string(argv[i]) == "-g") m_DoGenerateGoogleMapsLink = true;
        else if( std::string(argv[i]) == "-fast")
        {
            if(m_PacketRateFilter != P_ALL) invalidParameter();
            m_PacketRateFilter = P_FAST;
        }
        else if( std::string(argv[i]) == "-slow")
        {
            if(m_PacketRateFilter != P_ALL) invalidParameter();
            m_PacketRateFilter = P_SLOW;
        }
        else if( std::string(argv[i]) == "-noss") m_DoScreenshot = false;
        else if( std::string(argv[i]) == "-csv") m_DoGenerateCSV = true;
        else if( std::string(argv[i]) == "-quiet") m_QuietMode = true;
        else if( std::string(argv[i]) == "-kml") m_DoGenerateKML = true;
        else if( std::string(argv[i]) == "-v" || std::string(argv[i]) == "-ver")
        {
            printAbout();
            done();
        }
        else invalidParameter();

    }

    // check that required parameters have been set
    if(m_TargetFile.empty()) invalidParameter();
    //if(m_TargetTime.hh == -1) invalidParameter();
    if(m_UseStartAndStopTime)
    {
        if(m_TargetTimeEnd.hh == -1) invalidParameter();
    }


    // check that FFMPEG is present in the working directory
    if(!isFFMPEGPresent())
    {
        std::cout << "ffmpeg.exe was not found in working directory.  Make sure ffmpeg.exe is in same working directory.\n";
        exit(4);
    }

    // init tag list
    m_Taglist[1] = new TagItem_Data(1, "Checksum");
    m_Taglist[2] = new TagItem_Microseconds(2, "UTC Timestamp(us)");
    m_Taglist[3] = new TagItem_String(3, "Mission ID");
    m_Taglist[5] = new TagItem_Angle(5, "Platform Heading",0.f, 360.f);
    m_Taglist[6] = new TagItem_Angle(6, "Platform Pitch", -20.f, 20.f);
    m_Taglist[7] = new TagItem_Angle(7, "Platform Roll", -50.f, 50.f);
    m_Taglist[10] = new TagItem_String(10, "Platform Designation");
    m_Taglist[11] = new TagItem_String(11, "Sensor");
    m_Taglist[12] = new TagItem_String(12, "Coordinate System");
    m_Taglist[13] = new TagItem_Latitude(13, "Sensor Lat");
    m_Taglist[14] = new TagItem_Longitude(14, "Sensor Lon");
    m_Taglist[15] = new TagItem_Int(15, "Sensor Alt(msl)", "m", -900, 19000);
    m_Taglist[16] = new TagItem_Angle(16, "Sensor H_FOV", 0.f, 180.f);
    m_Taglist[17] = new TagItem_Angle(17, "Sensor V_FOV", 0.f, 180.f);
    m_Taglist[18] = new TagItem_Angle(18, "Sensor Az", 0.f, 360.f);
    m_Taglist[19] = new TagItem_Angle(19, "Sensor El", -180.f, 180.f);
    m_Taglist[20] = new TagItem_Angle(20, "Sensor Roll", 0.f, 360.f);
    m_Taglist[21] = new TagItem_Int(21, "Slant Range", "m", 0, 5000000);
    m_Taglist[22] = new TagItem_Int(22, "Target Width", "m", 0, 10000);
    m_Taglist[23] = new TagItem_Latitude(23, "Target Lat");
    m_Taglist[24] = new TagItem_Longitude(24, "Target Lon");
    m_Taglist[25] = new TagItem_Int(25, "Target Alt(msl)", "m", -900, 19000);
    m_Taglist[48] = new TagItem_Security(48, "Security");
    m_Taglist[65] = new TagItem_Int(65, "UAS Local Set Version");


    // attempt to create CSV file
    if(m_DoGenerateCSV)
    {
        std::stringstream csvfilenamess;

        csvfilenamess << m_TargetFile;

        // if a target time is provided,
        if(m_TargetTime.hh != -1) csvfilenamess << "_" << m_TargetTime.hh << "_" << m_TargetTime.mm << "_" << m_TargetTime.ss;

        csvfilenamess << ".csv";

        m_TargetCSVFilename = csvfilenamess.str();

        m_CSVFile.open(m_TargetCSVFilename.c_str());

        if(!m_CSVFile.is_open())
        {
            std::cout << "Error opening CSV file for writing:" << m_TargetCSVFilename << std::endl;
            m_DoGenerateCSV = false;
        }
        else
        {
            // write CSV header
            std::stringstream csvhdrss;
            int indexcounter = 0;

            csvhdrss << "UKEY,Video Position,";

            for(std::map<int, TagItem*>::iterator it = m_Taglist.begin(); it != m_Taglist.end(); it++)
            {
                csvhdrss << it->second->getName() << ",";
                m_TaglistIndex[it->second->getID()] = indexcounter;

                indexcounter++;
            }
            csvhdrss << std::endl;

            m_CSVFile << csvhdrss.str();

            //m_CSVFile.close();
        }
    }


    // rip KLV metadata index from stream to binary file
    binfile = std::string( m_TargetFile + ".bin");
    ifile.open( binfile.c_str(), std::ios::binary);

    // if binary file has not been ripped yet
    if(!ifile.is_open())
    {
        if(!m_QuietMode) std::cout << "Ripping binary stream to " << m_TargetFile << ".bin\n";
        myss << "ffmpeg.exe -y -v 0 -i " << m_TargetFile << " -map 0:3 -codec copy -f data " << std::string(m_TargetFile + ".bin");
        system(myss.str().c_str());

        ifile.open( binfile.c_str(), std::ios::binary);

        // try again now that new bin has been generated
        if(!ifile.is_open())
        {
            std::cout << "Error opening file:" << m_TargetFile << std::endl;
            exit(2);
        }
    }

    // begin parsing KLV into a KLV struct
    while(!ifile.eof())
    {

        // first data in klv bin file should be a key, starting with 0x06
        unsigned char c;
        c = ifile.get();

        // get KLV
        if( int(c) == 0x06)
        {
            // current klv and time data
            KLV current;
            UTCus packettime = -1;
            VidTime vtime;
            bool is_fast_rate = true;
            vtime.hh = -1;

            // begin getting key
            current.key.push_back(c);
            // read in KEY
            for(int i = 0; i < 15; i++) current.key.push_back( ifile.get());

            // get length
            current.len = int(ifile.get());

            // get value
            for(int i = 0; i < current.len; i++) current.val.push_back( ifile.get());

            // if this is a standard MISM packet (not timestamp only)
            if(stringToHexString(current.key) == "06 0e 2b 34 02 0b 01 01 0e 01 03 01 01 00 00 00")
            {
                int index = 0;

                // check the data
                while(index < int(current.val.length()) )
                {
                    int tag = int( (unsigned char)(current.val[index]) );
                    int tlen = int( (unsigned char)(current.val[index+1]) );
                    std::string tdat = current.val.substr( index+2, tlen);
                    std::string tstr = stringToHexString( tdat);

                    // if timestamp tag is found, process packet/video time
                    if(tag == 2)
                    {
                        packettime = getPacketTime(&current);
                        // if start time has not been set yet, use this as start time
                        if(!m_UTCusMISMStartTime) m_UTCusMISMStartTime =packettime;

                        // get video time (delta from UTC us start time)
                        vtime = getVidTime(m_UTCusMISMStartTime, packettime);
                    }
                    // if tag is belongs to a slow rate message only, flag this packet
                    else if(tag == 12 || tag == 3 || tag == 10 || tag == 48 )
                        is_fast_rate = false;

                    index += tlen + 2;
                }

                // if using timestamp parameters
                if(m_TargetTime.hh != -1 && vtime.hh != -1)
                {
                    // if using single target timestamp, do check
                    if(!m_UseStartAndStopTime)
                    {
                        // if using a slow rate filter
                        if(m_PacketRateFilter == P_SLOW)
                        {
                            // if current packet is a slow rate packet
                            if(!is_fast_rate)
                            {
                                // if current packet time has reached target time
                                if(vtime >= m_TargetTime)
                                {
                                    // add current packet if there are none
                                    if(cur_slowpackets.empty()) cur_slowpackets.push_back(current);
                                    // if not, process current packet list
                                    else
                                    {
                                        // if current slow packet has same time as packet in cur list, add it
                                        if( getPacketTime(&cur_slowpackets[0]) == packettime)
                                            cur_slowpackets.push_back(current);
                                        // else slow packets in current have all been found, check against previous

                                    }

                                }
                                // if not, add this slow rate packet to previous list
                                else
                                {
                                    // if prev packets are empty, add this packet
                                    if(prev_slowpackets.empty()) prev_slowpackets.push_back(current);
                                    // if not check, check packets
                                    else
                                    {
                                        // check if packets in slowpackets list has same time,
                                        // if so, add current packet to the list
                                        if( getPacketTime(&prev_slowpackets[0]) == getPacketTime(&current))
                                            prev_slowpackets.push_back(current);
                                        // if not, clear previous packets and time and set to current
                                        else
                                        {
                                            prev_slowtime = packettime;
                                            prev_slowpackets.clear();
                                            prev_slowpackets.push_back(current);
                                        }
                                    }
                                }

                            }

                        }
                        // DEBUGSTUFF
                        else if(vtime >= m_TargetTime)
                        {
                            std::stringstream ss;

                            // if packets have been found already
                            if(!packetsAtTargetTime.empty())
                            {
                                // if current packet does not match previously found packets at target
                                // time, then we are done
                                if(getPacketTime(&packetsAtTargetTime[0]) != packettime)
                                {
                                    for(int p = 0; p < int(packetsAtTargetTime.size()); p++)
                                    {
                                        // display KLV packet
                                        printKLV(packetsAtTargetTime[p]);
                                    }

                                    // take screen shot
                                    takeScreenshotAtVideoTime( getVidTime(m_UTCusMISMStartTime,getPacketTime(&packetsAtTargetTime[0]) )  );
                                    done();
                                }

                            }

                            packetsAtTargetTime.push_back(current);


                        }
                    }
                    else if(vtime >= m_TargetTime && vtime <= m_TargetTimeEnd) printKLV(current);
                }


            }
            // else commercial time code
            else
            {
                //unsigned char tstatus = current.val[0];

                std::string tsstr = current.val.substr(1, current.len-1);

                UTCus itime = bytesToU64(tsstr);
                if(!m_UTCusStartTime) m_UTCusStartTime = itime;

                VidTime vtime = getVidTime(m_UTCusStartTime, itime);

                if(m_UseStartAndStopTime)
                {
                    if(vtime >= m_TargetTime && vtime <= m_TargetTimeEnd) printKLV(current);
                }
            }

            // if not using any timestamp parameters, print all packets
            if(m_TargetTime.hh == -1)
            {
                switch(m_PacketRateFilter)
                {
                case P_SLOW:
                    if(!is_fast_rate) printKLV(current);
                    break;
                case P_FAST:
                    if(is_fast_rate) printKLV(current);
                    break;
                case P_ALL:
                default:
                    printKLV(current);
                    break;
                }
            }
        }
    }


    // print found slow packets
    if(m_TargetTime.hh != -1 && !m_UseStartAndStopTime && m_PacketRateFilter == P_SLOW)
    {
        // convert target time to UTC us time
        UTCus cur_utcus = getVidTimeSeconds(m_TargetTime) * 1000000 + m_UTCusMISMStartTime;

        // if previous slow packet list is not empty, compare
        if(!prev_slowpackets.empty())
        {
            // if previous packets were closer to target time, make them the current
            if( getPacketTime(&cur_slowpackets[0]) - cur_utcus > cur_utcus - getPacketTime(&prev_slowpackets[0]))
            {
                cur_slowpackets = prev_slowpackets;
            }
        }

        // print current slow packets in list
        for(int p = 0; p < int(cur_slowpackets.size()); p++)
        {
            printKLV(cur_slowpackets[p]);
        }
        takeScreenshotAtVideoTime( getVidTime(m_UTCusMISMStartTime, getPacketTime(&cur_slowpackets[0]) ));
        exit(1);
    }

    // done
    ifile.close();
    done();

}

void TSKLV::done()
{
    // close CSV file if necessary
    if(m_CSVFile.is_open()) m_CSVFile.close();

    // if generating kml file
    if(m_DoGenerateKML)
    {
        std::stringstream kmlss;

        kmlss << m_TargetFile;

        // if a target time is provided, add time to file name
        if(m_TargetTime.hh != -1)
        {
            kmlss << "_" << m_TargetTime.hh << "_" << m_TargetTime.mm << "_" << m_TargetTime.ss;
        }

        kmlss << ".kml";

        m_KML.writeToKML(kmlss.str());
    }

    exit(1);
}

void TSKLV::showHelp()
{
    std::cout << std::endl;
    printAbout();
    std::cout << std::endl;
    std::cout << "Parses MISM KLV metadata from mpeg .ts video.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  tsklv -i <input_file> [-g -rt | -raw] [-fast | -slow] [options]\n";
    std::cout << "  tsklv -i <input_file> -t=hh:mm:ss.ss [options]\n";
    std::cout << "  tsklv -i <input_file> -b=hh:mm:ss.ss -e=hh:mm:ss.ss [options]\n";
    std::cout << std::endl;
    std::cout << "Arguments:\n";
    std::cout << "  -i <file>   Target video file to rip metadata from.\n";
    std::cout << "  -t          Display metadata and take screenshot at this timestamp.\n";
    std::cout << "  -b          Start parsing metadata at this timestamp.\n";
    std::cout << "  -e          Stop parsing metadata at this timestamp.\n";
    std::cout << std::endl;
    std::cout << "Options:\n";
    std::cout << "  -h,-help    Display this help menu.\n";
    std::cout << "  -v,-ver     Display version.\n";
    std::cout << "  -rt         Display tags as raw data.\n";
    std::cout << "  -raw        Display all KLV as raw data.\n";
    std::cout << "  -g          Generate Google Map links for platform and target geolocations.\n";
    std::cout << "  -slow       Show only slow rate messages.\n";
    std::cout << "  -fast       Show only fast rate messages.\n";
    std::cout << "  -noss       Disable screenshot generation.\n";
    std::cout << "  -csv        Output metadata to CSV file.\n";
    std::cout << "  -kml        Output metadata to KML file.\n";
    std::cout << "  -quiet      Do not print metadata.\n";
    std::cout << std::endl;
    std::cout << "Examples:\n\n";
    std::cout << "  tsklv -i myvid.ts\n";
    std::cout << "    This command dumps all metadata packets.\n";
    std::cout << std::endl;
    std::cout << "  tsklv.exe -i myvid.ts -t=0:3:4.0\n";
    std::cout << "    This command dumps one frame of MISM metadata at 3 minutes and 4 seconds\n";
    std::cout << "    into the video.  A screen shot is also automatically generated at that\n";
    std::cout << "    video position.\n";
    std::cout << std::endl;
    std::cout << "  tsklv.exe -i myvid.ts -b=0:1:2.5 -e=0:1:4.5\n";
    std::cout << "    This command dumps all metadata between (b)egin time and (e)nd time in\n";
    std::cout << "    video.\n";

}

std::string TSKLV::getVersionString()
{

    std::string v;

#ifdef DEBUG
v = std::string("DBG_" + std::string(VER));
#else
v = VER;
#endif // DEBUG
    return v;
}

void TSKLV::invalidParameter()
{
    std::cout << "\nInvalid parameters.\n";
    showHelp();
    exit(1);
}

bool TSKLV::isFFMPEGPresent()
{
    std::ifstream ifile;
    ifile.open("ffmpeg.exe");
    if(!ifile.is_open()) return false;
    ifile.close();
    return true;
}

void TSKLV::takeScreenshotAtVideoTime(VidTime tt)
{
    std::stringstream ss;
    std::stringstream sname;

    if(!m_DoScreenshot) return;

    sname << m_TargetFile << "_" << tt.hh << "_" << tt.mm << "_" << tt.ss << ".jpg";

    ss << "ffmpeg.exe -y -v 0 -ss " << tt.hh << ":" << tt.mm << ":" << tt.ss;
    ss << " -i " << m_TargetFile << " -vframes 1 -q:v 2 " << sname.str();
    system(ss.str().c_str());
}

void TSKLV::printKLV(KLV tklv)
{
    VidTime vtime;
    std::stringstream gmss;
    std::stringstream gmts;
    std::stringstream printss;
    const std::string gmaps = ("https://www.google.com/maps/search/?api=1&query=");
    double plat = 0.f;
    double plon = 0.f;
    double tlat = 0.f;
    double tlon = 0.f;
    int palt = 0;
    int talt = 0;
    std::vector<std::string> csvstrings;

    // if generating csv file, init csvstrings vector
    if(m_DoGenerateCSV) csvstrings.resize( m_Taglist.size());

    // if data is MISM
    if(stringToHexString(tklv.key) == "06 0e 2b 34 02 0b 01 01 0e 01 03 01 01 00 00 00")
    {
        int index = 0;
        vtime = getVidTime(m_UTCusMISMStartTime, getPacketTime(&tklv));
        // determine if

        printss << "MISM METADATA [TAG]:\n";
        printss << "Video Position : " << vtime.hh << ":" << vtime.mm << ":" << vtime.ss << std::endl;


        while(index < int(tklv.val.length()) )
        {
            int tag = int( (unsigned char)(tklv.val[index]) );
            int tlen = int( (unsigned char)(tklv.val[index+1]) );
            std::string tdat = tklv.val.substr( index+2, tlen);

            // pad and print tag id
            printss << "[";
            printss << std::setfill('0') << std::setw(2) << tag;
            printss << "] ";

            std::stringstream ss;

            TagItem *titem = m_Taglist[tag];

            if(titem)
            {
                titem->setDataString(tdat);

                printss << titem->getName() << " = ";

                // if showing raw tag data
                if(m_ShowTagsRaw) printss << stringToHexString(titem->getDataString()) << std::endl;
                else printss << titem->getValueString() << titem->getUnitString() << std::endl;


                // handle some tags for special use
                // latitude tags
                if(tag == 13 || tag == 23)
                {
                    TagItem_Latitude *taglat = dynamic_cast<TagItem_Latitude*>(titem);
                    if(taglat)
                    {
                        if(tag == 13) plat = taglat->getValue();
                        else if(tag == 23) tlat = taglat->getValue();
                    }
                }
                // longitude tags
                else if(tag == 14 || tag == 24)
                {
                    TagItem_Longitude *taglon = dynamic_cast<TagItem_Longitude*>(titem);
                    if(taglon)
                    {
                        if(tag == 14) plon = taglon->getValue();
                        else if(tag == 24) tlon = taglon->getValue();
                    }
                }
                else if(tag == 15 || tag == 25)
                {
                    TagItem_Int *tagalt = dynamic_cast<TagItem_Int*>(titem);

                    if(tagalt && tag == 15) palt = int(tagalt->getValue());
                    else if(tagalt && tag == 25) talt = int(tagalt->getValue());
                }
                else if(tag == 48)
                {
                    TagItem_Security *tagss = dynamic_cast<TagItem_Security*>(titem);

                    if(tagss)
                    {
                        printss << tagss->getSecurityString();
                    }
                }

                if(m_DoGenerateCSV)
                {
                    TagItem_Angle *tagangle = dynamic_cast<TagItem_Angle*>(titem);

                    if(tagangle)
                    {
                        std::stringstream angless;
                        angless.precision(ANGLEPRECISION);

                        angless << tagangle->getValue();
                        csvstrings[ m_TaglistIndex[tag] ] = angless.str();
                    }
                    else csvstrings[ m_TaglistIndex[tag] ] = titem->getValueString();
                }

            }
            else printss << "Unknown tag = " << stringToHexString(tdat) << std::endl;



            index += tlen + 2;
        }



    }
    // else commercial time code
    else if(stringToHexString(tklv.key) == "06 0e 2b 34 02 05 01 01 0e 01 01 03 11 00 00 00")
    {
        unsigned char tstatus = tklv.val[0];

        std::string tsstr = tklv.val.substr(1, tklv.len-1);

        printss << "TIMESTAMP:\n";
        //printss << "    Len   :" << ukeylen << std::endl;
        printss << "    Status      :" << std::hex << "0x" << int(tstatus) << std::dec << std::endl;
        printss << "    Raw Time    :" << stringToHexString(tsstr) << std::endl;

        UTCus itime = bytesToU64(tsstr);
        vtime = getVidTime(m_UTCusStartTime, itime);

        printss << "    Epoch       :" << itime / 1000000 << std::endl;
        printss << "    Pos         :" << vtime.hh << ":" << vtime.mm << ":" << vtime.ss << std::endl;

        if(m_DoGenerateCSV)
        {
            std::stringstream itimess;
            itimess << itime;

            csvstrings[ m_TaglistIndex[2] ] = itimess.str();
        }
    }
    else
    {
        printss << "Unrecognized KEY\n";
        printss << "KEY:" << tklv.key << std::endl;
        printss << "LEN:" << tklv.len << std::endl;
        printss << "VAL:" << tklv.val << std::endl;
    }

    if(m_ShowKLVRaw)
    {
        printss.str(std::string());
        printss << stringToHexString(tklv.key) << std::endl;
        printss << tklv.len << std::endl;
        printss << stringToHexString(tklv.val) << std::endl;
    }

    if(m_DoGenerateGoogleMapsLink)
    {
        printss << "Sensor Position: " << gmaps << plat << "," << plon << std::endl;
        printss << "Target Position: " << gmaps << tlat << "," << tlon << std::endl;
    }

    // if generating CSV file, print tag data to file
    if(m_DoGenerateCSV)
    {
        //m_CSVFile.open(m_TargetCSVFilename.c_str(), std::ios::app);

        if(m_CSVFile.is_open())
        {
            std::stringstream csvss;

            csvss << stringToHexString(tklv.key) << "," << vtime.hh << ":" << vtime.mm << ":" << vtime.ss << ",";

            for(int i = 0; i < int(csvstrings.size()); i++) csvss << csvstrings[i] << ",";

            m_CSVFile << csvss.str() << std::endl;

            //m_CSVFile.close();
        }
    }

    if(m_DoGenerateKML)
    {
        // if using a target time only
        if(m_TargetTime.hh != -1 && m_TargetTimeEnd.hh == -1)
        {
            if(plat != 0 && plon != 0) m_KML.setSourcePosition(plat, plon, palt);
            if(tlat != 0 && tlon != 0) m_KML.setTargetPosition(tlat, tlon, talt);
        }
        else if(plat != 0 && plon != 0) m_KML.addPoint(plat, plon, palt);
    }

    if(!m_QuietMode) std::cout << printss.str();
}
