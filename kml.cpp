#include "kml.hpp"
#include <fstream>
#include <iostream>

KML::KML()
{
    // init position variables
    m_SourcePosition.lat = 0;
    m_SourcePosition.lon = 0;
    m_SourcePosition.alt = 0;

    m_TargetPosition.lat = 0;
    m_TargetPosition.lon = 0;
    m_TargetPosition.alt = 0;
}

KML::~KML()
{

}

void KML::clear()
{
    // clear kml item list
    m_Path.clear();

}

void KML::addPoint(double lat, double lon, int altitude)
{
    KML_Item newitem;
    newitem.lat = lat;
    newitem.lon = lon;
    newitem.alt = altitude;

    m_Path.push_back(newitem);
}

void KML::setSourcePosition(double lat, double lon, int altitude)
{
    m_SourcePosition.lat = lat;
    m_SourcePosition.lon = lon;
    m_SourcePosition.alt = altitude;
}

void KML::setTargetPosition(double lat, double lon, int altitude)
{
    m_TargetPosition.lat = lat;
    m_TargetPosition.lon = lon;
    m_TargetPosition.alt = altitude;
}

bool KML::writeToKML(std::string filename)
{
    std::ofstream kfile;
    KML_Item pathstart;
    KML_Item pathstop;

    pathstart.lat = -1;
    pathstart.lon = -1;
    pathstart.alt = -1;
    pathstop.lat = -1;
    pathstop.lon = -1;
    pathstop.alt = -1;



    kfile.open(filename.c_str());



    if(!kfile.is_open())
    {
        std::cout << "Unable to open KML file for write:" << filename << std::endl;
        return false;
    }

    // if there are no path points but there are sensor and target points, make a line from them
    if(m_Path.empty())
    {
        if(m_SourcePosition.lat != 0 && m_SourcePosition.lon != 0 && m_TargetPosition.lat != 0 && m_TargetPosition.lon != 0)
        {
            m_Path.push_back(m_SourcePosition);
            m_Path.push_back(m_TargetPosition);
        }
    }
    // if there is a path, determine first and last points of path
    else
    {
        pathstart.lat = m_Path[0].lat;
        pathstart.lon = m_Path[0].lon;
        pathstart.alt = m_Path[0].alt;

        pathstop.lat = m_Path.back().lat;
        pathstop.lon = m_Path.back().lon;
        pathstop.alt = m_Path.back().alt;
    }

    // write KML header
    kfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    kfile << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
    kfile << "  <Document>\n";
    // write KML doc
    kfile << "    <name>Doc Name</name>\n";
    kfile << "    <description>Description</description>\n";
    kfile << "    <Style id = \"yellowLineGreenPoly\">\n";
    kfile << "      <LineStyle>\n";
    kfile << "        <color>7f00ffff</color>\n";
    kfile << "        <width>4</width>\n";
    kfile << "      </LineStyle>\n";
    kfile << "      <PolyStyle>\n";
    kfile << "        <color>7f00ff00</color>\n";
    kfile << "      </PolyStyle>\n";
    kfile << "    </Style>\n";

    if(!m_Path.empty())
    {
    // write KML placemark
    kfile << "    <Placemark>\n";
    kfile << "      <name>Sensor Path</name>\n";
    kfile << "      <description>Path of sensor.</description>\n";
    kfile << "      <styleUrl>#yellowLineGreenPoly</styleUrl>\n";
    kfile << "      <LineString>\n";
    kfile << "        <extrude>1</extrude>\n";
    kfile << "        <tessellate>1</tessellate>\n";
    kfile << "        <altitudeMode>absolute</altitudeMode>\n";
    kfile << "        <coordinates> ";
    kfile << m_Path[0].lon << "," << m_Path[0].lat << "," << m_Path[0].alt << std::endl;

    for(int i = 1; i < int(m_Path.size()); i++)
    {
        kfile << "          " << m_Path[i].lon << "," << m_Path[i].lat << "," << m_Path[i].alt << std::endl;
    }

    kfile << "        </coordinates>\n";
    kfile << "      </LineString>\n";
    kfile << "    </Placemark>\n";
    }

    // create source position point
    if(m_SourcePosition.lat != 0 && m_SourcePosition.lon != 0)
    {
    kfile << "    <Placemark>\n";
    kfile << "      <name>Sensor</name>\n";
    kfile << "      <description>Sensor source position.</description>\n";
    kfile << "      <Point>\n";
    kfile << "        <coordinates>" << m_SourcePosition.lon << "," << m_SourcePosition.lat << "," << m_SourcePosition.alt << "</coordinates>\n";
    kfile << "      </Point>\n";
    kfile << "    </Placemark>\n";
    }

    // create target position point
    if(m_SourcePosition.lat != 0 && m_SourcePosition.lon != 0)
    {
    kfile << "    <Placemark>\n";
    kfile << "      <name>Target</name>\n";
    kfile << "      <description>LOS target position.</description>\n";
    kfile << "      <Point>\n";
    kfile << "        <coordinates>" << m_TargetPosition.lon << "," << m_TargetPosition.lat << "," << m_TargetPosition.alt << "</coordinates>\n";
    kfile << "      </Point>\n";
    kfile << "    </Placemark>\n";
    }

    if(pathstart.lat != -1 && pathstop.lat != -1)
    {
    kfile << "    <Placemark>\n";
    kfile << "      <name>Start</name>\n";
    kfile << "      <description>Source path start.</description>\n";
    kfile << "      <Point>\n";
    kfile << "        <coordinates>" << pathstart.lon << "," << pathstart.lat << "," << pathstart.alt << "</coordinates>\n";
    kfile << "      </Point>\n";
    kfile << "    </Placemark>\n";

    kfile << "    <Placemark>\n";
    kfile << "      <name>Stop</name>\n";
    kfile << "      <description>Source path stop.</description>\n";
    kfile << "      <Point>\n";
    kfile << "        <coordinates>" << pathstop.lon << "," << pathstop.lat << "," << pathstop.alt << "</coordinates>\n";
    kfile << "      </Point>\n";
    kfile << "    </Placemark>\n";
    }

    kfile << "  </Document>\n";
    kfile << "</kml>\n";


    // done
    kfile.close();

    return true;
}
