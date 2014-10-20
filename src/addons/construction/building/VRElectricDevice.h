#ifndef VRELECTRICDEVICE_H_INCLUDED
#define VRELECTRICDEVICE_H_INCLUDED

#include "core/setup/devices/VRDevice.h"
#include "core/objects/object/VRObject.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRElectricDevice {
    protected:
        bool active;
        VRFunction<bool>* callback;
        VRObject* EDev;

    public:
        VRElectricDevice(VRObject* obj, VRFunction<bool>* cb);

        bool isActive();

        void turnOn();

        void turnOff();

        void toggle(VRDevice* dev);
};

OSG_END_NAMESPACE;

#endif // VRELECTRICDEVICE_H_INCLUDED
