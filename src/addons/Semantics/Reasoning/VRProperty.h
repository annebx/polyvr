#ifndef VRPROPERTY_H_INCLUDED
#define VRPROPERTY_H_INCLUDED

#include "VROntologyUtils.h"
#include "addons/Semantics/VRSemanticsFwd.h"
#include "core/utils/VRName.h"
#include <OpenSG/OSGConfig.h>

using namespace std;
OSG_BEGIN_NAMESPACE;

struct VRProperty : public VROntoID, public VRName {
    string type;
    string value;
    vector<string> parents;

    VRProperty(string name, string type = "");
    static VRPropertyPtr create(string name = "none", string type = "");
    VRPropertyPtr copy();

    void setType(string type);
    void setValue(string value);
    string getType();
    string getValue();

    string toString();
};

OSG_END_NAMESPACE;

#endif
