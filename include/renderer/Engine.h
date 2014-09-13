#ifndef __engine__
#define __engine__

#include <vector>
#include <map>
#include <iostream>
using namespace std;

const bool USE_CACHE = false;
//if enabled, the first specified resolution remains regardless of further resolutions
//#define INIT_ONCE

class Bone
{
public:
	string name;
	double posX;
	double posY;
	double posZ;

	Bone(const string str, double x, double y, double z)
	{
		name = str;
		posX = x;
		posY = y;
		posZ = z;
	}
};

class MeshData {
	vector<float> vertices;
	vector<float> colors;
	vector<float> normals;
	int cbind;
	int nbind;

public:

	MeshData() {
	}

	MeshData(vector<float> _vertices, vector<float> _colors, int _cbind, vector<float> _normals,
			int _nbind) :
		vertices(_vertices), colors(_colors), normals(_normals), cbind(_cbind), nbind(_nbind) {
	}

	void copy(const MeshData &md) {
		vertices = md.vertices;
		colors = md.colors;
		normals = md.normals;
		cbind = md.cbind;
		nbind = md.nbind;
	}

	void clear() {
		vertices.clear();
		normals.clear();
		colors.clear();

	}
	void setCbind(const int& cbind) {
		this->cbind = cbind;
	}
	void setColors(const vector<float>& colors) {
		this->colors = colors;
	}
	void setNbind(const int& nbind) {
		this->nbind = nbind;
	}
	void setNormals(const vector<float>& normals) {
		this->normals = normals;
	}
	void setVertices(const vector<float>& vertices) {
		this->vertices = vertices;
	}
	const int& getCbind() const {
		return cbind;
	}
	const vector<float>& getColors() const {
		return colors;
	}
	const int& getNbind() const {
		return nbind;
	}
	const vector<float>& getNormals() const {
		return normals;
	}
	const vector<float>& getVertices() const {
		return vertices;
	}
};

class Engine {
protected:
	int width;
	int height;
	double *depthOutput;
	unsigned char *imageOutput;
	int iActive;
	std::vector<string> vecFilename;
	/*double distance;
	double elevation;
	double azimuth;
	double yaw;
	bool sphereOrientation;
	double *A;
	double *R;
	double *T;*/
	double *AOutput;
	double *ROutput;
	double *TOutput;
	double *unprojectOutput;
	std::vector<MeshData> vecMeshData;
	string writeFiles;
	bool lighting;
	bool offScreen;
	//bool getUnproject;
	double distanceInc;
	string deg_order;
public:
	Engine();
	virtual ~Engine();

	void setParams(int width, int height, double *depthOutput, unsigned char *imageOutput,
			string filename, double distance, double elevation, double azimuth, double yaw,
			bool sphereOrientation, double *A, double *R, double *T, double *unprojectOutput,
			MeshData meshData, string writeFiles, bool offScreen, double *AOutput, double *ROutput,
			double *TOutput, bool lighting,
					double distanceInc, string deg_order, bool getUnproject);

	void setMultiModeParams(int width, int height,
		double distance, double elevation, double azimuth, double yaw, bool sphereOrientation, double *A, double *R, double *T, double *unprojectOutput,
		string writeFiles, bool offScreen, bool lighting, double distanceInc, string deg_order, bool getUnproject);

	
	double axisX;
	double distance;
	double axisZ;
	double azimuth;
	double elevation;
	double yaw;
	bool sphereOrientation;
	double *A;
	double *R;
	double *T;
	bool getUnproject;
	bool multiMode;

	const MeshData& getMeshData(int i=-1) const {
		if(i=-1)
		{
			i = iActive;
		}
		return vecMeshData.at(i);
	}

	MeshData& getmeshData(int i=-1)
	{
		if(i=-1)
		{
			i = iActive;
		}
		return vecMeshData.at(i);
	}

	const string& getFilename(int i=-1) const
	{
		if(i=-1)
		{
			i = iActive;
		}
		return vecFilename.at(i);
	}

	string& getfilename(int i=-1)
	{
		if(i=-1)
		{
			i = iActive;
		}
		return vecFilename.at(i);
	}

	virtual void init()=0;
	virtual void initDataFromFile()=0;
	virtual void initData()=0;
	virtual void initCamera()=0;
	virtual void initCanvas()=0;
	virtual void draw()=0;
	virtual void InitFrame()=0;
	virtual void frame()=0;
	virtual void SetUnproj()=0;
	virtual void shutdown()=0;
	virtual bool IsDone()=0;
	
	virtual void startMultiMode()=0;
	virtual int addModel(string filename, MeshData meshData, bool light, bool removeInterior, bool autoCenter, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw)=0;
	virtual void setModelTransformation(int index, double axisX, double axisY, double axisZ, double azimuth, double elevation, double yaw)=0;
	virtual void setModelTransformation(int index, double* A, double* R, double* T)=0;
	virtual void setModelVisibility(int index, bool show)=0;

	virtual void SetBackgroundImage(unsigned char *rgbImage, int width, int height)=0;

	virtual map<string, Bone*>* getModelBones(int index)=0;
	virtual void moveModelBones(int index, vector<Bone*>* vec)=0;

	virtual void RegisterForImages(unsigned char* mem)=0;
};

#endif // __engine__
