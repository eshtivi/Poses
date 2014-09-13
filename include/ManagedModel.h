#include <iostream>
#include <sstream>
#include "util\util.h";
#include "renderer\renderer.h";
#include "ModelInfo.h";


using namespace std;

class ManagedModel
{
private:
	Renderer* renderer;
	MeshData meshData;
	string filename;
	bool lighting;
	ModelInfo* info;
	map<string, Bone*>* bones;

	bool visible;
	int id;

	double* difPos;

	cv::Mat GetCameraMatrix(double* A, double* R, double* T, int width, int height)
{
	double* mv = new double[16];
	double* proj = new double[16];
	double* win = new double[16];

	getOpenGLMatrices(A, R, T, width, height, mv, proj);
	
	memset(win, 0, sizeof(double)*16);

	win[0 + 4*0] = width/2;
	win[1 + 4*1] = height/2;
	win[2 + 4*2] = 0.5;
	win[0 + 4*3] = width/2;
	win[1 + 4*3] = height/2;
	win[2 + 4*3] = 0.5;
	win[3 + 4*3] = 1;

	cv::Mat mvMat(4, 4, CV_64FC1, mv);
	cv::Mat projMat(4, 4, CV_64FC1, proj);
	cv::Mat winMat(4, 4, CV_64FC1, win);
	
	cv::Mat camMat(4, 4, CV_64FC1);

	camMat = mvMat * projMat * winMat;

	mvMat.release();
	projMat.release();
	winMat.release();
	delete mv;
	delete proj;
	delete win;

	return camMat;
}

	// unproject
	void TwoToThree(double* in, double *out, cv::Mat inv)
	{
		double xyzTwo[4];
		double xyzThree[4];

		xyzTwo[0] = in[0];
		xyzTwo[1] = in[1];
		xyzTwo[2] = in[2];
		xyzTwo[3] = in[3];

		cv::Mat vecThree(1, 4, CV_64FC1, xyzThree);
		cv::Mat vecTwo(1, 4, CV_64FC1, xyzTwo);

		vecThree = vecTwo * inv;

		out[0] = xyzThree[0] / xyzThree[3];
		out[1] = xyzThree[1] / xyzThree[3];
		out[2] = xyzThree[2] / xyzThree[3];
	}

	// project
	void ThreeToTwo(double* in, double *out, cv::Mat cam)
	{
		double xyzTwo[4];
		double xyzThree[4];

		xyzThree[0] = in[0];
		xyzThree[1] = in[1];
		xyzThree[2] = in[2];
		xyzThree[3] = 1;

		cv::Mat vecThree(1, 4, CV_64FC1, xyzThree);
		cv::Mat vecTwo(1, 4, CV_64FC1, xyzTwo);

		vecTwo = vecThree * cam;

		out[0] = xyzTwo[0];
		out[1] = xyzTwo[1];
		out[2] = xyzTwo[2];
		out[3] = xyzTwo[3];
	}

	// A,R,T - camera matrices
	// twodee - 2D coordinates of current detected face
	// threedee - 3D points of reference static face model
	void CalcNewPosition(double* A, double* R, double* T, double* twodee, double* threedee, int width, int height)
	{
		// projection from 3D {X,Y,Z} to 2D {X,Y} returns {A,B,C}
		// to get 2D coordinates: {X = A/C, Y = B/C}
		// in order to unproject from 2D {X,Y} to 3D {X,Y,Z}, that C is required
		// therefore, this method first project the original points in threedee to get that C
		// then it uses that C (of each point) to unproject back the current twodee points into 3D

		// get camera matrix for projection and its inverse for unprojection
		cv::Mat cam = this->GetCameraMatrix(A, R, T, width, height);
		cv::Mat inv = cam.inv();

		// foreach face point
		for(int i=0; i<49; i++)
		{
			double ab[4];
			double origXYZ[3] = {threedee[3*i], threedee[3*i+1], threedee[3*i+2]};
			double newXYZ[3];

			// project original threedee point on current 2d face
			ThreeToTwo(origXYZ, ab, cam);

			// use C to calculate A & B of current twodee point
			ab[0] = twodee[2*i]*ab[2];
			ab[1] = (height-twodee[2*i+1])*ab[2];

			// unproject to 3d
			TwoToThree(ab, newXYZ, inv);

			// set diff of new and old 3d point in each dimention
			difPos[3*i + 0] = newXYZ[0] - origXYZ[0];
			difPos[3*i + 1] = newXYZ[1] - origXYZ[1];
			difPos[3*i + 2] = newXYZ[2] - origXYZ[2];
		}

		// facial expressions interfering pose estimation, causing an offset
		// point index 12 (13th starting from 0) is sitting on the nose's bone
		// it's unlikely to move and still has an average offset
		// substracting its diff gives better results
		for(int i=0; i<49; i++)
		{
			if(i != 12)
			{
				difPos[3*i+0] -= difPos[3*12+0];
				difPos[3*i+1] -= difPos[3*12+1];
				difPos[3*i+2] -= difPos[3*12+2];
			}
		}
	}

	Bone* CreateDestinationBone(string name, int n, double weightX, double weightY, double weightZ)
	{
		return new Bone(name,
			difPos[3*n+0]*weightX,
			difPos[3*n+1]*weightY,
			difPos[3*n+2]*weightZ);
	}

	Bone* CreateDestinationAnchoredBone(string name, int nPoint, int nAnchor, double weightX, double weightY, double weightZ)
	{
		return new Bone(name,
			(difPos[nPoint*3+2] - difPos[nAnchor*3+2])*weightX,
			(difPos[nPoint*3+2] - difPos[nAnchor*3+2])*weightY,
			(difPos[nPoint*3+2] - difPos[nAnchor*3+2])*weightZ);
	}

	Bone* CreateAnimationBone(AnimInfo* ai)
	{
		Bone* b;

		if(ai->anchor >= 0)
		{
			b = CreateDestinationAnchoredBone(ai->target, ai->point, ai->anchor, ai->weightX, ai->weightY, ai->weightZ);
		}
		else
		{
			b = CreateDestinationBone(ai->target, ai->point, ai->weightX, ai->weightY, ai->weightZ);
		}

		//swap(b->posZ, b->posY); //TODO - swap animation yz param/cfg

		return b;
	}

public:
	ManagedModel(Renderer* renderer, ModelInfo* info)
	{
		this->renderer = renderer;
		this->info = info;

		this->id = renderer->AddModel(info, meshData);

		this->visible=true;
	}

	void Hide()
	{
		if(visible)
		{
			renderer->SetModelVisibility(id, false);
			visible = false;
		}
	}

	void Show()
	{
		if(!visible)
		{
			renderer->SetModelVisibility(id, true);
			visible = true;
		}
	}

	void Show(double tempAzimuth, double tempElevation, double tempYaw,
				double tempX=0, double tempY=0, double tempZ=0)
	{
		tempX += info->axisX;
		tempY += info->axisY;
		tempZ += info->axisZ;

		if(!visible)
		{
			renderer->SetModelVisibility(id, true);
			visible = true;
		}

		renderer->SetModelPosition(id, tempX, tempY, tempZ, tempAzimuth, tempElevation, tempYaw);
	}

	void Show(double* A, double* R, double* T)
	{
		if(!visible)
		{
			renderer->SetModelVisibility(id, true);
			visible = true;
		}

		renderer->SetModelPosition(id, A, R, T);
	}

	void InitAnimation()
	{
		bones = renderer->getModelBones(id);
		
		difPos = new double[3*49];
	}

	void Animate(double* twodee, double* threedee, double* A, double* R, double* T, int width, int height)
	{
		vector<Bone*> vec;

		CalcNewPosition(A, R, T, twodee, threedee, width, height);

		for(int i=0; i<info->animCount; i++)
		{
			Bone* b = CreateAnimationBone(info->arrAnim[i]);

			// if not bone, position won't be reset
			// need to track changes and apply only delta from previous change
			// update: removed bone's update callback. need to track bones as well
			//if(!info->arrAnim[i]->isBone)
			{
				double tempX = b->posX;
				double tempY = b->posY;
				double tempZ = b->posZ;

				Bone* trace = bones->find(b->name)->second;

				b->posX -= trace->posX;
				b->posY -= trace->posY;
				b->posZ -= trace->posZ;

				trace->posX = tempX;
				trace->posY = tempY;
				trace->posZ = tempZ;
			}

			//cout << b->name << tab << b->posY << endl;

			vec.push_back(b);
		}

		renderer->moveModelBones(id, &vec);
	}
};