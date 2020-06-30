#include "VRSprite.h"

#include "VRSpriteResizeTool.h"
#include "core/tools/VRText.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/geometry/OSGGeometry.h"
#ifndef WITHOUT_BULLET
#include "core/objects/geometry/VRPhysics.h"
#endif
#include "core/objects/geometry/VRGeoData.h"
#include "core/tools/VRAnnotationEngine.h"
#include "core/tools/VRGeoPrimitive.h"
#include "core/setup/VRSetup.h"
#ifndef WITHOUT_CEF
#include "addons/CEF/CEF.h"
#endif
#include <OpenSG/OSGNameAttachment.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGeoProperties.h>
#include <sstream>

#define GLSL(shader) #shader

using namespace OSG;

template<> string typeName(const VRSprite& o) { return "Sprite"; }


VRSprite::VRSprite (string name, bool alpha, float w, float h) : VRGeometry(name) {
    width = w;
    height = h;
    type = "Sprite";
}

VRSprite::~VRSprite() {}

VRSpritePtr VRSprite::create(string name, bool alpha, float w, float h) {
    auto s = shared_ptr<VRSprite>(new VRSprite(name, alpha, w, h) );
    s->updateGeo();
    return s;
}

VRSpritePtr VRSprite::ptr() { return static_pointer_cast<VRSprite>( shared_from_this() ); }

void VRSprite::updateGeo() { // TODO: plane primitive would be better, but backwards compatibility??
    VRGeoData data;
    float w2 = width*0.5;
    float h2 = height*0.5;
    data.pushVert(Pnt3d(-w2,h2,0), Vec3d(0,0,1), Vec2d(0,1));
    data.pushVert(Pnt3d(-w2,-h2,0), Vec3d(0,0,1), Vec2d(0,0));
    data.pushVert(Pnt3d(w2,-h2,0), Vec3d(0,0,1), Vec2d(1,0));
    data.pushVert(Pnt3d(w2,h2,0), Vec3d(0,0,1), Vec2d(1,1));
#ifndef __EMSCRIPTEN__
    data.pushQuad();
#else
    data.pushTri(-4,-3,-2);
    data.pushTri(-4,-2,-1);
#endif
    data.apply(ptr());
}

VRTexturePtr VRSprite::setText(string l, float res, Color4f c1, Color4f c2, int ol, Color4f oc, string font) {
    label = l;
    auto m = VRMaterial::create(getName()+"label");
    auto tex = VRText::get()->create(l, font, res, 3, c1, c2, ol, oc);
    m->setTexture(tex);
    setMaterial(m);
    return tex;
}

void VRSprite::webOpen(string path, int res, float ratio) {
#ifndef WITHOUT_CEF
    VRMaterialPtr mat = VRMaterial::get(getName()+"web");
    setMaterial(mat);
    mat->setLit(false);
    web = CEF::create();

    VRDevicePtr mouse = VRSetup::getCurrent()->getDevice("mouse");
    VRDevicePtr keyboard = VRSetup::getCurrent()->getDevice("keyboard");
    VRDevicePtr flystick = VRSetup::getCurrent()->getDevice("flystick");
    VRDevicePtr multitouch = VRSetup::getCurrent()->getDevice("multitouch");

    web->setMaterial(mat);
    web->open(path);
    if (mouse) web->addMouse(mouse, ptr(), 0, 2, 3, 4);
    if (flystick) web->addMouse(flystick, ptr(), 0, -1, -1, -1);
    if (multitouch) web->addMouse(multitouch, ptr(), 0, 2, 3, 4);
    if (keyboard) web->addKeyboard(keyboard);
    web->setResolution(res);
    web->setAspectRatio(ratio);
#endif
}

void VRSprite::setTexture(string path) {
    auto m = VRMaterial::create(path);
    m->setTexture(path);
    setMaterial(m);
}

string VRSprite::getLabel() { return label; }
Vec2d VRSprite::getSize() { return Vec2d(width, height); }

void VRSprite::setSize(float w, float h) {
    width = w;
    height = h;
    updateGeo();
}

void VRSprite::setBillboard(int i) {
    auto mat = getMaterial();

    if (!usingShaders) {
        mat->setVertexShader(spriteShaderVP, "spriteShaderVP");
        mat->setFragmentShader(spriteShaderFP, "spriteShaderFP");

        GeoVec2fPropertyMTRecPtr tcs = GeoVec2fProperty::create();
        GeoVectorPropertyMTRecPtr pos = getMesh()->geo->getPositions();
        for (int i=0; i<4; i++) {
            Pnt3f p = pos->getValue<Pnt3f>(i);
            tcs->addValue(Vec2f(p[0], p[1]));
            pos->setValue(Pnt3f(), i);
        }
        setTexCoords(tcs, 1);
        usingShaders = true;
    }

    mat->setShaderParameter("doBillboard", Real32(i));
}

void VRSprite::setScreensize(bool b) {
    auto mat = getMaterial();

    if (!usingShaders) {
        mat->setVertexShader(spriteShaderVP, "spriteShaderVP");
        mat->setFragmentShader(spriteShaderFP, "spriteShaderFP");

        GeoVec2fPropertyMTRecPtr tcs = GeoVec2fProperty::create();
        GeoVectorPropertyMTRecPtr pos = getMesh()->geo->getPositions();
        for (int i=0; i<4; i++) {
            Pnt3f p = pos->getValue<Pnt3f>(i);
            tcs->addValue(Vec2f(p[0], p[1]));
            pos->setValue(Pnt3f(), i);
        }
        setTexCoords(tcs, 1);
        usingShaders = true;
    }

    mat->setShaderParameter("doScreensize", Real32(b));
}

void VRSprite::showResizeTool(bool b, float size, bool doAnnotations) {
    if (!b && resizeTool) resizeTool->select(false);
    if (b) {
        if (!resizeTool) {
            setPrimitive("Plane "+toString(width)+" "+toString(height)+" 1 1");
            resizeTool = VRGeoPrimitive::create();
            resizeTool->setGeometry(ptr());
            addChild(resizeTool);
        }
        resizeTool->select(true);
        resizeTool->setHandleSize(size);
        resizeTool->getLabels()->setVisible(doAnnotations);
    }
}

void VRSprite::convertToCloth() {
#ifndef WITHOUT_BULLET
    getPhysics()->setDynamic(true);
    getPhysics()->setShape("Cloth");
    getPhysics()->setSoft(true);
    getPhysics()->setPhysicalized(true);
#endif
}



string VRSprite::spriteShaderVP =
GLSL(
varying vec2 texCoord;

attribute vec4 osg_Vertex;
attribute vec4 osg_Normal;
attribute vec2 osg_MultiTexCoord0;
attribute vec2 osg_MultiTexCoord1;

uniform float doBillboard;
uniform float doScreensize;
uniform vec2 OSGViewportSize;

#ifdef __EMSCRIPTEN__
uniform mat4 OSGModelViewProjectionMatrix;
uniform mat4 OSGModelViewMatrix;
uniform mat4 OSGProjectionMatrix;
#endif

void main( void ) {
    if (doBillboard < 0.5) {
		if (doScreensize < 0.5) {
			vec4 v = osg_Vertex;
			v.xy += osg_MultiTexCoord1.xy;
#ifdef __EMSCRIPTEN__
			gl_Position = OSGModelViewProjectionMatrix * v;
#else
			gl_Position = gl_ModelViewProjectionMatrix * v;
#endif
		} else {
			vec4 v = osg_Vertex;
#ifdef __EMSCRIPTEN__
			vec4 k = OSGModelViewProjectionMatrix * v;
#else
			vec4 k = gl_ModelViewProjectionMatrix * v;
#endif
		    k.xyz = k.xyz/k.w;
		    k.w = 1.0;
    		float a = OSGViewportSize.y/OSGViewportSize.x;
    		vec4 d = vec4(osg_MultiTexCoord1.x*a, osg_MultiTexCoord1.y,0,0);
#ifdef __EMSCRIPTEN__
		    gl_Position = k + OSGModelViewProjectionMatrix * d * 0.1;
#else
		    gl_Position = k + gl_ModelViewProjectionMatrix * d * 0.1;
#endif
        }
    } else if (doBillboard < 1.5) {
        float a = OSGViewportSize.y/OSGViewportSize.x;
        vec4 d = vec4(osg_MultiTexCoord1.x*a, osg_MultiTexCoord1.y,0,0);
        vec4 v = osg_Vertex;
#ifdef __EMSCRIPTEN__
		vec4 k = OSGModelViewProjectionMatrix * v;
#else
		vec4 k = gl_ModelViewProjectionMatrix * v;
#endif
		if (doScreensize > 0.5) {
    		d *= 0.1;
		    k.xyz = k.xyz/k.w;
		    k.w = 1.0;
        }
    	gl_Position = k + d;
    } else {
	    float a = OSGViewportSize.y/OSGViewportSize.x;
	    vec4 d = vec4(osg_MultiTexCoord1.x*a, 0,0,0);
        vec4 v = osg_Vertex;
        vec4 k;
		if (doScreensize > 0.5) {
    		d *= 0.1;
#ifdef __EMSCRIPTEN__
			k = OSGModelViewProjectionMatrix * v;
#else
			k = gl_ModelViewProjectionMatrix * v;
#endif
		    k.xyz = k.xyz/k.w;
		    k.w = 1.0;
#ifdef __EMSCRIPTEN__
		    k += OSGModelViewProjectionMatrix * vec4(0, osg_MultiTexCoord1.y, 0, 0) * 0.1;
#else
		    k += gl_ModelViewProjectionMatrix * vec4(0, osg_MultiTexCoord1.y, 0, 0) * 0.1;
#endif
		} else {
			v.y += osg_MultiTexCoord1.y;
#ifdef __EMSCRIPTEN__
			k = OSGModelViewProjectionMatrix * v;
#else
			k = gl_ModelViewProjectionMatrix * v;
#endif
		}
    	gl_Position = k + d;
    }
    texCoord = osg_MultiTexCoord0;
}
);


string VRSprite::spriteShaderFP =
GLSL(
#ifdef __EMSCRIPTEN__
precision mediump float;
#endif
uniform sampler2D texture;

varying vec2 texCoord;

void main( void ) {
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    //gl_FragColor = vec4(texCoord.x,texCoord.y,0.0,1.0);
    gl_FragColor = texture2D(texture, texCoord);
}
);
