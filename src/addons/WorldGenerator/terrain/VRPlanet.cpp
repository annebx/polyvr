#include "VRPlanet.h"
#include "../VRWorldGenerator.h"
#include "VRTerrain.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/VRLod.h"
#include "core/tools/VRAnalyticGeometry.h"
#include "core/tools/VRAnnotationEngine.h"
#include "core/math/pose.h"
#include "core/utils/toString.h"
#include "core/scene/import/GIS/VRGDAL.h"

#define GLSL(shader) #shader

const double pi = 3.14159265359;

using namespace OSG;

template<> string typeName(const VRPlanet& t) { return "Planet"; }

VRPlanet::VRPlanet(string name) : VRTransform(name) {}
VRPlanet::~VRPlanet() {}

VRPlanetPtr VRPlanet::create(string name) {
    auto p = VRPlanetPtr( new VRPlanet(name) );
    p->rebuild();
    return p;
}

VRPlanetPtr VRPlanet::ptr() { return static_pointer_cast<VRPlanet>( shared_from_this() ); }

double VRPlanet::toRad(double deg) { return pi*deg/180; }
double VRPlanet::toDeg(double rad) { return 180*rad/pi; }

void VRPlanet::localize(double north, double east) {
    cout << "VRPlanet::localize" << endl;
    originCoords = Vec2d(north, east);
    auto p = fromLatLongPose(north, east);
    auto localOrigin = p->pos();
    p->invert();
    origin->setPose(p);

    lod->hide(); // TODO: work around due to problems with intersection action!!

    int nSec = 0;
    VRWorldGeneratorPtr sec1;
    for (auto s : sectors) {
        if (nSec==0) sec1 = s.second;
        nSec++;
    }

    for (auto s : sectors) {
        auto sector = s.second;
        addChild(sector);
        sector->setIdentity();

        Vec2d plI = sector->getPlanetCoords() + Vec2d(1,1)*sectorSize*0.5;
        auto pSector = fromLatLongPose(plI[0], plI[1]);
        auto newP = p->multRight(pSector);
        sector->setPose(newP);
        auto newPinv = newP;
        newPinv->invert();

        for (auto terrain:sector->getTerrains()){
            auto grid = terrain->getGrid();
            //auto fac = terrain->getLODFactor();
            auto size = terrain->getSize();
            terrain->setLocalized(true);
            Vec2i gridN = Vec2i(round(size[0]*1.0/grid-0.5), round(size[1]*1.0/grid-0.5));
            //cout << " terrain " << grid << " " << size << endl;
            if (gridN[0] < 1) gridN[0] = 1;
            if (gridN[1] < 1) gridN[1] = 1;
            vector<vector<vector<Vec3d>>> completeMesh;

            int t1 = 0;
            int t2 = 0;
            for (int i =0; i <= gridN[1]; i++) {
                vector<vector<Vec3d>> row;
                t1++;
                t2 = 0;
                for (int j =0; j <= gridN[0]; j++) {
                    t2++;
                    vector<Vec3d> posNorm;
                    //Vertex conversion from global to local patch coordinates
                    auto poseVertexGlobal = fromLatLongPose(sector->getPlanetCoords()[0]+sectorSize*(1.0-double(i)/double(gridN[1])), sector->getPlanetCoords()[1]+j*sectorSize/gridN[0]);
                    auto poseVertexOrigin = p->multRight(poseVertexGlobal);
                    auto poseVertexLocalInPatch = newPinv->multRight(poseVertexOrigin);
                    auto posVertexLocalInPatch = poseVertexLocalInPatch->pos();
                    auto upVertexLocalInPatch = poseVertexLocalInPatch->up();
                    posNorm.push_back(posVertexLocalInPatch);
                    posNorm.push_back(upVertexLocalInPatch);
                    row.push_back(posNorm);
                }
                completeMesh.push_back(row);
            }
            cout << "n,e ___: " << t1 << " -- " << t2 << " " << grid << " " << size << endl;
            terrain->setMeshTer(completeMesh);
            terrain->setupGeo();
        }
        sector->addTerrainsToLOD();
        //cout << " SecNorth: " << fromLatLongNorth(plI[], plI[1]) << endl;
        //cout << "VRPlanet::localize p " << p << " pSector: " << pSector << " localOrigin: " << localOrigin << endl;

        /*
        Vec2d size = sector->getTerrain()->getSize();
        Vec2d p = sector->getPlanetCoords() + Vec2d(1,1)*sectorSize*0.5; // sector mid point
        float X = p[0]-east;
        float Y = north - p[1];
        cout << "VRPlanet::localize p " << p << " P " << Vec2f(north, east) << " XY " << Vec2d(X, Y) << endl;
        //cout << "VRPlanet::localize " << Y << " " << p[0]*sectorSize << " " << north << endl;
        sector->translate(Vec3d(X*size[0]/sectorSize, 0, Y*size[1]/sectorSize));*/
    }

    /*auto s = getSector(north, east);
    if (s) {
        s->setIdentity();
        addChild(s);
    } else cout << "Warning: VRPlanet::localize, no sector found at location " << Vec2d(north, east) << " !\n";*/
}

Vec2d VRPlanet::getSurfaceUV(double north, double east) {
    Vec2d res;
    auto sector = getSector(north, east);
    if (!sector) return res;

    auto sectorCoords = sector->getPlanetCoords();
    auto s = fromLatLongSize(sectorCoords[0], sectorCoords[1], sectorCoords[0]+sectorSize, sectorCoords[1]+sectorSize); //u = east, v = north
    auto u = (east-sectorCoords[1]-sectorSize/2)/sectorSize*s[0];
    auto v = -(north-sectorCoords[0]-sectorSize/2)/sectorSize*s[1];
    return sector->getTerrain()->getTexCoord( Vec2d(u,v) );
}

double VRPlanet::getRadius() { return radius; }

PosePtr VRPlanet::getSurfacePose( double north, double east, bool local, bool sectorLocal){
    auto poseG = fromLatLongPose(north, east);

    auto sector = getSector(north, east);
    if (sector) {
        auto sectorCoords = sector->getPlanetCoords();
        auto s = fromLatLongSize(sectorCoords[0], sectorCoords[1], sectorCoords[0]+sectorSize, sectorCoords[1]+sectorSize); //u = east, v = north
        auto u = (east-sectorCoords[1]-sectorSize/2)/sectorSize*s[0];
        auto v = -(north-sectorCoords[0]-sectorSize/2)/sectorSize*s[1];

        Vec2d uv = Vec2d(u,v);
        auto height = sector->getTerrain()->getHeight(uv);
        auto newPos = poseG->pos() + poseG->up()*height;
        Vec3d f = newPos;
        Vec3d d = poseG->dir();
        Vec3d up = poseG->up();
        poseG = Pose::create(f,d,up); //global pose
    }

    if (local) {
        auto poseOrigin = origin->getPose()->multRight(poseG); //localized with transformed planed origin
        poseG = poseOrigin;
    }

    if (sectorLocal && sector) {
        auto newP = sector->getPose();
        auto newPinv = newP;
        newPinv->invert();
        auto localinSector = newPinv->multRight(poseG); //localized on sector
        poseG = localinSector;
    }

    return poseG;
}

void VRPlanet::divideTIFF(string pathIn, string pathOut, double minLat, double maxLat, double minLon, double maxLon, double res) {
    //cout << "hello " << pathIn << " - " << pathOut << endl;
#ifndef WITHOUT_GDAL
    divideTiffIntoChunks(pathIn, pathOut, minLat, maxLat, minLon, maxLon, res);
#endif
}

Vec2i VRPlanet::toSID(double north, double east) {
    return Vec2i( north/sectorSize+1e-9, east/sectorSize+1e-9 );
}

Vec3d VRPlanet::fromLatLongNormal(double north, double east, bool local) {
    if (local) {
        north -= originCoords[0] - 90;
        east  -= originCoords[1];
    }

    north = -north+90;
	double sT = sin(toRad(north));
	double sP = sin(toRad(east));
	double cT = cos(toRad(north));
	double cP = cos(toRad(east));
	return Vec3d(sT*cP, cT, -sT*sP);
}

Vec3d VRPlanet::fromLatLongPosition(double north, double east, bool local) {
    Vec3d pos;
    if (local) {
        auto s = fromLatLongSize(originCoords[0]-0.5, originCoords[1]-0.5, originCoords[0]+0.5, originCoords[1]+0.5);
        pos = Vec3d( (east-originCoords[1])*s[0], 0, -(north-originCoords[0])*s[1]);

        /*auto p = fromLatLongPose(originCoords[0], originCoords[1], 0); // DEPRECATED, numerically unstable!
        p->invert();
        p->asMatrix().mult(pos, pos);*/
    } else pos = fromLatLongNormal(north, east, 0) * radius;
    return Vec3d( pos );
}

Vec3d VRPlanet::fromLatLongEast(double north, double east, bool local) { return fromLatLongNormal(0, east+90, local); }
Vec3d VRPlanet::fromLatLongNorth(double north, double east, bool local) { return fromLatLongNormal(north+90, east, local); }

PosePtr VRPlanet::fromLatLongPose(double north, double east, bool local) {
    Vec3d f = fromLatLongPosition(north, east, local);
    Vec3d d = fromLatLongNorth(north, east, local);
    Vec3d u = fromLatLongNormal(north, east, local);
    return Pose::create(f,d,u);
}

Vec2d VRPlanet::fromLatLongSize(double north1, double east1, double north2, double east2) {
    auto n = (north1+north2)*0.5;
    auto e = (east1+east2)*0.5;
    Vec3d p1 = fromLatLongPosition(north1, east1);
    Vec3d p2 = fromLatLongPosition(north2, east2);
    //addPin("P1", north1, east1);
    //addPin("P2", north2, east2);
    Vec3d d = p2-p1;
    auto dirEast = fromLatLongEast(n, e);
    auto dirNorth = fromLatLongNorth(n, e);
    double u = d.dot( dirEast );
    double v = d.dot( dirNorth );

    /*d = fromLatLongPosition(north2, east2) - fromLatLongPosition(north2, east1);
    metaGeo->setVector(101, Vec3d(p1), Vec3d(d), Color3f(1,1,0), "D");
    metaGeo->setVector(102, Vec3d(p1), Vec3d(dirEast*u), Color3f(0,1,1), "E");
    metaGeo->setVector(103, Vec3d(p1), Vec3d(dirNorth*v), Color3f(0,1,1), "S");
    */
    return Vec2d(abs(u),abs(v));
}

Vec2d VRPlanet::fromPosLatLong(Pnt3d p, bool local) { // TODO: increase resolution by enhancing getWorldMatrix
    Pnt3d p1 = p;
    if (local) {
        auto m = origin->getWorldMatrix();
        m.invert();
        m.mult(p,p1);
    }

    Vec3d n = Vec3d(p1);
    n.normalize();
    double north = -toDeg(acos(n[1]))+90;
    double east = toDeg(atan2(-n[2], n[0]));

    if (local) {
        // increase precision by 2D planar approximation
        Vec2d s = fromLatLongSize(north-0.5, east-0.5, north+0.5, east+0.5);
        Vec3d p2 = fromLatLongPosition(north, east, local);
        int pr = cout.precision();
        cout.precision(17);
        cout << "p1: " << p << " p2: " << p2 << endl;
        Vec3d d = Vec3d(p)-p2;
        cout << std::setprecision (15) << Vec2d(north, east);
        north += -d[2]*1.0/s[1];
        east  += d[0]*1.0/s[0];
        cout << std::setprecision (15) << " -> " << Vec2d(north, east) << "  d: " << d << " s: " << s << endl;
        cout.precision(pr);
    }

    cout << "VRPlanet::fromPosLatLong p:" << p1 << " n:" << n << " acos:" << acos(n[1]) << " atan2: " << atan2(-n[2], n[0]) << endl;
    return Vec2d(north, east);
}

void VRPlanet::rebuild() {
    // sphere material
    if (!sphereMat) {
        sphereMat = VRMaterial::create("planet");
        sphereMat->setVertexShader(surfaceVP, "planet surface VS");
        sphereMat->setFragmentShader(surfaceFP, "planet surface FS");
        setLit(false);
    }

    // spheres
    if (lod) lod->destroy();
    lod = VRLod::create("planetLod");
    auto addLod = [&](int i, double d) {
        auto s = VRGeometry::create(getBaseName()+"Sphere");
        s->setPrimitive("Sphere " + toString(radius)+" "+toString(i));
        s->setMaterial(sphereMat);
        lod->addChild(s);
        lod->addDistance(d);
    };

    if (!origin) {
        origin = VRTransform::create("origin");
        addChild(origin);
    }
    origin->addChild(lod);
    anchor = VRObject::create("lod0");
    lod->addChild( anchor );
    addLod(5,radius*1.1);
    addLod(4,radius*2.0);
    addLod(3,radius*5.0);

    if (!metaGeo) setupMetaGeo();
}

void VRPlanet::setParameters( double r, string t, bool l, double s ) {
    radius = r;
    sectorSize = s;
    rebuild();
    setupMaterial(t, l);
}

void VRPlanet::setLayermode( string mode ) {
    if (mode == "full") layermode = 0;
    if (mode == "minimum") layermode = 1;
}

VRWorldGeneratorPtr VRPlanet::addSector( double north, double east, bool local ) {
    auto generator = VRWorldGenerator::create(layermode);
    auto sid = toSID(north, east);
    sectors[sid] = generator;
    anchor->addChild(generator);
    generator->setPlanet(ptr(), Vec2d(north, east));
    generator->setPose( fromLatLongPose(north+0.5*sectorSize, east+0.5*sectorSize) );

    Vec2d size = fromLatLongSize(north, east, north+sectorSize, east+sectorSize);
    for (auto ter:generator->getTerrains()) {
        ter->setLocalized(local);
        ter->setParameters( size, 2, 1);
    }
    generator->setTerrainSize( size );
    return generator;
}

OSMMapPtr VRPlanet::addOSMMap( string path ) {
    if (osmMaps.count(path)) return osmMaps[path];
    OSMMapPtr oMap = OSMMap::loadMap(path);
    osmMaps[path] = oMap;
    return oMap;
}

VRWorldGeneratorPtr VRPlanet::getSector( double north, double east ) {
    auto sid = toSID(north, east);
    if (sectors.count(sid)) return sectors[sid];
    return 0;
}

vector<VRWorldGeneratorPtr> VRPlanet::getSectors() {
    vector<VRWorldGeneratorPtr> res;
    for (auto s : sectors) res.push_back(s.second);
    return res;
}

void VRPlanet::setupMaterial(string texture, bool isLit) { sphereMat->setTexture(texture); setLit(isLit); }
VRMaterialPtr VRPlanet::getMaterial() { return sphereMat; }
void VRPlanet::setLit(bool b) { sphereMat->setShaderParameter("isLit", b?1:0); }

void VRPlanet::setupMetaGeo() {
    metaGeo = VRAnalyticGeometry::create("PlanetMetaData");
    metaGeo->setLabelParams(0.04, true, true, Color4f(0,0,0,1), Color4f(1,1,1,0));
    auto ae = metaGeo->getAnnotationEngine();
    ae->setOutline(4, Color4f(1,1,1,1));
    //ae->setScreenSpace(1);
    origin->addChild(metaGeo);
}

int VRPlanet::addPin( string label, double north, double east, double length ) {
    if (!metaGeo) setupMetaGeo();
    Vec3d n = fromLatLongNormal(north, east);
    Vec3d p = fromLatLongPosition(north, east);
    static int ID = -1; ID++;//metaGeo->getNewID(); // TODO
    metaGeo->setVector(ID, Vec3d(p), Vec3d(n)*length, Color3f(1,1,0.5), label, true);
    return ID;
}

void VRPlanet::remPin(int ID) {}// metaGeo->remove(ID); } // TODO


// shader --------------------------

string VRPlanet::surfaceVP =
"#version 120\n"
GLSL(
varying vec3 tcs;
varying vec3 normal;
varying vec4 position;

attribute vec4 osg_Vertex;
attribute vec4 osg_Normal;
attribute vec4 osg_Color;
attribute vec3 osg_MultiTexCoords0;

void main( void ) {
    tcs = osg_Normal.xyz;
    normal = gl_NormalMatrix * osg_Normal.xyz;
    position = gl_ModelViewMatrix*osg_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix*osg_Vertex;
}
);


string VRPlanet::surfaceFP =
"#version 120\n"
GLSL(
uniform sampler2D tex;
uniform int isLit;

varying vec3 tcs;
varying vec3 normal;
varying vec4 position;

const float pi = 3.1415926;

vec4 color;

void applyLightning() {
	vec3 n = normalize( normal);
	vec3  light = normalize( gl_LightSource[0].position.xyz - position.xyz ); // point light
	float NdotL = max(dot( n, light ), 0.0);
	vec4  ambient = gl_LightSource[0].ambient * color;
	vec4  diffuse = gl_LightSource[0].diffuse * NdotL * color;
	float NdotHV = max(dot(n, normalize(gl_LightSource[0].halfVector.xyz)),0.0);
	vec4  specular = gl_LightSource[0].specular * pow( NdotHV, gl_FrontMaterial.shininess );
	gl_FragColor = diffuse + ambient;// + specular;
	//gl_FragColor = ambient + diffuse + specular;
	//gl_FragColor = vec4(gl_LightSource[0].position.xyz,1);
}

void main( void ) {
	float r = length(tcs);
	float v = 1.0 - acos( tcs.y / r )/pi;
	float u0 = 1 - atan(abs(tcs.z), tcs.x)/pi;
	float u1 = 0.5 - atan(tcs.z, tcs.x)/pi*0.5;
	float u2 = 0.0 - atan(-tcs.z, -tcs.x)/pi*0.5;
	vec4 c1 = texture2D(tex, vec2(u1,v));
	vec4 c2 = texture2D(tex, vec2(u2,v));
	float s = clamp( (tcs.x+1)*0.3, 0, 1 );
	color = mix(c2, c1, u0);
	if (isLit != 0) applyLightning();
	else gl_FragColor = color;
}
);





