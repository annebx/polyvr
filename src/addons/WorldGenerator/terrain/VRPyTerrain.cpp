#include "VRPyTerrain.h"
#include "core/scripting/VRPyBaseT.h"
#include "core/scripting/VRPyBaseFactory.h"

using namespace OSG;

simpleVRPyType(Terrain, New_VRObjects_ptr);
simpleVRPyType(Planet, New_VRObjects_ptr);

PyMethodDef VRPyTerrain::methods[] = {
    {"setParameters", PyWrapOpt(Terrain, setParameters, "Set the terrain parameters, size, resolution, height scale, water level", "1|0|0.0001|0.7 0.9 1|1", void, Vec2d, double, double, float, float, Color3f, bool ) },
    {"setWaterLevel", PyWrap(Terrain, setWaterLevel, "Set the water level", void, float ) },
    {"setLit", PyWrap(Terrain, setLit, "Set lit or not", void, bool ) },
    {"setAtmosphericEffect", PyWrap(Terrain, setAtmosphericEffect, "Set the atmospheric density and color", void, float, Color3f ) },
    {"loadMap", PyWrapOpt(Terrain, loadMap, "Load height map", "3", void, string, int ) },
    {"setMap", PyWrapOpt(Terrain, setMap, "Set height map", "3", void, VRTexturePtr, int ) },
    {"getMap", PyWrap(Terrain, getMap, "Get height map", VRTexturePtr ) },
    {"physicalize", PyWrap(Terrain, physicalize, "Physicalize terrain", void, bool ) },
    {"projectOSM", PyWrap(Terrain, projectOSM, "Load an OSM file and project surface types onto terrain, OSM path, N, E", void ) },
    {"paintHeights", PyWrap(Terrain, paintHeights, "Simple function to paint by heights using a texture", void, string, string ) },
    {"getSize", PyWrap(Terrain, getSize, "Get terrain size", Vec2d ) },
    {"getHeight", PyWrapOpt(Terrain, getHeight, "Get height at point (local space)", "1", double, Vec2d, bool ) },
    {"getNormal", PyWrap(Terrain, getNormal, "Get normal at point (local space)", Vec3d, Vec3d ) },
    {"getTexCoord", PyWrap(Terrain, getTexCoord, "Get tex coord at point (local space)", Vec2d, Vec2d ) },
    {"probeHeight", PyWrap(Terrain, probeHeight, "Probe height at point, for debugging", vector<Vec3d>, Vec2d ) },
    {"elevatePoint", PyWrapOpt(Terrain, elevatePoint, "Elevate a point", "0|1", Vec3d, Vec3d, float, bool ) },
    {"elevatePose", PyWrapOpt(Terrain, elevatePose, "Elevate a pose", "0", void, PosePtr, float ) },
    {"elevateObject", PyWrapOpt(Terrain, elevateObject, "Elevate an Object onto the terrain", "0", void, VRTransformPtr, float ) },
    {"flatten", PyWrap(Terrain, flatten, "Flatten the area inside a perimeter", void, vector<Vec2d>, float ) },
    {"setHeightScale", PyWrap(Terrain, setHeightScale, "Set height scale", void, float ) },
    {NULL}  /* Sentinel */
};

PyMethodDef VRPyPlanet::methods[] = {
    {"addSector", PyWrap(Planet, addSector, "Add sector to planet", VRWorldGeneratorPtr, double, double, bool ) },
    {"addOSMMap", PyWrap(Planet, addOSMMap, "Add OSMMap to planet", OSMMapPtr, string ) },
    {"getRadius", PyWrap(Planet, getRadius, "Return planet radius", double ) },
    {"getSector", PyWrap(Planet, getSector, "Return sector at N E", VRWorldGeneratorPtr, double, double ) },
    {"getSectors", PyWrap(Planet, getSectors, "Return all sectors", vector<VRWorldGeneratorPtr> ) },
    {"getSurfaceUV", PyWrap(Planet, getSurfaceUV, "Get uv of surface terrain texture", Vec2d, double, double ) },
    {"getSurfacePose", PyWrapOpt(Planet, getSurfacePose, "Get pose on surface, default in planet coordinates", "0|0", PosePtr, double, double, bool, bool ) },
    {"getMaterial", PyWrap(Planet, getMaterial, "Get planet material", VRMaterialPtr ) },
    {"setParameters", PyWrapOpt(Planet, setParameters, "Set planet parameters: radius, texture, isLit, sector size", "0.1", void, double, string, bool, double ) },
    {"setLayermode", PyWrap(Planet, setLayermode, "Set planet layer mode: full, minimum", void, string ) },
    {"addPin", PyWrapOpt(Planet, addPin, "Add a pin: label, north, east, length", "10000", int, string, double, double, double) },
    {"remPin", PyWrap(Planet, remPin, "Remove a pin: ID", void, int) },
    {"fromLatLongPosition", PyWrapOpt(Planet, fromLatLongPosition, "Get Position on planet based on lat and long", "0", Vec3d, double, double, bool) },
    {"fromPosLatLong", PyWrapOpt(Planet, fromPosLatLong, "Convert space position to lat and long", "0", Vec2d, Pnt3d, bool) },
    {"localize", PyWrap(Planet, localize, "Center the planet origin on a sector", void, double, double) },
    {"divideTIFF", PyWrap(Planet, divideTIFF, "loads sat images as .tif, dividing into .png chunks - string pathIn, string pathOut, double minLat, double maxLat, double minLon, double maxLon, double res \n        pathOut only placeholder right now, new files are saved in project directory", void, string, string, double, double, double, double, double) },
    {NULL}  /* Sentinel */
};
