#ifndef VRPHYSICSMANAGER_H_INCLUDED
#define VRPHYSICSMANAGER_H_INCLUDED

#include <LinearMath/btAlignedObjectArray.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGVector.h>
#include <map>
#include <vector>

template<class T> class VRFunction;

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btCollisionShape;

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRGeometry;
class VRTransform;

class VRPhysicsManager {
     private:
        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btSequentialImpulseConstraintSolver* solver;
        btDiscreteDynamicsWorld* dynamicsWorld;

        btAlignedObjectArray<btCollisionShape*> collisionShapes;
        btRigidBody* body;

        map<btRigidBody*, VRTransform*> OSGobjs;

        vector<Vec3f> collisionPoints;
        /** timestamp last frame**/
        int t_last;

    protected:
        VRFunction<int>* updatePhysicsFkt;
        VRPhysicsManager();
        ~VRPhysicsManager();

        void updatePhysics();

    public:
        void physicalize(VRTransform* obj);

        void setGravity(Vec3f g);

        btDiscreteDynamicsWorld* bltWorld();

        void collectCollisionPoints();

        vector<Vec3f>& getCollisionPoints();
};

OSG_END_NAMESPACE;

#endif // VRPHYSICSMANAGER_H_INCLUDED
