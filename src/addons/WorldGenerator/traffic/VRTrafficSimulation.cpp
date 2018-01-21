#include "VRTrafficSimulation.h"
#include "../roads/VRRoad.h"
#include "../roads/VRRoadNetwork.h"
#include "../VRWorldGenerator.h"
#include "../terrain/VRTerrain.h"
#include "core/utils/toString.h"
#include "core/utils/VRFunction.h"
#include "core/math/polygon.h"
#include "core/math/graph.h"
#include "core/math/triangulator.h"
#include "core/math/Octree.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/scene/VRScene.h"
#include "core/scene/VRObjectManager.h"
#include "addons/Semantics/Reasoning/VREntity.h"
#include "addons/Semantics/Reasoning/VRProperty.h"

#include <boost/bind.hpp>

using namespace OSG;


VRTrafficSimulation::Vehicle::Vehicle(Graph::position p) : pos(p) {
    t = VRTransform::create("t");
}

VRTrafficSimulation::Vehicle::~Vehicle() {}


VRTrafficSimulation::VRTrafficSimulation() : VRObject("TrafficSimulation") {
    updateCb = VRUpdateCb::create( "traffic", boost::bind(&VRTrafficSimulation::updateSimulation, this) );
    VRScene::getCurrent()->addUpdateFkt(updateCb);

    auto box = VRGeometry::create("boxCar");
    box->setPrimitive("Box", "2 1.5 4 1 1 1");
    addVehicleModel(box);
}

VRTrafficSimulation::~VRTrafficSimulation() {}

VRTrafficSimulationPtr VRTrafficSimulation::create() { return VRTrafficSimulationPtr( new VRTrafficSimulation() ); }

void VRTrafficSimulation::setRoadNetwork(VRRoadNetworkPtr rds) {
    roadNetwork = rds;
    roads.clear();
    auto g = roadNetwork->getGraph();
    for (int i = 0; i < g->getNEdges(); i++) {
        roads[i] = road();
        auto& e = g->getEdge(i);
        Vec3d p1 = g->getNode(e.from).p.pos();
        Vec3d p2 = g->getNode(e.to).p.pos();
        roads[i].length = (p2-p1).length();
    }

    seedEdges.clear();
    for (auto eV : g->getEdges()) {
        for (auto e : eV) {
            if (g->getPrevEdges(e).size() == 0) seedEdges.push_back( e.ID );
        }
    }

    //updateDensityVisual(true);
}

template<class T>
T randomChoice(vector<T> vec) {
    return vec[ round((float(random())/RAND_MAX) * (vec.size()-1)) ];
}

void VRTrafficSimulation::updateSimulation() {
    auto g = roadNetwork->getGraph();
    Octree space(2);

    auto fillOctree = [&]() {
        for (auto& road : roads) { // fill octree
            for (auto& vehicle : road.second.vehicles) {
                auto pos = vehicle.t->getFrom();
                space.add(pos, &vehicle);
            }
        }
    };

    auto propagateVehicle = [&](Vehicle& vehicle, float d) {
        auto& gp = vehicle.pos;
        gp.pos += d;

        if (gp.pos > 1) {
            gp.pos -= 1;
            auto& edge = g->getEdge(gp.edge);
            auto edges = g->getNextEdges(edge);
            if (edges.size() > 0) gp.edge = randomChoice(edges).ID;
            else gp.edge = randomChoice(seedEdges);
        }

        auto p = roadNetwork->getPosition(vehicle.pos);
        vehicle.lastMove = p->pos() - vehicle.t->getFrom();
        vehicle.t->setPose(p);
    };

    auto inFront = [&](PosePtr p1, PosePtr p2, Vec3d lastMove) -> bool {
        Vec3d D = p2->pos() - p1->pos();
        float L = D.length();
        Vec3d Dn = D/L;

        lastMove.normalize();
        float d = Dn.dot(lastMove);
        Vec3d x = lastMove.cross(Vec3d(0,1,0));
        float r = abs( Dn.dot(x) );

        return d > 0 && L < 5 && r < 0.2;
    };

    auto commingRight = [&](PosePtr p1, PosePtr p2) -> bool {
        return false;

        Vec3d D = p2->pos() - p1->pos();
        float L = D.length();
        Vec3d Dn = D/L;
        float d = Dn.dot(p2->dir());
        float r1 = -Dn.dot(p2->x());
        float r2 = -p2->dir().dot( p1->x() );
        return d > 0.5 && r1 > 0.5 && r2 < -0.5;
    };

    auto propagateVehicles = [&]() {
        for (auto& road : roads) {
            for (auto& vehicle : road.second.vehicles) {
                float d = vehicle.speed/road.second.length;

                // check if road ahead is free
                auto pose = vehicle.t->getPose();
                auto res = space.radiusSearch(pose->pos(), 5);
                bool wait = false;
                for (auto vv : res) {
                    auto v = (Vehicle*)vv;
                    auto p = v->t->getPose();
                    if (inFront(pose, p, vehicle.lastMove)) wait = true;
                    else if (commingRight(pose, p)) wait = true;
                }

                if (!wait) propagateVehicle(vehicle, d);
            }
        }
    };

    /*auto resolveCollisions = [&]() {
        float R = 2;
        for (auto& road : roads) { // do range searches
            for (auto& vehicle : road.second.vehicles) {
                auto pos = vehicle.t->getFrom();
                auto res = space.radiusSearch(pos, R);
                if (res.size() <= 1) vehicle.t->setScale(Vec3d(1,1,1));
                else vehicle.t->setScale(Vec3d(2,2,2));
            }
        }
    };*/

    fillOctree();
    propagateVehicles();
    //resolveCollisions();
    updateDensityVisual();
}

void VRTrafficSimulation::addVehicle(int roadID, int type) {
    //if () cout << "VRTrafficSimulation::updateSimulation " << roads.size() << endl;
    auto& road = roads[roadID];
    auto v = Vehicle( Graph::position(roadID, 0.0) );
    v.mesh = models[0]->duplicate();
    v.t->addChild(v.mesh);
    addChild(v.t);
    road.vehicles.push_back( v );
}

void VRTrafficSimulation::addVehicles(int roadID, float density, int type) {
    auto g = roadNetwork->getGraph();
    auto e = g->getEdge(roadID);
    int n1 = e.from;
    int n2 = e.to;
    float L = (g->getNode(n2).p.pos() - g->getNode(n1).p.pos()).length();
    int N = L*density/5.0; // density of 1 means one car per 5 meter!
    for (int i=0; i<N; i++) addVehicle(roadID, type);
}

void VRTrafficSimulation::setTrafficDensity(float density, int type) {
    for (auto road : roads) addVehicles(road.first, density, type);
}

void VRTrafficSimulation::addVehicleModel(VRObjectPtr mesh) {
    models.push_back( mesh->duplicate() );
}

void VRTrafficSimulation::updateDensityVisual(bool remesh) {
    if (!flowGeo) {
        flowGeo = VRGeometry::create("trafficFlow");
        addChild(flowGeo);

        auto mat = VRMaterial::create("trafficFlow");
        mat->setLit(0);
        mat->setLineWidth(5);
        flowGeo->setMaterial(mat);
    }

    if (roadNetwork && remesh) {
        VRGeoData geo;
        float h = 2;
        auto g = roadNetwork->getGraph();
        for (auto n : g->getNodes()) geo.pushVert( n.p.pos() + Vec3d(0,h,0) );
        for (auto eV : g->getEdges()) for (auto e : eV) geo.pushLine(e.from, e.to);
        geo.apply(flowGeo);
    }

    if (flowGeo->size() > 0) { // TODO
        //for (auto road)
    }
}



