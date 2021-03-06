#ifndef VRSTATEMACHINE_H_INCLUDED
#define VRSTATEMACHINE_H_INCLUDED

#include "core/math/VRMathFwd.h"
#include "core/utils/VRFunction.h"
#include "core/utils/VRName.h"
#include "core/math/OSGMathFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

template<class P>
class VRStateMachine : public VRName {
    public:
        typedef VRFunction< P, string > VRTransitionCb;
        typedef shared_ptr<VRTransitionCb> VRTransitionCbPtr;

        typedef VRFunction< string, void > VRStateEnterCb;
        typedef shared_ptr<VRStateEnterCb> VRStateEnterCbPtr;

        typedef VRFunction< string, void > VRStateLeaveCb;
        typedef shared_ptr<VRStateLeaveCb> VRStateLeaveCbPtr;

        class State : public VRName {
            private:
                VRStateEnterCbPtr enterCb;
                VRTransitionCbPtr transitionCb;
                VRStateLeaveCbPtr leaveCb;

            public:
                State(string name, VRTransitionCbPtr t);
                ~State();

                static shared_ptr<State> create(string name, VRTransitionCbPtr t);

                string process(const P& parameters);
                void setStateEnterCB(VRStateEnterCbPtr);
                void setStateLeaveCB(VRStateLeaveCbPtr);

                void enter();
                void leave();
        };

        typedef shared_ptr<State> StatePtr;

    private:
        StatePtr currentState;
        map<string, StatePtr> states;

    public:
        VRStateMachine( string name );
        ~VRStateMachine();

        static shared_ptr<VRStateMachine<P>> create(string name = "StateMachine");

        StatePtr addState(string s, VRTransitionCbPtr t);
        StatePtr getState(string s);
        StatePtr setCurrentState(string s);
        StatePtr getCurrentState();

        StatePtr process(const P& parameters);
};

OSG_END_NAMESPACE;

#endif // VRSTATEMACHINE_H_INCLUDED
