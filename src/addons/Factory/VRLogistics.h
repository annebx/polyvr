#ifndef VRLOGISTICS_H_INCLUDED
#define VRLOGISTICS_H_INCLUDED

#include <map>
#include <vector>
#include <stack>

#include <OpenSG/OSGVector.h>

class FID;
class FNode;
class FNetwork;
class FProduct;
class FStack;
class FTransporter;
class FLogistics;

namespace OSG{ class VRTransform; class VRStroke; class VRSprite; }

class FID {
    private:
        int ID;

    protected:
        FID();

    public:
        int getID();
};

class FObject : public FID {
    public:
        enum Type { NONE, PRODUCT, CONTAINER };

    private:
        Type type;
        OSG::VRTransform* transform = 0;
        OSG::VRSprite* metaData = 0;

    protected:
        FObject();

    public:
        virtual ~FObject();

        void setType(Type);
        Type getType();

        void setTransformation(OSG::VRTransform* t);
        OSG::VRTransform* getTransformation();
        bool move(FNode* n, float dx);

        void setMetaData(std::string s);

        friend class FLogistics;
};

class FNode : public FID {
    public:
        enum State { FREE, CONTAINER, RESERVED, PRODUCT };

    private:
        FObject* object;
        FTransporter* transporter;
        State state;
        OSG::VRTransform* transform;

        std::map<int, FNode*> out;
        std::map<int, FNode*> in;
        std::map<int, FNode*>::iterator itr;

    protected:
        FNode();

    public:
        ~FNode();
        void set(FObject* o);
        FObject* get();

        std::map<int, FNode*>& getIncoming();
        std::map<int, FNode*>& getOutgoing();
        FNode* previous();
        FNode* next();

        void setState(State);
        State getState();

        OSG::Vec3f getTangent();

        void connect(FNode* n);
        void disconnect(FNode* n);
        void isolate();

        OSG::VRTransform* getTransform();

        friend class FNetwork;
};

class FPath : public FID {
    private:
        std::vector<FNode*> nodes;

    public:
        FPath();
        void set(FNode* n1, FNode* n2);
        void add(FNode* n);
        std::vector<FNode*>& get();
};


class FNetwork : public FID {
    private:
        std::map<int, FNode*> nodes;
        std::map<int, FNode*>::iterator itr;

    protected:
        FNetwork();
        ~FNetwork();

    public:
        FNode* addNode(FNode* parent = 0);
        FNode* addNodeChain(int N, FNode* parent = 0);

        OSG::VRStroke* stroke(OSG::Vec3f c, float k);

        friend class FLogistics;
};

class FProduct : public FObject {
    private:
        void* data;

    protected:
        FProduct();
        ~FProduct();

    public:
        void move();

        friend class FLogistics;
};

class FContainer : public FObject {
    private:
        int capacity;
        std::vector<FProduct*> products;

    protected:
        FContainer();
        ~FContainer();

    public:
        void setCapacity(int i);
        int getCapacity();

        void add(FProduct* p);
        FProduct* pop();

        bool isFull();
        bool isEmpty();

        friend class FLogistics;
};

class FTransporter : public FID {
    public:
        enum FTType { PRODUCT, CONTAINER_FULL, CONTAINER_EMPTY };

    private:
        std::map<FNode*, FObject*> cargo;
        FPath* path;
        FTType transport_type;
        float speed;

    protected:
        FTransporter();

    public:
        void setRoute(FNode* n1, FNode* n2);
        void setPath(FPath* p);
        void setTransportType(FTType type);
        void setSpeed(float s);
        float getSpeed();

        void load();
        void unload();

        void update(float dt);

        friend class FLogistics;
};

class FLogistics {
    private:
        std::map<int, FNetwork*> networks;
        std::map<int, FObject*> objects;
        std::map<int, FTransporter*> transporter;
        std::map<int, FPath*> paths;
        std::map<int, FNetwork*>::iterator n_itr;
        std::map<int, FObject*>::iterator o_itr;
        std::map<int, FTransporter*>::reverse_iterator t_ritr;

    public:
        FLogistics();
        ~FLogistics();

        FProduct* addProduct();
        FNetwork* addNetwork();
        FTransporter* addTransporter(FTransporter::FTType type);
        FPath* addPath();
        FContainer* addContainer(OSG::VRTransform* t);
        void fillContainer(FContainer* c, int N, OSG::VRTransform* t);

        void update();
        void run();
};

#endif // VRLOGISTICS_H_INCLUDED
