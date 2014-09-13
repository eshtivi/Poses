#include "renderer\renderer.h"

using namespace std;

void Renderer::run(int width, int height, double *depthOutput, unsigned char * imageOutput, string filename,
		double distance, double elevation, double azimuth, double yaw, bool sphereOrientation,
		double *A, double *R, double *T, double *unprojectOutput, MeshData &meshData,
		string writeFiles, bool offScreen, double *AOutput, double *ROutput, double *TOutput,
		bool lighting, double distanceInc, string deg_order, bool getUnproject) {
	
	engine = new EngineOSG();
	engine->setParams(width, height, depthOutput, imageOutput, filename, distance, elevation,
			azimuth, yaw, sphereOrientation, A, R, T, unprojectOutput, meshData, writeFiles,
			offScreen, AOutput, ROutput, TOutput, lighting, distanceInc, deg_order, getUnproject);
	engine->init();

	try
	{
		if (meshData.getVertices().empty()) {
			engine->initDataFromFile();
		} else {
			engine->initData();
		}
		meshData.copy(engine->getMeshData());
		engine->initCamera();

		engine->initCanvas();
		engine->draw();
		engine->draw(); // double draw since some models require 2 frames to build up properly
	}
	catch (char const *e)
	{
		engine->shutdown();
		delete engine;
		throw e;
	}
	engine->shutdown();
	delete engine;
}

void Renderer::Show(int width, int height, string filename, double distance, double elevation, double azimuth, double yaw,
			MeshData &meshData, bool lighting)
{
	
	engine = new EngineOSG();

	double *A = new double[9];
	double *R = new double[9];
	double *T = new double[3];

	engine->setParams(width, height, NULL, NULL, filename, distance, elevation,
			azimuth, yaw, true, /*NULL, NULL, NULL*/A,R,T, NULL, meshData, "",
			false, NULL, NULL, NULL, lighting, 50, "zxy", false);
	engine->init();

	try
	{
		if (meshData.getVertices().empty()) {
			engine->initDataFromFile();
		} else {
			engine->initData();
		}
		meshData.copy(engine->getMeshData());
		engine->initCamera();

		engine->initCanvas();

		engine->InitFrame();
		engine->frame();
	}
	catch (char const *e)
	{
		engine->shutdown();
		delete engine;
		throw e;
	}
}

void Renderer::frame()
{
	engine->frame();
}

void Renderer::frame(double* _A, double* _R, double* _T)
{
	size_t s = sizeof(double);
	memcpy(engine->A, _A, 9*s);
	memcpy(engine->R, _R, 9*s);
	memcpy(engine->T, _T, 3*s);
	engine->sphereOrientation = false;
	if(engine->getUnproject)
	{
		engine->SetUnproj();
	}
	engine->frame();
}

void Renderer::frame(double _elevation, double _azimuth, double _yaw)
{
	engine->elevation = _elevation;
	engine->azimuth = _azimuth;
	engine->yaw = _yaw;
	engine->sphereOrientation = true;
	engine->frame();
}

void Renderer::frame(double _elevation, double _azimuth, double _yaw, double _distance, double _test1, double _test3)
{
	engine->elevation = _elevation;
	engine->azimuth = _azimuth;
	engine->yaw = _yaw;
	engine->distance = _distance;
	engine->axisX = _test1;
	engine->axisZ = _test3;
	engine->sphereOrientation = true;
	engine->frame();
}

void Renderer::unproject(int width, int height, string filename, MeshData &meshData, bool lighting, double* output)
{
	engine = new EngineOSG();

	double *A = new double[9];
	double *R = new double[9];
	double *T = new double[3];

	engine->setParams(width, height, NULL, NULL, filename, 0, 0,
			0, 0, false, /*NULL, NULL, NULL*/A,R,T, output, meshData, "",
			true, NULL, NULL, NULL, lighting, 50, "zxy", true);
	engine->init();

	try
	{
		if (meshData.getVertices().empty()) {
			engine->initDataFromFile();
		} else {
			engine->initData();
		}
		meshData.copy(engine->getMeshData());
		engine->initCamera();

		engine->initCanvas();

		engine->InitFrame();
	}
	catch (char const *e)
	{
		engine->shutdown();
		delete engine;
		throw e;
	}
}

void Renderer::stop()
{
	engine->shutdown();
	delete engine;
}

bool Renderer::IsDone()
{
	return engine->IsDone();
}

void Renderer::StartMultiMode(int width, int height, bool offScreen, bool lighting, double* refA, bool sphere)
{
	engine = new EngineOSG();

	double* A = new double[9];
	double* R = new double[9];
	double* T = new double[3];

	memcpy(A, refA, sizeof(double)*9);
	memset(R, 0, sizeof(double)*9);
	memset(T, 0, sizeof(double)*3);

	R[0] = 1;
	R[4] = 1;
	R[8] = 1;

	A[4] = A[0];

	engine->setMultiModeParams(width, height, 0, 0, 0, 0, sphere, /*NULL, NULL, NULL*/A, R, T, NULL, "",	offScreen, lighting, 50, "zxy", false);
	engine->init();

	try
	{
		engine->initCamera();
		engine->initCanvas();
		engine->InitFrame();
	}
	catch (char const *e)
	{
		engine->shutdown();
		delete engine;
		throw e;
	}
}

int Renderer::AddModel(ModelInfo* info, MeshData meshData)
{
	return engine->addModel(info->filename, meshData, info->light, info->removeInterior, info->autoCenter, 0,0,0/*info->axisX, info->axisY, info->axisZ*/, info->azimuth, info->elevation, info->yaw);
}

void Renderer::SetModelPosition(int index, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw)
{
	engine->setModelTransformation(index, axisX, axisY, axisZ, azimuth, elevation, yaw);
}

void Renderer::SetModelPosition(int index, double* A, double* R, double* T)
{
	engine->setModelTransformation(index, A, R, T);
}

void Renderer::SetModelVisibility(int index, bool show)
{
	engine->setModelVisibility(index, show);
}

void Renderer::SetBackgroundImage(unsigned char *rgbImage, int width, int height)
{
	engine->SetBackgroundImage(rgbImage, width, height);
}

map<string, Bone*>* Renderer::getModelBones(int index)
{
	return engine->getModelBones(index);
}

void Renderer::moveModelBones(int index, vector<Bone*>* vec)
{
	engine->moveModelBones(index, vec);
}

void Renderer::RegisterForImageOutput(unsigned char* imageOutput)
{
	engine->RegisterForImages(imageOutput);
}
