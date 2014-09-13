#ifndef __RENDERER__
#define __RENDERER__

#include <iostream>
#include <sstream>

#include "util\util.h";
#include "renderer\Engine.h";
#include "renderer\EngineOSG.h";
#include "ModelInfo.h";

using namespace std;

class Renderer
{

public: void run(int width, int height, double *depthOutput, unsigned char * imageOutput, string filename,
		double distance, double elevation, double azimuth, double yaw, bool sphereOrientation,
		double *A, double *R, double *T, double *unprojectOutput, MeshData &meshData,
		string writeFiles, bool offScreen, double *AOutput, double *ROutput, double *TOutput,
		bool lighting, double distanceInc, string deg_order, bool getUnproject);

		void Show(int width, int height, string filename, double distance, double elevation, double azimuth, double yaw,
			MeshData &meshData, bool lighting);
		void frame();
		void frame(double elevation, double azimuth, double yaw);
		void frame(double elevation, double azimuth, double yaw, double distance, double test1, double test3);
		void frame(double* _A, double* _R, double* _T);
		void unproject(int width, int height, string filename, MeshData &meshData, bool lighting, double* output);
		void stop();
		bool IsDone();
		void StartMultiMode(int width, int height, bool offScreen, bool lighting, double* refA, bool sphere);
		int AddModel(ModelInfo* info, MeshData meshData);
		void SetModelPosition(int index, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw);
		void SetModelPosition(int index, double* A, double* R, double* T);
		void SetModelVisibility(int index, bool show);
		void SetBackgroundImage(unsigned char *rgbImage, int width, int height);
		map<string, Bone*>* getModelBones(int index);
		void moveModelBones(int index, vector<Bone*>* vec);
		void RegisterForImageOutput(unsigned char* imageOutput);
private:
	Engine *engine;
};

#endif