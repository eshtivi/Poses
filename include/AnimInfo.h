#ifndef __ANIMINFO__
#define __ANIMINFO__

#include <iostream>
#include <sstream>

using namespace std;

class AnimInfo
{
public:
	string target;
	bool isBone;
	int point;
	int anchor;

	double weightX, weightY, weightZ;

	AnimInfo(string targetName,	bool isTargetBone, 
		int nPoint, int nAnchorPoint=-1,
		double weightX=1, double weightY=1, double weightZ=1)
	{
		this->target = *(new string(targetName));
		this->isBone = isTargetBone;
		this->point = nPoint;
		this->anchor = nAnchorPoint;

		this->weightX = weightX;
		this->weightY = weightY;
		this->weightZ = weightZ;
	}
};

#endif