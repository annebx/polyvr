#include "VRProcessLayout.h"
#include "VRProcess.h"
#include "core/objects/geometry/VRSprite.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/geometry/VRConstraint.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/utils/toString.h"
#include "core/tools/VRText.h"

#include <OpenSG/OSGMatrixUtility.h>

using namespace OSG;

VRProcessLayout::VRProcessLayout(string name) : VRTransform(name) {}
VRProcessLayout::~VRProcessLayout() {}

VRProcessLayoutPtr VRProcessLayout::ptr() { return static_pointer_cast<VRProcessLayout>( shared_from_this() ); }
VRProcessLayoutPtr VRProcessLayout::create(string name) { return VRProcessLayoutPtr(new VRProcessLayout(name) ); }


/* IDEAS

- one material for all widgets
    - one big texture for all labels


*/

int wrapString(string& s, int width) {
    int w = 0;
    vector<int> newBreaks;
    for (int i=0; i<s.size(); i++) {
        char c = s[i];

        if (c == '\n') w = 0;
        else w++;

        if (w >= width) {
            newBreaks.push_back(i);
            w = 0;
        }
    }

    for (auto i : newBreaks) s.insert(i, "\n");

    int N = 1;
    for (auto c : s) if (c == '\n') N++;
    return N;
}

Vec4i pushRectVerts(VRGeoData& geo, float x0, float x1, float y0, float y1, Vec3f n, Vec3f u, float d) {
    Matrix m;
    MatrixLookAt(m, Pnt3f(0,0,0), n, u);

    Pnt3f p1(x0,y0,0); m.mult(p1,p1);
    Pnt3f p2(x1,y0,0); m.mult(p2,p2);
    Pnt3f p3(x1,y1,0); m.mult(p3,p3);
    Pnt3f p4(x0,y1,0); m.mult(p4,p4);

    int A = geo.pushVert(p1 - n*d, n, Vec2f(0,0));
    int B = geo.pushVert(p2 - n*d, n, Vec2f(1,0));
    int C = geo.pushVert(p3 - n*d, n, Vec2f(1,1));
    int D = geo.pushVert(p4 - n*d, n, Vec2f(0,1));
    return Vec4i(A,B,C,D);
}

void pushLabelFace(VRGeoData& geo, int N, float s, float h1, float h2, Vec3f n, Vec3f u) {
    auto q1 = pushRectVerts(geo, -s, s, -h1, h1, n, u, s);
    auto q2 = pushRectVerts(geo, -s + 0.5, -s + 0.5 + N, -h2, h2, n, u, s);

    geo.pushQuad(q1[0],q2[0],q2[1],q1[1]);
    geo.pushQuad(q1[1],q2[1],q2[2],q1[2]);
    geo.pushQuad(q1[2],q2[2],q2[3],q1[3]);
    geo.pushQuad(q1[3],q2[3],q2[0],q1[0]);
    geo.pushQuad(q2[0],q2[3],q2[2],q2[1]); // inner quad for label
}

void pushSubjectBox(VRGeoData& geo, int N, float h) {
    float s = 0.5 * N + 0.5;
    pushLabelFace(geo, N,s,s,h, Vec3f(0,-1,0), Vec3f(0,0,1));
    pushLabelFace(geo, N,s,s,h, Vec3f(0,1,0), Vec3f(0,0,1));
    pushLabelFace(geo, N,s,s,h, Vec3f(1,0,0), Vec3f(0,1,0));
    pushLabelFace(geo, N,s,s,h, Vec3f(-1,0,0), Vec3f(0,1,0));
    pushLabelFace(geo, N,s,s,h, Vec3f(0,0,1), Vec3f(0,1,0));
    pushLabelFace(geo, N,s,s,h, Vec3f(0,0,-1), Vec3f(0,1,0));
}

void pushMsgBox(VRGeoData& geo, int N, float h) {
    float s = 0.5 * N + 0.5;
    Vec4i q1 = pushRectVerts(geo, -s, s, -h, h, Vec3f(0,-1,0), Vec3f(0,0,1), h);
    Vec4i q2 = pushRectVerts(geo, -s, s, -h, h, Vec3f(0,1,0), Vec3f(0,0,1), h);
    Vec4i q3 = pushRectVerts(geo, -s, s, -h, h, Vec3f(0,0,-1), Vec3f(0,1,0), h);
    Vec4i q4 = pushRectVerts(geo, -s, s, -h, h, Vec3f(0,0,1), Vec3f(0,1,0), h);
    Vec4i q5 = pushRectVerts(geo, -h, h, -h, h, Vec3f(1,0,0), Vec3f(0,1,0), s);
    Vec4i q6 = pushRectVerts(geo, -h, h, -h, h, Vec3f(-1,0,0), Vec3f(0,1,0), s);
    geo.pushQuad(q1[0],q1[3],q1[2],q1[1]);
    geo.pushQuad(q2[0],q2[3],q2[2],q2[1]);
    geo.pushQuad(q3[0],q3[3],q3[2],q3[1]);
    geo.pushQuad(q4[0],q4[3],q4[2],q4[1]);
    geo.pushQuad(q5[0],q5[3],q5[2],q5[1]);
    geo.pushQuad(q6[0],q6[3],q6[2],q6[1]);
}

VRGeometryPtr VRProcessLayout::newWidget(VRProcessNode& n, float height) {
    Color4f fg, bg;
    if (n.type == SUBJECT) { fg = Color4f(0,0,0,1); bg = Color4f(1,1,1,1); }
    if (n.type == MESSAGE) { fg = Color4f(0,0,0,1); bg = Color4f(1,1,0,1); }

    int wrapN = 12;
    if (n.type == MESSAGE) wrapN = 22;
    string l = n.label;
    int lineN = wrapString(l, wrapN);

    auto txt = VRText::get()->create(l, "MONO 20", 18*wrapN, 32*lineN, fg, bg);
    auto mat = VRMaterial::create("ProcessElement");
    mat->setTexture(txt, false);
    mat->setTextureParams(GL_LINEAR, GL_LINEAR);
    //mat->enableTransparency(0);
    VRGeoData geo;

    if (n.type == SUBJECT) pushSubjectBox(geo, wrapN, lineN*height*0.5);
    if (n.type == MESSAGE) pushMsgBox(geo, wrapN, lineN*height*0.5);

    auto w = geo.asGeometry("ProcessElement");
    w->setMaterial(mat);
    w->setPickable(1);
    w->getConstraint()->setTConstraint(Vec3f(0,1,0), VRConstraint::PLANE);
    w->getConstraint()->setRConstraint(Vec3f(0,1,0), VRConstraint::POINT);
    w->getConstraint()->setReferential( dynamic_pointer_cast<VRTransform>(ptr()) );
    addChild(w);
    return w;
}

void VRProcessLayout::setProcess(VRProcess::DiagramPtr diag) {
    //process = p;

    clearChildren();

    float height = 2;
    float f=0;
    //auto diag = process->getInteractionDiagram();
    for (int i=0; i<diag->getElements().size(); i++) {
        auto& e = diag->getElement(i);
        auto geo = newWidget(e, height);
        e.widget = geo;

        Vec3f p = Vec3f(f, 0, 0.01*(rand()%100));
        f += e.label.size()+2;
        e.widget->setFrom(p);

        auto& n = diag->getNode(i);
        n.box.update(geo);
        n.box.setCenter(p);
    }
}

VRObjectPtr VRProcessLayout::getElement(int i) { return getChild(i); }