#ifndef VRSKELETON_H_INCLUDED
#define VRSKELETON_H_INCLUDED

#include "core/objects/geometry/VRGeometry.h"
#include "core/math/kinematics/VRConstraint.h"
#include "core/math/pose.h"
#include "VRCharacterFwd.h"

#include <OpenSG/OSGColor.h>

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRSkeleton : public VRGeometry {
    public:
        struct Configuration : public VRName {
            map<int,Vec3d> joints;

            Configuration(string name);
            ~Configuration();

            static shared_ptr<Configuration> create(string name);

            void setPose(int i, Vec3d p);

            Configuration interpolate(Configuration& other);
        };

        struct Bone {
            string name;
            Pose pose;
            float length;
        };

        struct Joint {
            string name;
            Color3f col;
            Vec3d pos;
            Vec3d dir1;
            Vec3d dir2;
            Vec3d up1;
            Vec3d up2;
            int bone1;
            int bone2;
            VRConstraintPtr constraint;
        };

        struct EndEffector {
            string name;
            int boneID = -1;
            int jointID = -1;
            PosePtr target;
        };

        struct ChainData {
            vector<int> chainedBones;
            vector<int> joints;
            vector<float> d;
            Vec3d targetPos;
            float Dtarget;
        };

        struct SystemData {
            int bone;
            vector<int> joints;
            map<int,map<int,float>> d;
        };

        typedef shared_ptr<Configuration> ConfigurationPtr;

    private:

        GraphPtr armature;
        map<int, Bone > bones;
        map<int, Joint> joints;
        int rootBone = -1;
        map<string, EndEffector> endEffectors;
        map<string, ChainData> ChainDataMap;
        map<string, SystemData> SystemDataMap;

        VRUpdateCbPtr simCB; // sim override

        VRGeometryPtr jointsGeo;
        VRGeometryPtr constraintsGeo;
        VRGeometryPtr angleProjGeo;

        void initMaterial(); // skeleton visualisation
        void updateJointPositions();
        vector<int> getBoneJoints(int bone);
        Vec3d& jointPos(int j);

        double computeAngleProjection(double l, double g, double d1, double d2);
        void simStep();
        void resolveKinematics();
        void updateBones();

    public:
        VRSkeleton();
        ~VRSkeleton();

        static VRSkeletonPtr create();

        int addBone(PosePtr pose, float length, string name);
        int addJoint(int bone1, int bone2, VRConstraintPtr constraint, string name, Color3f col = Color3f(1,1,1));
        void setEndEffector(string label, int bone);
        void setRootBone(int bone);

        map<int, Vec3d> getJointsPositions();

        void clear();
        void setupSimpleHumanoid();

        void asGeometry(VRGeoData& data);
        void setupGeometry();
        void updateGeometry();

        void move(string endEffector, PosePtr pose);

        void overrideSim(VRUpdateCbPtr cb);
        map<string, EndEffector> getEndEffectors();
        vector<Joint> getChain(string endEffector);
        vector<int> getBonesChain(string endEffector);
        vector<int> getJointsChain(vector<int>& chainedBones);

        void resolveSystem(string bone);
        void applyFABRIK(string EE);
};

OSG_END_NAMESPACE;


#endif // VRSKELETON_H_INCLUDED
