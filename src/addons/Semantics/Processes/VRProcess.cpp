#include "VRProcess.h"

#include "addons/Semantics/Reasoning/VROntology.h"
#include "addons/Semantics/Reasoning/VRReasoner.h"
#include "addons/Semantics/Reasoning/VRProperty.h"
#include "core/utils/VRStorage_template.h"
#include "core/objects/VRTransform.h"
#include <iostream>

using namespace OSG;

template<> string typeName(const VRProcessPtr& o) { return "Process"; }
template<> string typeName(const VRProcessNodePtr& o) { return "ProcessNode"; }
template<> string typeName(const VRProcessDiagramPtr& o) { return "ProcessDiagram"; }


VRProcessDiagram::VRProcessDiagram() {}
VRProcessDiagram::~VRProcessDiagram() {}
VRProcessDiagramPtr VRProcessDiagram::create() { return VRProcessDiagramPtr( new VRProcessDiagram() ); }

void VRProcessDiagram::update(int i, bool changed) {
    if (processnodes.count(i)) processnodes[i]->update(nodes[i], changed);
}

void VRProcessDiagram::remNode(int i) { Graph::remNode(i); processnodes.erase(i); }
void VRProcessDiagram::clear() { Graph::clear(); processnodes.clear(); }

VRProcessNode::VRProcessNode(string name, PROCESS_WIDGET type, int ID, int sID) : type(type), label(name), ID(ID), subject(sID) {;}
VRProcessNode::~VRProcessNode() {}
VRProcessNodePtr VRProcessNode::create(string name, PROCESS_WIDGET type, int ID, int sID) { return VRProcessNodePtr( new VRProcessNode(name, type, ID, sID) ); }

void VRProcessNode::update(Graph::node& n, bool changed) { // called when graph node changes
    if (widget && !widget->isDragged() && changed) widget->setFrom(n.box.center());

    if (widget && widget->isDragged()) {
        auto m = widget->getDragParent()->getMatrixTo( widget );
        n.box.setCenter( Vec3d(m[3]) );
    }
}

VREntityPtr VRProcessNode::getEntity() { return entity; }
int VRProcessNode::getID() { return ID; }
string VRProcessNode::getLabel() { return label; }

VRProcess::VRProcess(string name) {
    setStorageType("Process");
    setPersistency(0);
    setNameSpace("Process");
    setName(name);

    storeObj("Ontology", ontology);
}

VRProcessPtr VRProcess::create(string name) { return VRProcessPtr( new VRProcess(name) ); }
VRProcessPtr VRProcess::ptr() { return shared_from_this(); }

void VRProcess::open(string path) {
    if (!ontology) ontology = VROntology::create(getBaseName());
    ontology->openOWL(path);
    update();
}

void VRProcess::setOntology(VROntologyPtr o) { ontology = o; update(); }
VROntologyPtr VRProcess::getOntology() { return ontology; }

VRProcessDiagramPtr VRProcess::getInteractionDiagram() {
    if (!interactionDiagram) interactionDiagram = VRProcessDiagram::create();
    return interactionDiagram;
}

VRProcessDiagramPtr VRProcess::getBehaviorDiagram(int subject) { return behaviorDiagrams.count(subject) ? behaviorDiagrams[subject] : 0; }

vector<VRProcessNodePtr> VRProcess::getSubjects() {
    vector<VRProcessNodePtr> res;
    if (!interactionDiagram) return res;
    for (auto node : interactionDiagram->processnodes) {
        if (node.second->type == SUBJECT) res.push_back(node.second);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getMessages() {
    vector<VRProcessNodePtr> res;
    if (!interactionDiagram) return res;
    for (auto node : interactionDiagram->processnodes) {
        if (node.second->type == MESSAGE) res.push_back(node.second);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getSubjectMessages(int subjectID) {
    vector<VRProcessNodePtr> res;
    if (!interactionDiagram) return res;
    auto d = interactionDiagram;
    auto neighbors = d->getNeighbors( subjectID );
    for (auto node : neighbors) {
        auto subject = getNode( node.ID );
        res.push_back(subject);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getOutgoingMessages(int subjectID) {
    auto diag = behaviorDiagrams[subjectID];
    auto nextNodes = diag->getNextNodes(subjectID);
    vector<VRProcessNodePtr> res;
    for (auto node : nextNodes){
        res.push_back(diag->processnodes[node.ID]);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getIncomingMessages(int subjectID) {
    auto diag = behaviorDiagrams[subjectID];
    auto previousNodes = diag->getPreviousNodes(subjectID);
    vector<VRProcessNodePtr> res;
    for (auto node : previousNodes){
        res.push_back(diag->processnodes[node.ID]);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getMessageSubjects(int messageID) {
    vector<VRProcessNodePtr> res;
    if (!interactionDiagram) return res;
    auto d = interactionDiagram;
    auto neighbors = d->getNeighbors( messageID );
    for (auto node : neighbors) {
        auto message = getNode( node.ID );
        res.push_back(message);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getMessageReceiver(int messageID){
    auto diag = interactionDiagram;
    auto nextNodes = diag->getNextNodes(messageID);
    vector<VRProcessNodePtr> res;
    for (auto node : nextNodes){
        res.push_back(diag->processnodes[node.ID]);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getMessageSender(int messageID){
    auto diag = interactionDiagram;
    auto previousNodes = diag->getPreviousNodes(messageID);
    vector<VRProcessNodePtr> res;
    for (auto node : previousNodes){
        res.push_back(diag->processnodes[node.ID]);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getSubjectStates(int subjectID) {
    auto d = behaviorDiagrams[subjectID];
    vector<VRProcessNodePtr> res;
    if (!behaviorDiagrams.count(subjectID)) return res;
    for (auto node : d->processnodes) {
        if (node.second->type == STATE) res.push_back(node.second);
    }
    return res;
}

void VRProcess::printNodes(VRProcessDiagramPtr d){
    for (auto node : d->processnodes) cout << node.second->getLabel() << endl;
}

vector<VRProcessNodePtr> VRProcess::getStateTransitions(int subjectID, int stateID) {
    auto d = behaviorDiagrams[subjectID];
    auto neighbors = d->getNeighbors( stateID );
    vector<VRProcessNodePtr> res;

    for (auto neighbor : neighbors){
        auto transition = d->processnodes[neighbor.ID];
        res.push_back(transition);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getStateOutTransitions(int subjectID, int stateID) {
    auto d = behaviorDiagrams[subjectID];
    auto neighbors = d->getNextNodes( stateID );
    vector<VRProcessNodePtr> res;

    for (auto neighbor : neighbors){
        auto transition = d->processnodes[neighbor.ID];
        res.push_back(transition);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getTransitionStates(int subjectID, int transitionID) {
    auto d = behaviorDiagrams[subjectID];
    auto neighbors = d->getNeighbors( transitionID );
    vector<VRProcessNodePtr> res;
    for (auto node : neighbors) {
        auto state = d->processnodes[node.ID];
        res.push_back(state);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getTransitionSourceState(int subjectID, int transitionID) {
    auto d = behaviorDiagrams[subjectID];
    auto neighbors = d->getNextNodes( transitionID );
    vector<VRProcessNodePtr> res;
    for (auto node : neighbors) {
        auto state = d->processnodes[node.ID];
        res.push_back(state);
    }
    return res;
}

vector<VRProcessNodePtr> VRProcess::getTransitions(int subjectID) {
    vector<VRProcessNodePtr> res;
    auto d = behaviorDiagrams[subjectID];

    for (auto node : d->processnodes){
        if (node.second->type == TRANSITION) res.push_back(node.second);
    }
    return res;
}

map<VRProcessNodePtr, VRProcessNodePtr> VRProcess::getInitialStates(){
    map<VRProcessNodePtr, VRProcessNodePtr> res;
    for (auto s : getSubjects()){
        auto diag = behaviorDiagrams[s->getID()];
        if (!diag) cout << "VRProcess::getInitialStates: No Behavior for subject " << s->getLabel() << " found." << endl;
        for (auto n : diag->processnodes) {
            if (n.second->isInitialState) {
                res[s] = n.second;
            }
        }
    }
    if(!res.size()) cout << "VRProcess:: No initial states could be found." << endl;
    return res;
}


VRProcessNodePtr VRProcess::getStateMessage(VRProcessNodePtr state){
    return stateToMessage[state];
}

void VRProcess::update() {
    if (!ontology) return;

    VRReasonerPtr reasoner = VRReasoner::create();
    reasoner->setVerbose(true,  false); //
    auto query = [&](string q) { return reasoner->process(q, ontology); };

    /** get interaction diagram **/
    auto layers = ontology->getEntities("ModelLayer");
    if (layers.size() == 0) return;
    auto layer = layers[0]; // only use first layer
    interactionDiagram = VRProcessDiagram::create();

    map<string, int> nodes;
    string q_subjects = "q(x):Subject(x);ModelLayer("+layer->getName()+");has("+layer->getName()+",x)";
    cout << "q_subjects: " << q_subjects << endl;
    for ( auto subject : query(q_subjects) ) {
        string label;
        if (auto l = subject->get("hasModelComponentLabel") ) label = l->value;
        int nID = addSubject(label)->ID;
        if (auto ID = subject->get("hasModelComponentID") ) nodes[ID->value] = nID;
        //cout << " VRProcess::update subject: " << label << endl;
    }

    map<string, map<string, vector<VREntityPtr>>> messages;
    string q_messages = "q(x):MessageExchange(x);ModelLayer("+layer->getName()+");has("+layer->getName()+",x)";
    for ( auto message : query(q_messages) ) {
        string sender;
        string receiver;
        if (auto s = message->get("hasSender") ) sender = s->value;
        else cout << "Warning! in VRProcess::update, no sender for message '" << message->getName() << "'" << endl;
        if (auto r = message->get("hasReceiver") ) receiver = r->value;
        else cout << "Warning! in VRProcess::update, no receiver for message '" << message->getName() << "'" << endl;
        if (sender != "" && receiver != "") messages[sender][receiver].push_back(message);
    }

    for ( auto sender : messages ) {
        for (auto receiver : sender.second) {
            string label = "Msg:";
            for (auto message : receiver.second) {
                string q_message = "q(x):MessageSpecification(x);MessageExchange("+message->getName()+");is(x,"+message->getName()+".hasMessageType)";
                auto msgs = query(q_message);
                if (msgs.size())
                    if (auto l = msgs[0]->get("hasModelComponentLabel") ) label += "\n - " + l->value;
            }

            addMessage(label, nodes[sender.first], nodes[receiver.first]);
        }
    }

    //return;
    /** get behavior diagrams **/
    for (auto behavior : query("q(x):SubjectBehavior(x)")) {
        auto behaviorDiagram = VRProcessDiagram::create();
        string q_Subject = "q(x):Subject(x);SubjectBehavior("+behavior->getName()+");has(x,"+behavior->getName()+")";
        auto subjects = query(q_Subject);
        if (subjects.size() == 0) continue;
        auto subject = subjects[0];
        auto ID = subject->get("hasModelComponentID");
        int sID = nodes[ID->value];
        behaviorDiagrams[sID] = behaviorDiagram;

        string q_States = "q(x):State(x);SubjectBehavior("+behavior->getName()+");has("+behavior->getName()+",x)"; //State
        for (auto state : query(q_States)) {
            string label;
            if (auto l = state->get("hasModelComponentLabel") ) label = l->value;
            int nID = addState(label, sID)->ID;
            if (auto ID = state->get("hasModelComponentID") ) nodes[ID->value] = nID;
        }

        map<string, map<string, vector<VREntityPtr>> > transitions;
        map<VREntityPtr, TRANSITION_CONDITION> entityToTransitionCondition;
        TRANSITION_CONDITION conditionType = DEFAULT;
        string q_Transitions = "q(x):Transition(x);SubjectBehavior("+behavior->getName()+");has("+behavior->getName()+",x)";
        for (auto transition : query(q_Transitions)) {
            string source;
            string target;
            VREntityPtr transitionCondition;
            if (auto s = transition->get("hasSourceState") ) source = s->value;
            if (auto r = transition->get("hasTargetState") ) target = r->value;
            transitions[source][target].push_back(transition);

            if (auto c = transition->get("hasTransitionCondition") ) {
                transitionCondition = ontology->getEntity(c->value);
                if (transitionCondition->is_a("ReceiveTransitionCondition") ) {
                    conditionType = RECEIVE_CONDITION;
                    entityToTransitionCondition[transitionCondition] = conditionType;
                } else if (transitionCondition->is_a("SendTransitionCondition") ) {
                    conditionType = SEND_CONDITION;
                    entityToTransitionCondition[transitionCondition] = conditionType;
                }
            }
        }

        for ( auto source : transitions ) {
            for (auto target : source.second) {
                for (auto message : target.second) {
                    //cout << " add message from " << source.first << " to " << target.first << " (" << nodes.count(source.first)  << "," << nodes.count(target.first) << ")" << endl;
                    string msg = message->get("hasModelComponentLabel")->value;
                    auto transitionNode = addTransition(msg, sID, nodes[source.first], nodes[target.first], behaviorDiagram);
                    auto type = entityToTransitionCondition[message];
                    if ( type == RECEIVE_CONDITION) {
                        transitionToCondition[transitionNode] = type;
                    }
                    if ( type == SEND_CONDITION) {
                        transitionToCondition[transitionNode] = type;
                    }
                }
            }
        }
        int count = 0;
        for (auto e : entityToTransitionCondition) {
            cout << "entity " << e.first->getName() << " has conditiontype " << e.second << endl;
            count ++;
        }
        cout << "entityToTransitionCondition size: " << count << endl;
    }
    int count = 0;
    for (auto condition : transitionToCondition) {
        auto type = condition.second;
        string t = "";
        if (type == 1) t = "RECEIVE_CONDITION";
        else if (type == 0) t = "SEND_CONDITION";
        cout << condition.first->getLabel() << " has condition type " << t << endl;
        count++;
    }
    cout << "transitionToCondition size: " << count << endl;
}

void VRProcess::remNode(VRProcessNodePtr n) {
    if (auto diag = interactionDiagram) {
        if (diag->processnodes.count(n->ID)) {
            diag->remNode(n->ID);
            if (behaviorDiagrams.count(n->ID)) behaviorDiagrams.erase(n->ID);
        }
    }

    for (auto diag : behaviorDiagrams) {
        if (diag.second->processnodes.count(n->ID)) diag.second->remNode(n->ID);
    }
}

VRProcessNodePtr VRProcess::addSubject(string name) {
    if (!interactionDiagram) interactionDiagram = VRProcessDiagram::create();
    auto sID = interactionDiagram->addNode();
    auto s = VRProcessNode::create(name, SUBJECT, sID, sID);
    interactionDiagram->processnodes[sID] = s;
    auto behaviorDiagram = VRProcessDiagram::create();
    behaviorDiagrams[sID] = behaviorDiagram;
    return s;
}

VRProcessNodePtr VRProcess::addMessage(string name, int i, int j, VRProcessDiagramPtr diag) {
    if (!diag) diag = interactionDiagram;
    if (!diag) return 0;
    auto mID = diag->addNode();
    auto m = VRProcessNode::create(name, MESSAGE, mID, i);
    diag->processnodes[mID] = m;
    diag->connect(i, mID, Graph::HIERARCHY);
    diag->connect(mID, j, Graph::DEPENDENCY);
    return m;
}

VRProcessNodePtr VRProcess::addState(string name, int sID) {
    if (!behaviorDiagrams.count(sID)) return 0;
    auto diag = behaviorDiagrams[sID];
    //if (!diag) return 0;
    auto nID = diag->addNode();
    auto s = VRProcessNode::create(name, STATE, nID, sID);
    diag->processnodes[nID] = s;
    return s;
}


VRProcessNodePtr VRProcess::addSendState(string name, int sID, VRProcessNodePtr message){
    auto state = addState(name, sID);
    /*if (!behaviorDiagrams.count(sID)) return 0;
    auto diag = behaviorDiagrams[sID];

    auto nID = diag->addNode();
    auto state = VRProcessNode::create(name, SENDSTATE, nID, sID);
    diag->processnodes[nID] = state;*/

    stateToMessage[state] = message;
    return state;
}

VRProcessNodePtr VRProcess::addReceiveState(string name, int sID, VRProcessNodePtr message){
    auto state = addState(name, sID);
    /*if (!behaviorDiagrams.count(sID)) return 0;
    auto diag = behaviorDiagrams[sID];

    auto nID = diag->addNode();
    auto state = VRProcessNode::create(name, RECEIVESTATE, nID, sID);
    diag->processnodes[nID] = state;*/

    stateToMessage[state] = message;
    return state;
}

void VRProcess::setInitialState(VRProcessNodePtr state, int sID){
    auto diag = behaviorDiagrams[sID];
    if (!diag) cout << "VRProcess::setInitialState: No Behavior for subject " << getSubjects()[sID]->getLabel() << " found." << endl;

    if (state->type != STATE){
        cout << "VRProcess::setInitialState: The given Processnode if not of type STATE." << endl;
        return;
    }
    //lookup for state value in processnodes map and set it to initial state
    auto oldInitialState = 0;
    bool newInitialState = false;
    for(auto n : diag->processnodes){
        if(n.second->isInitialState) {
            if (newInitialState) n.second->isInitialState = false;
            else oldInitialState = n.first;
        }
        else if(n.second == state){
            if (oldInitialState){
                diag->processnodes[oldInitialState]->isInitialState = false;
            }
            n.second->isInitialState = true;
            newInitialState = true;
        }
    }
}


VRProcessNodePtr VRProcess::addTransition(string name, int sID, int i, int j, VRProcessDiagramPtr diag ){
    if (!diag) diag = behaviorDiagrams[sID];
    if (!diag) return 0;
    auto tID = diag->addNode();
    auto t = VRProcessNode::create(name, TRANSITION, tID, sID);
    diag->processnodes[tID] = t;
    diag->connect(i, tID, Graph::HIERARCHY);
    diag->connect(tID, j, Graph::DEPENDENCY);
    return t;
}


VRProcessNodePtr VRProcess::getNode(int i, VRProcessDiagramPtr diag) {
    if (!diag) diag = interactionDiagram;
    return diag->processnodes[i];

}

VRProcessNodePtr VRProcess::getTransitionState(VRProcessNodePtr transition) {
    int tID = transition->getID();
    int sID = transition->subject;
    auto diag = behaviorDiagrams[sID];
    auto nextNodes = diag->getNextNodes(tID);
    return diag->processnodes[ nextNodes[0].ID ];
}





