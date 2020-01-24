#ifndef VRSyncNode_H_INCLUDED
#define VRSyncNode_H_INCLUDED

#include "VRTransform.h"
#include "core/networking/VRWebSocket.h"
#include "core/networking/VRNetworkingFwd.h"
#include <OpenSG/OSGChangeList.h>

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRLight;

class VRSyncRemote {//: public VRName {
    private:
        map<int, int> mapping; // <remote container ID, local container ID>
        string uri;

        VRWebSocket socket;

    public:
        VRSyncRemote(string uri = "");
        ~VRSyncRemote();
//        static VRSyncRemotePtr create(string name = "None");
//        VRSyncRemotePtr ptr();
};

class VRSyncNode : public VRTransform {
    private:
        VRSocketPtr socket;
        VRFunction<void*>* socketCb;
        VRUpdateCbPtr updateFkt;

        map<int, bool> container; // local containers
        map<string, VRSyncRemote> remotes;

        VRObjectPtr copy(vector<VRObjectPtr> children);

        void update();
        void handleChangeList(void* msg);

    public:
        VRSyncNode(string name = "syncNode");
        ~VRSyncNode();

        static VRSyncNodePtr create(string name = "None");
        VRSyncNodePtr ptr();

        void startInterface(int port);

        void addRemote(string host, int port, string name);

        void printChangeList();
};

OSG_END_NAMESPACE;

#endif // VRSyncNode_H_INCLUDED
