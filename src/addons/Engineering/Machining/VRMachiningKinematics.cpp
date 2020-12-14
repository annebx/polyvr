#include "VRMachiningKinematics.h"

#include "core/utils/toString.h"

using namespace OSG;

template<> string typeName(const VRMachiningKinematics& t) { return "MachiningKinematics"; }

VRMachiningKinematics::VRMachiningKinematics() {}
VRMachiningKinematics::~VRMachiningKinematics() {}

VRMachiningKinematicsPtr VRMachiningKinematics::ptr() { return static_pointer_cast<VRMachiningKinematics>(shared_from_this()); }

void VRMachiningKinematics::setEndEffector(PosePtr pose) {} // dummy implementation
