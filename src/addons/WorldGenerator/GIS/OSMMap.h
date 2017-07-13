#ifndef OSMMAP_H
#define OSMMAP_H

#include "GISFwd.h"
#include "core/math/VRMathFwd.h"
#include <string>
#include <map>
#include <vector>
#include <OpenSG/OSGConfig.h>

namespace xmlpp { class Element; }
using namespace std;

OSG_BEGIN_NAMESPACE;

struct OSMBase {
    string id;
    map<string, string> tags;

    OSMBase(string id);
    OSMBase(xmlpp::Element* e);
};

struct OSMNode : OSMBase {
    double lat = 0;
    double lon = 0;

    OSMNode(string id, double lat, double lon);
    OSMNode(xmlpp::Element* e);
};

struct OSMWay : OSMBase {
    vector<string> nodeRefs;

    OSMWay(string id);
    OSMWay(xmlpp::Element* e);
};

class OSMMap {
    private:
        BoundingboxPtr bounds;
        map<string, OSMWayPtr> ways;
        map<string, OSMNodePtr> nodes;

        void readNode(xmlpp::Element* element);
        void readWay(xmlpp::Element* element);
        void readBounds(xmlpp::Element* element);

    public:
        OSMMap(string filepath);
        static OSMMapPtr loadMap(string filepath);

        map<string, OSMWayPtr> getWays();
        map<string, OSMNodePtr> getNodes();
        OSMNodePtr getNode(string id);
        OSMNodePtr getWay(string id);
};

OSG_END_NAMESPACE;

#endif // OSMMAP_H