#ifndef __engineosg__
#define __engineosg__

#include <osgViewer/Viewer>
#include <osgAnimation/Bone>
#include "Engine.h"

class EngineOSG : public Engine {
	osg::ref_ptr<osgViewer::Viewer> viewer;
	osg::ref_ptr<osg::Image> imgBG;
	unsigned char* memBG;
	bool initBG;
	//vector<osgAnimation::BoneMap*> vecBoneMaps;
	vector<map<string, osg::MatrixTransform*>*> vecBoneMaps;

	void drawFromFileAndData();
	void drawPoints(vector<GLfloat> &vertices);
	void setupSphereOrientation();
	void getIntrisnicMatrix();
public:
	EngineOSG();
	virtual ~EngineOSG();
	void init();
	void initDataFromFile();
	void initData();
	void initCamera();
	void initCanvas();
	void draw();
	void InitFrame();
	void frame();
	void SetUnproj();
	void shutdown();
	bool IsDone();
	void startMultiMode();
	int addModel(string filename, MeshData meshData, bool light, bool removeInterior, bool autoCenter, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw);
	void setModelTransformation(int index, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw);
	void setModelTransformation(int index, double* A, double* R, double* T);
	void setModelVisibility(int index, bool show);
	void SetBackgroundImage(unsigned char *rgbImage, int width, int height);
	map<string, Bone*>* getModelBones(int index);
	void moveModelBones(int index, vector<Bone*>* vec);
	void RegisterForImages(unsigned char* mem);
	
friend class CaptureCB;
friend class TextureCB;
friend class KeyboardEventHandler;
friend class ContinuousImageCB;
};
#endif // __engineosg__
