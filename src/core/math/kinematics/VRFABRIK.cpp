#include "VRFABRIK.h"
#include "core/utils/toString.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/objects/material/VRMaterial.h"

using namespace OSG;

template<> string typeName(const FABRIK& k) { return "FABRIK"; }

FABRIK::FABRIK() {}
FABRIK::~FABRIK() {}

FABRIKPtr FABRIK::create() { return FABRIKPtr( new FABRIK() ); }

void FABRIK::addJoint(int ID, PosePtr p, vector<int> in, vector<int> out) {
    Joint j;
    j.ID = ID;
    j.p = p;
    j.in = in;
    j.out = out;
    joints[ID] = j;
}

PosePtr FABRIK::getJointPose(int ID) { return joints[ID].p; }

void FABRIK::addChain(string name, vector<int> joints) {
    Chain c;
    c.name = name;
    c.joints = joints;
    for (int i=0; i<joints.size()-1; i++) {
        auto p1 = this->joints[joints[i  ]].p->pos();
        auto p2 = this->joints[joints[i+1]].p->pos();
        c.distances.push_back((p2-p1).length());
    }
    chains[name] = c;
}

vector<int> FABRIK::getChainJoints(string name) { return chains[name].joints; }

Vec3d FABRIK::movePointTowards(Chain& chain, int i, Vec3d target, float t) {
    auto interp = [](Vec3d& a, Vec3d& b, float t) {
        return a*t + b*(1-t);
    };

    auto& J = joints[chain.joints[i]];
    Vec3d pOld = J.p->pos();
    J.p->setPos( interp(pOld, target, t) );
    //if (verbose) cout << " movePointTowards: " << J.name << " (" << pOld << ") -> " << 1-t << " / " << target << " -> " << J.pos << endl;
    //if (J.name == "elbowLeft")
    //cout << " movePointTowards: " << J.name << " (" << pOld << ") -> " << 1-t << " / " << target << " -> " << J.pos << endl;
    return pOld;
};

Vec3d FABRIK::moveToDistance(Chain& chain, int i1, int i2, int dID1, int dID2) {
    auto& J1 = joints[chain.joints[i1]];
    auto& J2 = joints[chain.joints[i2]];

    /*if (bones[J1.bone2].name == "lArmLeft" || bones[J2.bone1].name == "lArmLeft") {
        cout << "   moveToDistance " << J1.name << " around " << J2.name << endl;
        Vec3d seg = J2.pos - J1.pos;
        seg.normalize();
        cout << "       check seg1: " << seg << ",  dir1: " << J1.dir1 << ", dot: " << J1.dir1.dot(seg) << endl;
        cout << "       check seg1: " << seg << ",  dir2: " << J1.dir2 << ", dot: " << J1.dir2.dot(seg) << endl;
    }*/

    Vec3d pOld = J1.p->pos();
    //if (verbose) cout << "moveToDistance between: " << J1.name << " -> " << J2.name << endl;

    // joint angles
    /*auto& O1 = chain.orientations[chain.joints[i1]];
    float aD = O1.dir2.enclosedAngle(O1.dir1);
    float aU = O1.up2.enclosedAngle(O1.up1);

    // joint angle direction
    Vec3d cX = O1.dir1.cross(O1.up1);
    Vec3d cD = O1.dir2.cross(O1.dir1);
    Vec3d cU = O1.up2.cross(O1.up1);
    if (cD.dot(cX) < 0) aD = 2*Pi - aD;
    if (cU.dot(cX) < 0) aU = 2*Pi - aU;


    if (J1.name == "elbowLeft") { // test first constraint
        cout << "moveToDistance between: " << J1.name << " -> " << J2.name << endl;
        cout << " aD " << aD << ", aU " << aU << endl;
        // elbow: constrain aD between 0 and 90

        double cA1 = 0.1; // some small angle
        double cA2 = Pi*0.95; // nearly stretched arm

        if (aD < cA1 || aD > cA2) {
            double g = cA1;
            if (aD > cA2) g = cA2;
            double d = computeAngleProjection(aD*0.5, g, distances[dID1], distances[dID2]);
            //Vec3d D = J1.up1 + J1.up2;
            Vec3d D = O1.dir1 + O1.dir2;
            D.normalize();
            J1.pos += D*d;
        }
    }*/


    /*if (J1.name == "waist") { // test first spring, not usefull like this
        // elbow: constrain aD between 0 and 90

        double cA1 = -0.1; // some small angle
        double cA2 =  0.1; // nearly stretched arm

        if (aU < cA1 || aU > cA2) {
            double g = (cA1+cA2)*0.5;
            double d = computeAngleProjection(aU*0.5, g, distances[dID1], distances[dID2]);
            Vec3d U = O1.up1 + O1.up2;
            U.normalize();
            J1.pos += U*d;
        }
    }*/

    // move point to fix distance
    float li = chain.distances[dID1] / (J2.p->pos() - J1.p->pos()).length();
    movePointTowards(chain, i1, J2.p->pos(), li);
    return pOld;
}

void FABRIK::forward(Chain& chain) {
    for (int i = 1; i <= chain.distances.size(); i++) {
        //cout << "\nmove2 " << joints[data.joints[i]].name << endl;
        auto pOld = moveToDistance(chain, i,i-1,i-1,i);
        //auto pOld = joints[data.joints[i]].pos;
        /*if (i < Nd) {
            auto R = getRotation(data.joints[i], data.joints[i+1], pOld);
            rotateJoints(data.joints[i], data.joints[i+1], R, data);
        }

        if (i > 0) { // waist 2
            auto R = getRotation(data.joints[i], data.joints[i-1], pOld);
            rotateJoints(data.joints[i-1], data.joints[i], R, data);
        }*/
    }
}

void FABRIK::backward(Chain& chain) {
    for (int i = chain.distances.size()-1; i > 0; i--) {
        //cout << "\nmove1 " << joints[data.joints[i]].name << endl;
        auto pOld = moveToDistance(chain, i,i+1,i,i-1);
        //auto pOld = joints[data.joints[i]].pos;
        /*if (i > 0) { // waist 1
            auto R = getRotation(data.joints[i], data.joints[i-1], pOld);
            rotateJoints(data.joints[i-1], data.joints[i], R, data);
        }

        if (i < Nd) {
            auto R = getRotation(data.joints[i], data.joints[i+1], pOld);
            rotateJoints(data.joints[i], data.joints[i+1], R, data);
        }*/
    }
}

void FABRIK::chainIteration(Chain& chain) {
    //ChainData& data = ChainDataMap[EE];
    if (chain.joints.size() == 0) return;
    int ee = chain.joints.back();
    if (!joints.count(ee)) return;
    auto target = joints[ee].target;
    if (!target) return;

    auto targetPos = target->pos();

    float tol = 0.001; // 1 mm tolerance
    vector<float>& distances = chain.distances;
    int Nd = distances.size();

    bool verbose = false;

    auto sum = [](vector<float> v) {
        float r = 0;
        for (auto f : v) r += f;
        return r;
    };

    // basic FABRIK algorithm
    float Dtarget = (targetPos - joints[chain.joints[0]].p->pos()).length();
    if (Dtarget > sum(distances)) { // position unreachable
        for (int i=1; i<=Nd; i++) {
            joints[chain.joints[i]].p->setPos( targetPos );
            moveToDistance(chain,i,i-1,i-1,i);
        }
    } else { // position reachable
        float difA = (joints[ee].p->pos()-targetPos).length();
        int k=0;
        while (difA > tol) {
            k++; if (k>15) break;
            int iE = chain.joints.size()-1;
            Vec3d pOld = movePointTowards(chain, iE, targetPos, 0);

            backward(chain);
            forward(chain);
            difA = (joints[chain.joints[iE]].p->pos()-targetPos).length();
        }
    }
}

void FABRIK::setTarget(int i, PosePtr p) {
    joints[i].target = p;
}

void FABRIK::iterate() {
    for (auto chain : chains) {
        chainIteration(chain.second);
    }
}

void FABRIK::iterateChain(string chain) {
    if (!chains.count(chain)) return;
    chainIteration(chains[chain]);
}

void FABRIK::visualize(VRGeometryPtr geo) {
    VRGeoData data;

    for (auto j : joints) {
        data.pushVert(j.second.p->pos(), Vec3d(0,0,0), Color3f(1,0,0));
        data.pushPoint();
    }

    for (auto j : joints) {
        if (j.second.target) {
            data.pushVert(j.second.target->pos(), Vec3d(0,0,0), Color3f(0,0,1));
            data.pushPoint();
        }
    }

    for (auto c : chains) {
        int nj = c.second.joints.size();
        for (int i=0; i<nj-1; i++) {
            data.pushLine(c.second.joints[i], c.second.joints[i+1]);
        }
    }

    data.apply(geo);

    auto m = VRMaterial::get("fabrikPnts");
    m->setPointSize(5);
    m->setLit(0);
    geo->setMaterial(m);
}










