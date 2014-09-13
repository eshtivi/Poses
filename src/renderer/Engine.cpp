#include "renderer\Engine.h"
Engine::Engine() {
}

Engine::~Engine() {
}

void Engine::setParams(int width, int height, double *depthOutput, unsigned char *imageOutput,
		string filename, double distance, double elevation, double azimuth, double yaw,
		bool sphereOrientation, double *A, double *R, double *T, double *unprojectOutput,
		MeshData meshData, string writeFiles, bool offScreen, double *AOutput, double *ROutput,
		double *TOutput, bool lighting, double distanceInc, string deg_order, bool getUnproject) {
	this->iActive = this->vecFilename.size();
	this->width = width;
	this->height = height;
	this->depthOutput = depthOutput;
	this->imageOutput = imageOutput;
	this->vecFilename.push_back(filename);
	this->distance = distance;
	this->elevation = elevation;
	this->azimuth = azimuth;
	this->yaw = yaw;
	this->sphereOrientation = sphereOrientation;
	this->A = A;
	this->R = R;
	this->T = T;
	this->unprojectOutput = unprojectOutput;
	this->vecMeshData.push_back(meshData);
	this->writeFiles = writeFiles;
	this->offScreen = offScreen;
	this->AOutput = AOutput;
	this->ROutput = ROutput;
	this->TOutput = TOutput;
	this->lighting = lighting;
	this->distanceInc = distanceInc;
	this->deg_order = deg_order;
	this->getUnproject = getUnproject;
	this->multiMode = false;
}

void Engine::setMultiModeParams(int width, int height, double distance, double elevation, double azimuth, double yaw, bool sphereOrientation,
								double *A, double *R, double *T, double *unprojectOutput, string writeFiles, bool offScreen, bool lighting,
								double distanceInc, string deg_order, bool getUnproject)
{
	this->iActive = -1;
	this->width = width;
	this->height = height;
	this->depthOutput = depthOutput;
	this->imageOutput = imageOutput;
	this->distance = distance;
	this->elevation = elevation;
	this->azimuth = azimuth;
	this->yaw = yaw;
	this->sphereOrientation = sphereOrientation;
	this->A = A;
	this->R = R;
	this->T = T;
	this->unprojectOutput = unprojectOutput;
	this->writeFiles = writeFiles;
	this->offScreen = offScreen;
	this->AOutput = NULL;
	this->ROutput = NULL;
	this->TOutput = NULL;
	this->lighting = lighting;
	this->distanceInc = distanceInc;
	this->deg_order = deg_order;
	this->getUnproject = getUnproject;
	this->multiMode = multiMode;
	this->startMultiMode();
}
