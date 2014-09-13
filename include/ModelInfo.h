#ifndef __MODELINFO__
#define __MODELINFO__

#include <iostream>
#include <sstream>
#include "AnimInfo.h"

using namespace std;

class ModelInfo
{
public:
	string filename;
	bool light;
	bool removeInterior;
	bool autoCenter;
	double azimuth;
	double elevation;
	double yaw;
	double axisX;
	double axisY;
	double axisZ;
	int animCount;
	AnimInfo** arrAnim;

	ModelInfo(string filename,
		bool light, bool removeInterior=false, bool autoCenter=false,
		double pi_azimuth=0, double pi_elevation=0, double pi_yaw=0,
		double axisX=0, double axisY=0, double axisZ=0,
		int nAnim=0, AnimInfo** animArray=NULL)
	{
		this->filename = *(new string(filename));

		this->light = light;
		this->removeInterior = removeInterior;
		this->autoCenter = autoCenter;

		this->azimuth	= M_PI * pi_azimuth;
		this->elevation = M_PI * pi_elevation;
		this->yaw		= M_PI * pi_yaw;

		this->axisX = axisX;
		this->axisY = axisY;
		this->axisZ = axisZ;

		this->animCount = nAnim;
		this->arrAnim = animArray;
	}
};

#endif