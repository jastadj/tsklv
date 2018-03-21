#ifndef CLASS_TAGITEM
#define CLASS_TAGITEM

#include <string>

#define ANGLEPRECISION 10

class TagItem
{
protected:

    std::string m_Name;
    std::string m_DataString;
    std::string m_UnitString;
    int m_TagID;

    uint64_t getuint64(std::string ts);
    std::string getHexString(std::string ts);

public:
    TagItem(int nid, std::string nname);
    std::string getName() { return m_Name;}
    int getID() { return m_TagID;}

    // string of data chars
    virtual void setDataString(std::string dstr) { m_DataString = dstr;}
    std::string getDataString() { return m_DataString;}

    // unit string if necessary
    void setUnitString(std::string ustr) {m_UnitString = ustr;}
    std::string getUnitString() { return m_UnitString;}

    // actual value converted to a string for printing
    virtual std::string getValueString()=0;
};

class TagItem_Data:public TagItem
{
protected:
    std::string m_Value;

public:
    TagItem_Data(int nid, std::string nname);

    void setDataString(std::string dstr);

    std::string getValueString() { return m_Value;}
};

class TagItem_String: public TagItem
{
protected:

public:
    TagItem_String(int nid, std::string nname);

    std::string getValueString() { return m_DataString;}

};

class TagItem_Microseconds:public TagItem
{
protected:
    uint64_t m_Value;

public:
    TagItem_Microseconds(int nid, std::string nname);

    void setDataString(std::string dstr);

    std::string getValueString();
    uint64_t getValue() { return m_Value;}
};

class TagItem_Angle:public TagItem
{
protected:
    double m_Value;
    double m_Min;
    double m_Max;

    void recalculateValue();

public:
    TagItem_Angle(int nid, std::string nname, double dmin = 0, double dmax = 0);

    void setDataString(std::string dstr);

    virtual std::string getValueString();
    double getValue() { return m_Value;}
};

class TagItem_Latitude:public TagItem_Angle
{
public:
    TagItem_Latitude(int nid, std::string nname);
    std::string getValueString();
};

class TagItem_Longitude:public TagItem_Angle
{
public:
    TagItem_Longitude(int nid, std::string nname);
    std::string getValueString();
};

class TagItem_Int:public TagItem
{
protected:
    int64_t m_Value;
    int64_t m_Min;
    int64_t m_Max;

    void recalculateValue();

public:
    TagItem_Int(int nid, std::string nname, std::string nunit = "", int dmin = 0, int dmax = 0);

    void setDataString(std::string dstr);

    std::string getValueString();
    int64_t getValue() { return m_Value;}
};

class TagItem_Security:public TagItem
{
protected:

public:
    TagItem_Security(int nid, std::string nname);

    std::string getValueString() { return getHexString(m_DataString);}
    std::string getSecurityString();

};

#endif // CLASS_TAGITEM
