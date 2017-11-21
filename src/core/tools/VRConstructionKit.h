#ifndef VRCONSTRUCTIONKIT_H_INCLUDED
#define VRCONSTRUCTIONKIT_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGVector.h>
#include <OpenSG/OSGColor.h>
#include <map>
#include "core/objects/VRObjectFwd.h"
#include "VRToolsFwd.h"
#include "selection/VRSelectionFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class VRSnappingEngine;
class VRSelector;

class VRConstructionKit {
    private:
        VRSnappingEnginePtr snapping;
        VRSelectorPtr selector;
        VRObjectPtr root;

        map<int, VRGeometryPtr> anchors;
        map<VRTransform*, VRTransformPtr> objects;

        int ID();

    public:
        VRConstructionKit();
        ~VRConstructionKit();
        static VRConstructionKitPtr create();

        void clear();

        VRSnappingEnginePtr getSnappingEngine();
        VRSelectorPtr getSelector();
        vector<VRObjectPtr> getObjects();

        int addAnchorType(float size, Color3f color);
        void addObject(VRTransformPtr t);
        VRGeometryPtr addObjectAnchor(VRTransformPtr t, int a, Vec3d pos, float radius);

        void remObject(VRTransformPtr t);

        void breakup(VRTransformPtr obj);
};

OSG_END_NAMESPACE;

#endif // VRCONSTRUCTIONKIT_H_INCLUDED
