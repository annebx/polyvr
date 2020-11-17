#ifndef VRPYTERRAIN_H_INCLUDED
#define VRPYTERRAIN_H_INCLUDED

#include "core/scripting/VRPyBase.h"
#include "VRTerrain.h"
#include "VRPlanet.h"
#include "VROrbit.h"

struct VRPyTerrain : VRPyBaseT<OSG::VRTerrain> {
    static PyMethodDef methods[];
};

struct VRPyPlanet : VRPyBaseT<OSG::VRPlanet> {
    static PyMethodDef methods[];
};

struct VRPyOrbit : VRPyBaseT<OSG::VROrbit> {
    static PyMethodDef methods[];
};

#endif // VRPYTERRAIN_H_INCLUDED
