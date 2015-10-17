#include "OSMMapDB.h"
#include <iostream>
#include <fstream>

OSMMap* OSMMapDB::getMap(string posStr) {
    if (maps.count(posStr)) return maps[posStr];

    string filename = "world/mapdata/map-"+posStr+".osm";

    // check if file exists
    ifstream ifile(filename.c_str());
    if (ifile) {
        ifile.close();
    } else {
        cout << "MAP FILE NOT FOUND: " << filename << "\n" << flush;
        return NULL;
    }

    // load the map
    OSMMap* osmMap = OSMMap::loadMap(filename);
    maps[posStr] = osmMap;
    return osmMap;

    // TODO: Garbage Collection. While ->maps has more than 30 entries, remove least recently used.
}
