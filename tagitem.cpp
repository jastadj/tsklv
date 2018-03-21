#include "tagitem.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

TagItem::TagItem(int nid, std::string nname)
{
    m_TagID = nid;
    m_Name = nname;
}

uint64_t TagItem::getuint64(std::string ts)
{
    uint64_t result = 0;

    int len = int(ts.length());

    for(int n = len-1; n >= 0; n--)
        result = result | ( (uint64_t)((unsigned char)ts[n]) << ((len-1-n)*8) );

    return result;
}

std::string TagItem::getHexString(std:: string ts)
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

/////////////////////////////////////////////////////////////
//  DATA
TagItem_Data::TagItem_Data(int nid, std::string nname):TagItem(nid, nname)
{

}

void TagItem_Data::setDataString(std::string dstr)
{
    m_DataString = dstr;

    m_Value = getHexString(m_DataString);
}

/////////////////////////////////////////////////////////////
//  DATA STRING
TagItem_String::TagItem_String(int nid, std::string nname):TagItem(nid, nname)
{

}


/////////////////////////////////////////////////////////////
//  DATA MICROSECONDS
TagItem_Microseconds::TagItem_Microseconds(int nid, std::string nname):TagItem(nid, nname)
{
    m_Value = 0;
}

void TagItem_Microseconds::setDataString(std::string dstr)
{
    // set data string
    m_DataString = dstr;

    // calculate new value
    m_Value = getuint64(m_DataString);
}

std::string TagItem_Microseconds::getValueString()
{
    std::stringstream ss;

    ss << m_Value;

    return ss.str();
}

/////////////////////////////////////////////////////////////
//  DATA angle
TagItem_Angle::TagItem_Angle(int nid, std::string nname, double dmin, double dmax):TagItem(nid, nname)
{
    m_Value = 0;
    m_Min = dmin;
    m_Max = dmax;
    m_UnitString = "deg";
}

void TagItem_Angle::recalculateValue()
{
    if(m_Min == m_Max) m_Value = float(getuint64(m_DataString));
    else
    {
        uint64_t mask = 0xff;
        uint64_t val = 0x00;
        double totalrange = m_Max - m_Min;

        for(int i = 0; i < int(m_DataString.length()); i++)
            val = val | (mask << 8*i);

        m_Value = float(getuint64(m_DataString)) * (totalrange / val);

        if(m_Value > m_Max)
            m_Value -= totalrange;

    }
}

void TagItem_Angle::setDataString(std::string dstr)
{
    m_DataString = dstr;

    recalculateValue();
}

std::string TagItem_Angle::getValueString()
{
    std::stringstream ss;

    ss << m_Value;

    return ss.str();
}

/////////////////////////////////////////////////////////////
//  DATA latitude and longitude
TagItem_Latitude::TagItem_Latitude(int nid, std::string nname):TagItem_Angle(nid, nname, -90.f, 90.f)
{
    m_UnitString = "";
}

std::string TagItem_Latitude::getValueString()
{
    double tval = m_Value;
    std::string dir = "N";

    if(tval < 0)
    {
        tval = tval*(-1.f);
        dir = "S";
    }

    std::stringstream ss;
    ss.precision(ANGLEPRECISION);

    ss << tval << " " << dir;

    return ss.str();
}

TagItem_Longitude::TagItem_Longitude(int nid, std::string nname):TagItem_Angle(nid, nname, -180.f, 180.f)
{
    m_UnitString = "";
}

std::string TagItem_Longitude::getValueString()
{
    double tval = m_Value;
    std::string dir = "E";

    if(tval < 0)
    {
        tval = tval * -1.f;
        dir = "W";
    }

    std::stringstream ss;
    ss.precision(ANGLEPRECISION);

    ss << tval << " " << dir;

    return ss.str();
}



/////////////////////////////////////////////////////////////
//  DATA int
TagItem_Int::TagItem_Int(int nid, std::string nname, std::string nunit, int dmin, int dmax):TagItem(nid, nname)
{
    m_Value = 0;
    m_Min = dmin;
    m_Max = dmax;
    m_UnitString = nunit;
}

void TagItem_Int::recalculateValue()
{
    if(m_Min == m_Max) m_Value = int64_t(getuint64(m_DataString));
    else
    {
        uint64_t mask = 0xff;
        uint64_t val = 0x00;
        int64_t totalrange = m_Max - m_Min;

        for(int i = 0; i < int(m_DataString.length()); i++)
            val = val | (mask << 8*i);

        m_Value = int64_t(float(getuint64(m_DataString)) * (float(totalrange) / val) ) + m_Min;

    }
}

void TagItem_Int::setDataString(std::string dstr)
{
    m_DataString = dstr;

    recalculateValue();
}

std::string TagItem_Int::getValueString()
{
    std::stringstream ss;

    ss << m_Value;

    return ss.str();
}

/////////////////////////////////////////////////////////////
//  DATA Security

TagItem_Security::TagItem_Security(int nid, std::string nname): TagItem(nid, nname)
{

}

std::string TagItem_Security::getSecurityString()
{
    // example security data : 01 01 01 02 01 02 03 05 2f 2f 55 53 41 0c 01 02 0d 03 55 53 41 04 04 4e 6f 6e 65 05 04 4e 6f 6e 65 06 04 4e 6f 6e 65 16 02 00 08

    if(m_DataString.empty()) return "";

    int cindex = 0;

    std::string result;


    while(cindex != int(m_DataString.length()) )
    {
        std::stringstream ss;
        unsigned int key;
        unsigned int len;
        std::string val;

        // get key
        key = (unsigned int)(m_DataString[cindex]);

        len = (unsigned int)(m_DataString[cindex+1]);

        for(unsigned int i = 0; i < len; i++)
            val.push_back(m_DataString[cindex+2+i]);

        cindex = cindex + len + 2;

        ss << "48/" << key <<  " = ";

        if(key == 1)
        {
            ss << "Security Classification:" << getHexString(val) << std::endl;
        }
        else if(key == 2)
        {
            ss << "CC & RI Country Coding Method:" << getHexString(val) << std::endl;
        }
        else if(key == 3)
        {
            ss << "Classifying Country:" << val << std::endl;
        }
        else if(key == 4)
        {
            ss << "Security-SCI/SHI Information:" << val << std::endl;
        }
        else if(key == 5)
        {
            ss << "Caveats:" << val << std::endl;
        }
        else if(key == 6)
        {
            ss << "Releasting Instructions:" << val << std::endl;
        }
        else if(key == 12)
        {
            ss << "Object Country Coding Method:" << getHexString(val) << std::endl;
        }
        else if(key == 13)
        {
            ss << "Object Country Codes:" << val << std::endl;
        }
        else if(key == 22)
        {
            ss << "Security Metadata Version:" << getuint64(val) << std::endl;
        }
        else
        {
            ss << "Unknown:" << getHexString(val);
        }

        result = std::string(result + ss.str());
    }

    return result;
}
