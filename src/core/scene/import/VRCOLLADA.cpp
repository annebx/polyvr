#include "VRCOLLADA.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/objects/VRAnimation.h"

#include "core/utils/rapidxml/rapidxml.hpp"
#include "core/utils/rapidxml/rapidxml_print.hpp"
#include "core/utils/VRFunction.h"
#include "core/utils/toString.h"
#include "core/objects/object/VRObjectT.h"

#include <fstream>
#include <math.h>

#include <OpenSG/OSGGeoProperties.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGSimpleMaterial.h>

using namespace rapidxml;
using namespace OSG;

typedef xml_node<> xNode;
typedef xml_attribute<> Attrib;

struct SamplerIn {
    string semantic;//common name like INTANGENT
    string source;//id name of a source like #Cube_location_Y-input
};

struct Sampler {
    string ID;
    map<string, string> inputs;
    //vector<SamplerIn> inputs;
};

struct Channel {
    string source;
    string target;
    string type;
    string axis;
};

/*

Source has 2 child nodes: array_element (variable) and technique common
array_element can be: have id and count of elements in the array
        <IDREF_array>
        <Name_array> used by interpolation input
        <bool_array>
        <float_array> by all others
        <int_array>

technique_common
    accessor has source=array_element ID and count/stride=how the data in the array should be parsed
        param (1+) a name (eg X or TIME) these define like TIME or float for array_element

*/

struct Source {
    string ID;
    xNode* array_element;
    xNode* technique_common;
   // string data;//contains the data associated with semantic, see above comment
};

struct Animation {
    string ID;
    Channel channel;
    Sampler sampler;
    map<string, Source> sources;
};

struct AnimationLibrary {
    map<string, Animation> animations;
};

vector<xNode*> getxNodes(xNode* node, string name = "") {
    vector<xNode*> res;
    if (name.size() > 0) for (xNode* n = node->first_node(name.c_str()); n; n = n->next_sibling(name.c_str()) ) res.push_back(n);
    else                 for (xNode* n = node->first_node(            ); n; n = n->next_sibling(            ) ) res.push_back(n);
    return res;
}

AnimationLibrary parseColladaAnimations(string data) {
    xml_document<> doc;
    doc.parse<0>(&data[0]);

    AnimationLibrary library;

    xNode* node = doc.first_node("COLLADA");
    if (node) { //found COLLADA tag
        node = node->first_node("library_animations");
        if (node) { //found animations tag...look for animations
            for (xNode* animxNode : getxNodes(node, "animation")) {
                Attrib* animID = animxNode->first_attribute("id");
                xNode* channels = animxNode->first_node("channel");
                xNode* sampler = animxNode->first_node("sampler");
                if (!channels) continue;
                if (!sampler) continue;

                Attrib* source = channels->first_attribute("source");
                Attrib* target = channels->first_attribute("target");
                Attrib* samplerID = sampler->first_attribute("id");

                string tta = target->value();
                int ttaSlash = 0;
                for (int i=tta.size()-1; i>= 0; i--) if (tta[i] == '/') { ttaSlash = i; break; }

                Animation anim;
                anim.ID = animID->value();
                anim.channel = Channel();
                anim.channel.source = source->value();
                anim.channel.target = tta.substr(0, ttaSlash);
                anim.channel.type = tta.substr(ttaSlash+1, tta.size()-ttaSlash-3);
                anim.channel.axis = tta.substr(tta.size()-1, 1);
                anim.sampler = Sampler();
                anim.sampler.ID = samplerID->value();

                if (anim.channel.source.find(anim.sampler.ID) == string::npos) continue;

                for (xNode* samplerIn : getxNodes(sampler, "input")) {
                    //SamplerIn sampIn;
                    string samplerSemantic = samplerIn->first_attribute("semantic")->value();
                    string samplerSource = samplerIn->first_attribute("source")->value();
                    samplerSource = samplerSource.substr(1, samplerSource.size()-1);
                    anim.sampler.inputs[samplerSemantic] = samplerSource;

                    for (xNode* animSource : getxNodes(animxNode, "source")) {//parse through all <source> nodes
                        if (samplerSource.find(animSource->first_attribute("id")->value()) == string::npos) continue;

                        Source source;
                        source.ID = animSource->first_attribute("id")->value();

                        for (xNode* sourcexNode : getxNodes(animSource)) {//parse through child nodes of source node
                            string sourceName = sourcexNode->name();
                            if (sourceName.find("_array") != string::npos) {
                                source.array_element = sourcexNode;
                            } else if(sourceName == "technique_common") {
                                source.technique_common = sourcexNode;
                            }
                        }
                        anim.sources[source.ID] = source;
                    }
                }
                library.animations[anim.ID] = anim;
            }
        } else cout << "<library_animations> tag not found" << endl;
    } else cout << "<COLLADA> tag not found" << endl;

    return library;
}

void printAll(const AnimationLibrary& library) {
    cout << "Imported COLLADA animations:\n";
    cout << " Animations:\n";
    for (auto a : library.animations) {
        cout << "  Animation " << a.first << endl;

        cout << "      Channel source: " << a.second.channel.source << endl;
        cout << "      Channel target: " << a.second.channel.target << endl;
        cout << "      Channel type: " << a.second.channel.type << " | " << a.second.channel.axis << endl;

        cout << "      Sampler " << a.second.sampler.ID << endl;
        cout << "         Sampler inputs:\n";
        for (auto i : a.second.sampler.inputs) {
            cout << "            Input semantic: " << i.first << endl;
            cout << "            Input source: " << i.second << endl;
        }

        cout << "      Sources:\n";
        for (auto s : a.second.sources){
            cout << "         Source " << s.second.ID << endl;//prints source i
            cout << "            Source technique " << s.second.technique_common << endl;//prints source id
            cout << "            Source array " << s.second.array_element->value() << endl;//prints source id
        }
    }
}

void setPose(OSG::VRTransform* o, int i, float t) {// object, axis, new axis values
    if (i < 0 || i > 2) { return; }
    Vec3f f = o->getFrom();
    f[i] = t;
    o->setFrom(f);
}

void setRot(OSG::VRTransform* o, int i, float t) {
    if (i < 0 || i > 2) { return; }
    Vec3f f = o->getEuler();
    f[i] = t;
    o->setEuler(f);
}

void setScale(OSG::VRTransform* o, int i, float t) {
    if (i < 0 || i > 2) { return; }
    Vec3f f = o->getScale();
    f[i] = t;
    o->setScale(f);
}

void setPose3(VRTransform* o, int i, Vec3f t) {
    o->setFrom(t);
}

VRObject* findTarget(VRObject* o, string Name) {
    if (o->hasAttachment("collada_name")) {
        if (o->getAttachment<string>("collada_name") == Name) return o;
    }

    for (auto c : o->getChildren(false)) {
        if (c == o) continue;
        VRObject* tmp = findTarget(c,Name);
        if (tmp != 0) return tmp;
    }
    return 0;
}

template<class T>
void string2Vector(char* input, vector<T> &output){
        std::istringstream iss(input);
        std::copy(std::istream_iterator<T>(iss),
        std::istream_iterator<T>(),
        std::back_inserter(output));
}

void buildAnimations(AnimationLibrary& lib, VRObject* objects) {
    for (auto a : lib.animations) {
        VRObject* obj = findTarget(objects, a.second.channel.target);
        if (obj == 0) continue;
        if (!obj->hasAttachment("transform")) continue;

        map<string, Source> sources = a.second.sources;
        Sampler sampler = a.second.sampler;
        Source inputSource = sources.find(sampler.inputs.find("INPUT")->second)->second;
        Source outputSource = sources.find(sampler.inputs.find("OUTPUT")->second)->second;
        Source interpolationSource = sources.find(sampler.inputs.find("INTERPOLATION")->second)->second;
        Source intangentSource = sources.find(sampler.inputs.find("IN_TANGENT")->second)->second;
        Source outtangentSource = sources.find(sampler.inputs.find("OUT_TANGENT")->second)->second;

        vector<float> inputValues;
        vector<float> outputValues;
        vector<string> interpolationValues;
        vector<float> intangentValues;
        vector<float> outtangentValues;
        string2Vector(inputSource.array_element->value(), inputValues);
        string2Vector(outputSource.array_element->value(), outputValues);
        string2Vector(interpolationSource.array_element->value(), interpolationValues);
        string2Vector(intangentSource.array_element->value(), intangentValues);
        string2Vector(outtangentSource.array_element->value(), outtangentValues);

/*
 *         xNode* technique_common = inputSource.technique_common;
 *         xNode* accessor = technique_common->first_node("accessor");
 *         string inputArrayType =  accessor->first_node("param")->first_attribute("type")->value();//differentiate type
 *         string outputArrayType =  accessor->first_node("param")->first_attribute("type")->value();//differentiate type
 *         string interpolationArrayType =  accessor->first_node("param")->first_attribute("type")->value();//differentiate type
 *         string intangentArrayType =  accessor->first_node("param")->first_attribute("type")->value();//differentiate type
 *         string outtangentArrayType =  accessor->first_node("param")->first_attribute("type")->value();//differentiate type
 */

        xNode* technique_common = inputSource.technique_common;
        xNode* accessor = technique_common->first_node("accessor");
        int inputCount = toInt(accessor->first_attribute("count")->value());
        int outputCount = toInt(accessor->first_attribute("count")->value());
        int interpolationCount = toInt(accessor->first_attribute("count")->value());
        int intangenCount = toInt(accessor->first_attribute("count")->value());
        int outtangentCount = toInt(accessor->first_attribute("count")->value());

        int inputStride = toInt(accessor->first_attribute("stride")->value());
        int outputStride = toInt(accessor->first_attribute("stride")->value());
        int interpolationStride = toInt(accessor->first_attribute("stride")->value());
        int intangentStride = toInt(accessor->first_attribute("stride")->value());
        int outtangentStride = toInt(accessor->first_attribute("stride")->value());

        int inputArrayCount = toInt(inputSource.array_element->first_attribute("count")->value());

        VRTransform* t = (VRTransform*)obj;
        int axis = -1;
        VRFunction<float>* fkt;

        if(a.second.channel.type.find("rotation") != string::npos) { // check if rotation
            for (int i=0; i<outputValues.size(); i++) outputValues[i] = outputValues[i]*Pi/180.f; // convert degrees to radians

            if(a.second.channel.type.find("X") != string::npos) axis = 0;
            else if(a.second.channel.type.find("Y") != string::npos) axis = 1;
            else if(a.second.channel.type.find("Z") != string::npos) axis = 2;
            fkt = new VRFunction<float>(a.first, boost::bind(setRot, t, axis, _1) );

        } else { // translation or scale
            if(a.second.channel.axis.find("X") != string::npos) axis = 0;
            else if(a.second.channel.axis.find("Y") != string::npos) axis = 1;
            else if(a.second.channel.axis.find("Z") != string::npos) axis = 2;

            if(a.second.channel.type.find("location") != string::npos) {
                fkt = new VRFunction<float>(a.first, boost::bind(setPose, t, axis, _1) );
                //(*fkt)(1.0)
            }
            else if(a.second.channel.type.find("scale") != string::npos){
                fkt = new VRFunction<float>(a.first, boost::bind(setScale, t, axis, _1) );
            }

        }


        //auto fkt = new VRFunction<Vec3f>(a.first, boost::bind(setPose3f, t, i, _1) );
        //Vec3f start = Vec3f(x,y,z);
        //Vec3f end = Vec3f(2*x,y,z);

        for(size_t i = 0; i < inputArrayCount - 1; i++){
            float start = outputValues[i];
            float end = outputValues[i+1];
            float duration = inputValues[i+1] - inputValues[i];
            float offset = inputValues[i];
            bool loop = false;
            VRAnimation* anim = new VRAnimation(duration, offset, fkt, start, end, loop);
            t->addAnimation(anim);
        }
    }
}

VRObject* OSG::loadCollada(string path, VRObject* objects) {
    ifstream file(path);
    string data( (std::istreambuf_iterator<char>(file) ), (std::istreambuf_iterator<char>() ) );
    file.close();

    auto library = parseColladaAnimations(data);
    buildAnimations(library, objects);

    //printAll(library);

    VRObject* res = new VRObject("COLLADA");
    //res->addChild(n);
    return res;
}
