#ifndef CLASS_KML
#define CLASS_KML

#include <string>
#include <vector>

class KML
{
private:

    struct KML_Item
    {
        double lat;
        double lon;
        int alt;
    };

    std::vector<KML_Item> m_Path;
    KML_Item m_SourcePosition;
    KML_Item m_TargetPosition;

public:
    KML();
    ~KML();

    void addPoint(double lat, double lon, int altitude);
    void setSourcePosition(double lat, double lon, int altitude);
    void setTargetPosition(double lat, double lon, int altitude);
    bool writeToKML(std::string filename);
    void clear();

};
#endif // CLASS_KML
