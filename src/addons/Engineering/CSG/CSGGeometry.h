#ifndef CSGGEOMETRY_H_
#define CSGGEOMETRY_H_

#include <string>
#include <OpenSG/OSGGeometry.h>
#include "core/objects/geometry/VRGeometry.h"
#include "addons/Engineering/VREngineeringFwd.h"

class CGALPolyhedron;

OSG_BEGIN_NAMESPACE;

class Octree;

class CSGGeometry : public VRGeometry {
    private:
        CGALPolyhedron* polyhedron = 0;
        string operation = "unite";
        bool editMode = true;
        Matrix4d oldWorldTrans;
        float thresholdL = 1e-4;
        float thresholdA = 1e-8;
        OctreePtr oct;

    protected:
        void applyTransform(CGALPolyhedron* p, Matrix4d m);
        void setCSGGeometry(CGALPolyhedron* p);
        CGALPolyhedron* getCSGGeometry();
        size_t isKnownPoint(OSG::Pnt3f newPoint);
        GeometryTransitPtr toOsgGeometry(CGALPolyhedron* p);
        CGALPolyhedron* toPolyhedron(GeometryMTRecPtr geometry, Matrix4d worldTransform, bool& success);

        void operate(CGALPolyhedron* minuend, CGALPolyhedron* subtrahend);

        void enableEditMode();
        bool disableEditMode();

    public:
        CSGGeometry(string name);
        virtual ~CSGGeometry();

        void init();

        static CSGGeometryPtr create(string name = "csgGeometry");
        CSGGeometryPtr ptr();

        void setThreshold(float tL, float tA);
        Vec2d getThreshold();

        bool setEditMode(bool b);
        bool getEditMode();

        void setOperation(string op);
        string getOperation();
        static vector<string> getOperations();

        void markEdges(vector<Vec2i> edges);
};

OSG_END_NAMESPACE;

#endif /* CSGGEOMETRY_H_ */
