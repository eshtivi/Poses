#include <iostream>
#include <sstream>

#include <fstream>

#include <Windows.h>
#include <math.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <FaceAlignment.h>
#include <XXDescriptor.h>

#include "calib\calib.h"
#include "renderer\renderer.h"

#include "ConfigHelper.h";
#include "ManagedModel.h";




using namespace std;

#define FACEPOINTS 49

#define RND 50
#define RSZ 0.25

string strCurrentFolder;
string strXmlPath;
string strRefModelPath;

ConfigHelper *cfg;

string strInputImage;
string strInputDepth;
string strOutputImage;
string strOutputDepth;
bool useWebcam = false;
bool blackBackground = false;

Calib *c;
Renderer* r;
MeshData meshData;

void PrintToFile3(double* mat, int width, int height, string name)
{
	ofstream myfile;
	myfile.open ("d:\\workshop\\" + name + ".txt");
	
	for (int i = 0; i < height; i++)
	{
		myfile << "===Row " << i << " ===" << endl;
		for (int j = 0;j < width;j++)
		{
			int offset = (i*width+j)*3;
			myfile << "[" << tab << mat[offset] << tab << mat[offset+1] << tab << mat[offset+2] << tab << "]" << endl;
		}

		myfile << endl << endl;
	}

	myfile.close();
}



void ParseArgs(int argc, const char* argv[])
{
	char exePath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, exePath);
	strcat(exePath, "\\");
	strCurrentFolder = string(exePath);


	if(argc < 2)
	{
		strXmlPath = "Config.xml";
	}
	else
	{
		strXmlPath = argv[1];
	}
}

cv::Mat RgbToMat(unsigned char* rgb, int width, int height)
{
	cv::Mat img = cv::Mat(height, width, CV_8UC3);

	for (int i = 0;i < height;i++)
	{
		for (int w = 0;w < width;w++)
		{
			img.at<cv::Vec3b>(i,w)[0] = rgb[height*width*2+w*height+i];
			img.at<cv::Vec3b>(i,w)[1] = rgb[height*width+w*height+i];
			img.at<cv::Vec3b>(i,w)[2] = rgb[w*height+i];
		}
	}

	return img;
}

void RunThing(int width, int height)
{
	double distanceInc = 100;
	string deg_order = "zxy";
	
	double elevation = 0;
	double azimuth = 0;
	double yaw = 0;

	double tA[9];
	double tR[9];
	double TOutput[9];

	double* depthOutput = new double[width*height];
	unsigned char *imageOutput = new unsigned char[width*height*3];
	double *unprojectOutput = new double[width*height*3];

	bool writeFiles = false;
	bool offScreen = false;
	bool lighting = true;
	bool getUnproject = false;
	
	try
	{
			sout.str("");

			r->run(width, height, depthOutput, imageOutput, strInputImage/*strRefModelPath*/, 0, osg::DegreesToRadians(elevation),
					osg::DegreesToRadians(azimuth), osg::DegreesToRadians(yaw), /*sphere orientation*/true, NULL, NULL,
					NULL, unprojectOutput, meshData, "", offScreen, tA, tR, TOutput, lighting,
					distanceInc, deg_order, getUnproject);
			printf("%s", sout.str().c_str());

	}
	catch (char const *e)
	{
		//execution errors
		printf("%s", sout.str().c_str());
	}

}

bool GenerateRefModelOutput(int width, int height, double* depthOutput, unsigned char *imageOutput, double *unprojectOutput, double *AOutput, double *ROutput, double *TOutput)
{
	double distanceInc = 1;
	string deg_order = "zyx";
	
	ModelInfo* info = cfg->GetProcessingModel();

	double elevation = info->elevation;
	double azimuth = info->azimuth;
	double yaw = info->yaw;

	double tA[9];
	double tR[9];

	bool writeFiles = true;
	bool offScreen = true;
	bool lighting = info->light;
	bool getUnproject = true;
	
	try
	{
			sout.str("");

			r->run(width, height, depthOutput, imageOutput, info->filename, info->axisZ, elevation, azimuth, yaw,/*sphere orientation*/true,
				NULL, NULL, NULL, unprojectOutput, meshData, strCurrentFolder + "RefModelOutput.bmp", offScreen, tA, tR, TOutput, lighting,
					distanceInc, deg_order, getUnproject);
			printf("%s", sout.str().c_str());

			transpose(tA, 3, 3, AOutput);
			transpose(tR, 3, 3, ROutput);
	}
	catch (char const *e)
	{
		//execution errors
		printf("%s", sout.str().c_str());
		throw;
	}

	return true;
}

bool GenerateUnproject(string filename, int width, int height, double *unprojectOutput, /*double *AOutput,*/ double* Ain, double* Rin, double* Tin)
{
	double distanceInc = 1;
	string deg_order = "zxy";

	bool writeFiles = false;
	bool offScreen = true;
	bool lighting = true;
	bool getUnproject = true;
	
	double* depthOutput = new double[width*height];
	unsigned char* imageOutput= new unsigned char[width*height*3];
	double *AOutput = new double[9];


	try
	{
			sout.str("");

			r->run(width, height, depthOutput, imageOutput, filename, 0, 0, 0, 0, /*sphere orientation*/false,
					Ain, Rin, Tin, unprojectOutput, meshData, "", offScreen, AOutput, NULL, NULL,
					lighting, distanceInc, deg_order, getUnproject);
			printf("%s", sout.str().c_str());
	}
	catch (char const *e)
	{
		//execution errors
		printf("%s", sout.str().c_str());

		delete depthOutput;
		delete imageOutput;

		return false;
	}

	delete depthOutput;
	delete imageOutput;
	delete AOutput;

	return true;
}

char detectionModel[] = "DetectionModel-v1.5.bin";
char trackingModel[] = "TrackingModel-v1.10.bin";
string faceDetectionModel("haarcascade_frontalface_alt2.xml");
INTRAFACE::FaceAlignment *fa;
cv::CascadeClassifier* face_cascade;
INTRAFACE::XXDescriptor* xxd;

int InitDetect()
{
	char* detection = new char[strCurrentFolder.length() + strlen(detectionModel) + 1];
	strcpy(detection, strCurrentFolder.c_str());
	strcat(detection, detectionModel);
	
	char* tracking = new char[strCurrentFolder.length() + strlen(trackingModel) + 1];
	strcpy(tracking, strCurrentFolder.c_str());
	strcat(tracking, trackingModel);

	xxd = new INTRAFACE::XXDescriptor(4);
	fa = new INTRAFACE::FaceAlignment(detection, tracking, xxd);
	face_cascade = new cv::CascadeClassifier();

	if (!fa->Initialized()) {
		cerr << "FaceAlignment cannot be initialized." << endl;
		return -1;
	}
	
	if( !face_cascade->load( strCurrentFolder + faceDetectionModel ) )
	{ 
		cerr << "Error loading face detection model." << endl;
		return -1; 
	}

	return 0;
}

int Detect(cv::Mat* output, cv::Mat frame)
{
	bool drawing_box = false;
	//cv::Mat X;
	cv::Rect box;
	
	vector<cv::Rect> faces;
	float score, notFace = 0.5;
	// face detection
	face_cascade->detectMultiScale(frame, faces, 1.2, 2, 0, cv::Size(50, 50));
	
	int retVal = -1;

	for (int i = 0 ;i < faces.size(); i++)
	{
		// face alignment
		if (fa->Detect(frame,faces[i],*output,score) == INTRAFACE::IF_OK)
		{
			// only draw valid faces
			if (score >= notFace)
			{
				retVal = 0;

				//for (int j = 0 ; j < output->cols ; j++)
				//	cv::circle(frame,cv::Point((int)output->at<float>(0,j), (int)output->at<float>(1,j)), 1, cv::Scalar(0,255,0), -1);

				break;
			}
		}
	}
	return retVal;
}

vector<cv::Mat*>* MultiDetect(cv::Mat frame)
{
	vector<cv::Rect> faces;
	float score, notFace = 0.5;
	// face detection
	face_cascade->detectMultiScale(frame, faces, 1.2, 2, 0, cv::Size(50, 50));
	
	vector<cv::Mat*>* vec = new vector<cv::Mat*>();

	cv::Mat tempMat;

	for (int i = 0 ;i < faces.size(); i++)
	{
		// face alignment
		if (fa->Detect(frame,faces[i], tempMat,score) == INTRAFACE::IF_OK)
		{
			// only draw valid faces
			if (score >= notFace)
			{
				cv::Mat* mat = new cv::Mat(tempMat.size(), tempMat.type());
				tempMat.copyTo(*mat);
				vec->push_back(mat);
			}
		}
	}

	return vec;
}

int Track(cv::Mat* output, cv::Mat prev, cv::Mat frame)
{
	float score, notFace = 0.5;

	int retVal = -1;

	if (fa->Track(frame,prev,*output,score) == INTRAFACE::IF_OK)
	{
		if(score > notFace)
		{
			retVal = 0;
		}
	}

	return retVal;
}


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

void PaintOriginal(cv::Mat img, double* threedee, double* A, double* R, double* T, int width, int height)
{
	cv::Mat cam = GetCameraMatrix(A, R, T, width, height);
	for(int j=0; j<FACEPOINTS; j++)
	{
		double xyzTwo[4];
		double xyzThree[4];

		xyzThree[0] = threedee[j*3+0];
		xyzThree[1] = threedee[j*3+1];
		xyzThree[2] = threedee[j*3+2];
		xyzThree[3] = 1;

		cv::Mat vecThree(1, 4, CV_64FC1, xyzThree);
		cv::Mat vecTwo(1, 4, CV_64FC1, xyzTwo);

		vecTwo = vecThree * cam;

		cv::circle(img,cv::Point((int)(xyzTwo[0] / xyzTwo[2]), (int)(height - (xyzTwo[1] / xyzTwo[2]))), 1, cv::Scalar(255,0,0), -1);
	}
}

void PaintFace(cv::Mat img, cv::Mat face)
{
	int minX, minY, maxX, maxY;
	minX = maxX = face.at<float>(0,0);
	minY = maxY = face.at<float>(1,0);

	for (int j = 0 ; j < face.cols ; j++)
	{
		cv::circle(img,cv::Point((int)face.at<float>(0,j), (int)face.at<float>(1,j)), 1, cv::Scalar(0,255,0), -1);

		float currX = face.at<float>(0, j);
		float currY = face.at<float>(1, j);
		
		maxX = currX > maxX ? currX : maxX;
		minX = currX < minX ? currX : minX;

		maxY = currY > maxY ? currY : maxY;
		minY = currY < minY ? currY : minY;
	}

	cv::rectangle(img, cv::Point(minX-5, minY-5), cv::Point(maxX+5, maxY+5), cv::Scalar(0,255,0));
}

bool WithinRange(int x1, int y1, int x2, int y2, int px, int py)
{
	return (px < max(x1, x2) &&
			px > min(x1, x2) &&
			py < max(y1, y2) &&
			py > min(y1, y2));
}

void GetThreedee(int width, int height, double* unproject, cv::Mat faceDetected,	// input
				 double* threedee)													// output
{
	for (int i = 0 ; i < faceDetected.cols ; i++)
	{
		//cv::circle(ref_mat,cv::Point((int)faceDetected.at<float>(0,i), (int)faceDetected.at<float>(1,i)), 1, cv::Scalar(0,255,0), -1);
				
		//zxy -> idx will point to xy
		int a = faceDetected.at<float>(0,i);
		int b = faceDetected.at<float>(1,i);
		int idx = (int)(a*height + b);
		threedee[i*3] = unproject[idx];
		threedee[i*3 + 1] = unproject[width*height + idx];
		threedee[i*3 + 2] = unproject[width*height*2 + idx];
	}
}

void GetTwodee(cv::Mat faceDetected,
				double* twodee)
{
	for (int i = 0 ; i < faceDetected.cols ; i++)
	{
		//cv::circle(faceImage,cv::Point((int)faceDetected.at<float>(0,i), (int)faceDetected.at<float>(1,i)), 1, cv::Scalar(0,255,0), -1);
				
		int a = faceDetected.at<float>(0,i);
		int b = faceDetected.at<float>(1, i);

		twodee[i*2] = a;
		twodee[i*2 + 1] = b;
	}
}

void DoCalib(int width, int height, double* twodee, double* threedee,
			double* outA, double* outR, double* outT,
			double* inA, double* inR = NULL, double* inT = NULL)
{
	double tempA[9], tempR[9];
	
	/*double partial2[9*2];
	double partial3[9*3];
	for(int i=0; i<=9; i++)
	{
		partial2[2*i+0] = twodee[2*10+2*i+0];
		partial2[2*i+1] = twodee[2*10+2*i+1];

		partial3[3*i+0] = threedee[3*10+3*i+0];
		partial3[3*i+1] = threedee[3*10+3*i+1];
		partial3[3*i+2] = threedee[3*10+3*i+2];
	}
	c->run(width, height, 9, partial2, partial3,*/
	c->run(width, height, FACEPOINTS, twodee, threedee,
	tempA, tempR, outT, inA,
	0, true, false, 0, inR, inT);

	transpose(tempA, 3, 3, outA);
	transpose(tempR, 3, 3, outR);
}

bool InitAll()
{
	cfg = new ConfigHelper(strXmlPath);

	if(cfg->IsInitialized())
	{
		strRefModelPath = strCurrentFolder + "ref_model.obj";

		r = new Renderer();
		c = new Calib();

		return true;
	}
	else
	{
		return false;
	}
}


void TurnFace(cv::Mat imgSrc, cv::Mat imgDst, double* unprojDstToRef, cv::Mat camRefToSrc)
{
	int srcWidth = imgSrc.cols;
	int srcHeight = imgSrc.rows;

	double xyzRef[4];
	double xyzSrc[4];

	xyzRef[3] = xyzSrc[3] = 1;

	cv::Mat vecRef(1, 4, CV_64FC1, xyzRef);
	cv::Mat vecSrc(1, 4, CV_64FC1, xyzSrc);

	for(int xDst=0; xDst < srcWidth; xDst++)
	{
		for(int yDst=0; yDst < srcHeight; yDst++)
		{
			int idx = (xDst * srcHeight + yDst);

			xyzRef[0] = unprojDstToRef[idx];
			xyzRef[1] = unprojDstToRef[srcHeight * srcWidth + idx];
			xyzRef[2] = unprojDstToRef[srcHeight * srcWidth * 2 + idx];
					
			if(xyzRef[0] != 0 || xyzRef[1] != 0 || xyzRef[2] != 0)
			{
				vecSrc = vecRef * camRefToSrc;

				if(xyzSrc[2] != 0)
				{
					int xSrc = (xyzSrc[0] / xyzSrc[2]);
					int ySrc = srcHeight - (xyzSrc[1] / xyzSrc[2]);

					if(xSrc >= 0 && xSrc < imgDst.cols && ySrc >= 0 && ySrc < imgDst.rows)
					{
						imgDst.at<cv::Vec3b>(yDst,xDst)[0] = imgSrc.at<cv::Vec3b>(ySrc,xSrc)[0];
						imgDst.at<cv::Vec3b>(yDst,xDst)[1] = imgSrc.at<cv::Vec3b>(ySrc,xSrc)[1];
						imgDst.at<cv::Vec3b>(yDst,xDst)[2] = imgSrc.at<cv::Vec3b>(ySrc,xSrc)[2];
					}
				}
			}
		}
	}
}

void GenerateRefData   (
						int widthWebcam,
						int heightWebcam,
						double *refAbyWebcam,
						double *refRbyWebcam,
						double *refTbyWebcam,
						double* threedeeRefByWebcam)
{
	// allocate memory
	double* depthRefByWebcam = new double[widthWebcam*heightWebcam];
	unsigned char *rgbRefByWebcam = new unsigned char[widthWebcam*heightWebcam*3];
	double *unprojectRefByWebcam = new double[widthWebcam*heightWebcam*3];

	// generate ref model image and unproj
	GenerateRefModelOutput(widthWebcam, heightWebcam, depthRefByWebcam, rgbRefByWebcam, unprojectRefByWebcam, refAbyWebcam, refRbyWebcam, refTbyWebcam);

	// convert renderer's output to opencv mat
	cv::Mat imgRefByWebcam = RgbToMat(rgbRefByWebcam, widthWebcam, heightWebcam);

	// detect ref model's face in image
	cv::Mat faceRefByWebcam;
	if(Detect(&faceRefByWebcam, imgRefByWebcam) != -1)
	{
		// get face points coordinates in 3d space by unproj
		GetThreedee(widthWebcam, heightWebcam, unprojectRefByWebcam, faceRefByWebcam, threedeeRefByWebcam);

		// clean memory
		delete depthRefByWebcam;
		delete rgbRefByWebcam;
		delete unprojectRefByWebcam;
		imgRefByWebcam.release();
		faceRefByWebcam.release();
	}
	else
	{
		throw "Could not detect ref model's face";
	}
}

void WriteArrayToFile(ofstream &myfile, double* arr, int length)
{
	if(length > 0)
	{
		myfile << arr[0];
		for(int i=1; i<length; i++)
		{
			myfile << " " << arr[i];
		}
	}
}

void WriteRefData(string path, double* A, double* R, double* T, double* threedee)
{
	ofstream myfile;
	myfile.open (path);
	
	myfile << "  <preprocessing enabled=\"N\">" << endl;
	myfile << "    <points>" << endl;

	for(int i=0; i<49; i++)
	{
		myfile << "      <point id=\"" << i+1 << "\">";
		WriteArrayToFile(myfile, threedee + (i*3), 3);
		myfile << "</point>" << endl;
	}

	myfile << "    </points>" << endl;
	myfile << "    <matrices>" << endl;

	myfile << "      <a>";
	WriteArrayToFile(myfile, A, 9);
	myfile << "</a>" << endl;

	myfile << "      <r>";
	WriteArrayToFile(myfile, R, 9);
	myfile << "</r>" << endl;

	myfile << "      <t>";
	WriteArrayToFile(myfile, T, 3);
	myfile << "</t>" << endl;

	myfile << "    </matrices>" << endl;
	myfile << "  </preprocessing>" << endl;


	myfile.close();
}

int main(int argc, const char* argv[])
{
	try
	{
		ParseArgs(argc, argv);

		cout << "base dir: " << strCurrentFolder << endl;
		cout << "XML: " << strXmlPath << endl;
	
		if(InitAll() &&
			InitDetect() == 0)
		{
			
			int widthWebcam;
			int heightWebcam;
			cv::Mat imgWebcam;

			// open input video (file/webcam)
			cv::VideoCapture cap;
			if(cfg->GetInputType() == "webcam")
			{
				useWebcam = true;
				cap.open(cfg->GetInputWebcam());
				if(!cap.isOpened())  
					return -1;

				cap >> imgWebcam;

				// busy wait until webcam start providing real images for resolution
				while(imgWebcam.cols == 0)
				{
					cap >> imgWebcam;
				}
			}
			else
			{
				cap.open(cfg->GetInputPath());
				if(!cap.isOpened())  
					return -1;
			}

			// input width
			widthWebcam = cap.get(CV_CAP_PROP_FRAME_WIDTH); //imgWebcam.cols;
			heightWebcam = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //imgWebcam.rows;

			cout << "Webcam Resolution: " << widthWebcam << " x " << heightWebcam << endl;

			// allocate memory for reference data
			double *refAbyWebcam = new double[9];
			double *refRbyWebcam = new double[9];
			double *refTbyWebcam = new double[3];

			double* depthRefByWebcam = new double[widthWebcam*heightWebcam];
			unsigned char *rgbRefByWebcam = new unsigned char[widthWebcam*heightWebcam*3];
			double *unprojectRefByWebcam = new double[widthWebcam*heightWebcam*3];
			double* threedeeRefByWebcam = new double[FACEPOINTS*3];
			
			// get reference data by either pre-processing from config or just generate it
			if(cfg->IsPreProcessing())
			{
				cfg->GetPreProcessingData(refAbyWebcam, refRbyWebcam, refTbyWebcam, threedeeRefByWebcam);
			}
			else
			{
				GenerateRefData(widthWebcam, heightWebcam, refAbyWebcam, refRbyWebcam, refTbyWebcam, threedeeRefByWebcam);
				
				// TODO - swap threedee yz param
				//for(int i=0; i<49; i++)
				//{
				//	swap(threedeeRefByWebcam[i*3+1], threedeeRefByWebcam[i*3+2]);
				//	threedeeRefByWebcam[i*3+1] *= -1;
				//	//threedeeRefByWebcam[i*3+2] -= 128;
				//}

				WriteRefData("ref.xml", refAbyWebcam, refRbyWebcam, refTbyWebcam, threedeeRefByWebcam);
				r = new Renderer();
			}

			double* twodeeWebcam = new double[FACEPOINTS*2];
			
			double *webcamA = new double[9];
			double *webcamR = new double[9];
			double *webcamT = new double[3];
			
			double *euler = new double[3];
			
			// start multi (models) mode
			r->StartMultiMode(widthWebcam, heightWebcam, !cfg->ShowRenderer(), false, refAbyWebcam, true);
			
			// init models
			vector<ModelInfo*>* vecModels = cfg->getModels();
			vector<ManagedModel*>* vecMMs = new vector<ManagedModel*>();
			for(int i=0; i<vecModels->size(); i++)
			{
				ManagedModel* mm = new ManagedModel(r, vecModels->at(i));
				vecMMs->push_back(mm);
			}

			// dummy frame to load and build up all models
			r->frame();
			
			int usedModels = 0;
			int totalModels = vecMMs->size();
			cv::Mat* arrFaces = new cv::Mat[totalModels];
			vector<int> vecAvailableModels;
			vector<int> vecUsedModels;

			// init available models and their animation
			for(int i=0; i<totalModels; i++)
			{
				vecMMs->at(i)->InitAnimation();
				vecAvailableModels.push_back(i);
			}

			// init output file
			string strOutput = cfg->GetOutputFile();
			cv::VideoWriter outFile;
			unsigned char* osgFrame;
			if(!strOutput.empty() || cfg->TunnelRenderer())
			{
				if(!strOutput.empty())
				{
					outFile.open(strOutput, -1 /*CV_FOURCC('X','2','6','4')*/, cap.get(CV_CAP_PROP_FPS), cv::Size(widthWebcam, heightWebcam), true);
				}
				osgFrame = new unsigned char[widthWebcam*heightWebcam*3];
				r->RegisterForImageOutput(osgFrame);
			}

			int maxConcurrent = cfg->MaxConcurrent();
			if(maxConcurrent < 0)
			{
				maxConcurrent = totalModels;
			}

			// loop over frames until ESC key pressed / EOF
			bool track = false;
			int key = 0;
			while(key != 27 && !r->IsDone())
			{
				cap >> imgWebcam;

				if(imgWebcam.empty())
				{
					key = 27; // ESC
				}
				else
				{
					if(key == '-')
					{
						if(maxConcurrent > 0)
						{
							maxConcurrent--;
							cout << "Decreased Max Concurrent to " << maxConcurrent << endl;
						}
					}
					else if(key == '+')
					{
						if(maxConcurrent < totalModels)
						{
							maxConcurrent++;
							cout << "Increased Max Concurrent to " << maxConcurrent << endl;
						}
					}

					if(useWebcam)
					{
						// for mirror effect
						cv::flip(imgWebcam, imgWebcam, 1);
					}
					
					// Track known faces
					if(!vecUsedModels.empty())
					{
						for(int i=vecUsedModels.size()-1; i>=0; i--)
						{
							int idx = vecUsedModels[i];

							cv::Mat faceWebcam;
							cv::Mat facePrev = arrFaces[idx];
							if(Track (&faceWebcam, facePrev, imgWebcam) == 0)
							{
								arrFaces[idx] = faceWebcam;
							}
							else
							{
								// Failed to track - remove to end of available models queue
								vecUsedModels.erase(vecUsedModels.begin() + i);
								vecAvailableModels.push_back(int(idx));
							}
						}
					}

					// If there are available models then detect
					if(!vecAvailableModels.empty() && vecUsedModels.size() < maxConcurrent)
					{
						vector<cv::Mat*>* vecDetected = MultiDetect(imgWebcam);

						for(int i=0; i<vecDetected->size(); i++)
						{
							if(!vecAvailableModels.empty() && vecUsedModels.size() < maxConcurrent)
							{
								bool exist = false;
								cv::Mat* faceDetected = vecDetected->at(i);

								// go over all already known face and skip if already known
								for(int j=0; j<vecUsedModels.size(); j++)
								{
									cv::Mat* faceUsed = &arrFaces[vecUsedModels.at(j)];
									
									// TODO generic instead of intraface
									if(WithinRange(faceUsed->at<float>(0, 0), faceUsed->at<float>(1, 0),
												   faceUsed->at<float>(0, 38), faceUsed->at<float>(1, 38),
												   faceDetected->at<float>(0,16), faceDetected->at<float>(1,16))
										||
										WithinRange(faceUsed->at<float>(0, 9), faceUsed->at<float>(1, 9),
												   faceUsed->at<float>(0, 42), faceUsed->at<float>(1, 42),
												   faceDetected->at<float>(0,16), faceDetected->at<float>(1,16)))
									{
										exist = true;
										break;
									}
								}
								
								// if not exist - grab an available model
								if(!exist)
								{
									int idx = vecAvailableModels.at(0);
									vecAvailableModels.erase(vecAvailableModels.begin());
									arrFaces[idx] = *faceDetected;
									vecUsedModels.push_back(int(idx));
								}
							}
						}
					}
					
					if(cfg->CopyBackground())
					{
						r->SetBackgroundImage(imgWebcam.data, widthWebcam, heightWebcam);
					}

					for(int i=0; i<vecAvailableModels.size(); i++)
					{
						vecMMs->at(vecAvailableModels.at(i))->Hide();
					}

					for(int i=0; i<vecUsedModels.size(); i++)
					{
						int idx = vecUsedModels.at(i);

						GetTwodee(arrFaces[idx], twodeeWebcam);

						DoCalib(widthWebcam, heightWebcam, twodeeWebcam, threedeeRefByWebcam, webcamA, webcamR, webcamT, refAbyWebcam);
						
						eulerAngles(webcamR, euler);
						//cout << euler[0] << tab << euler[1] << tab << euler[2] << tab << webcamT[0] <<  tab << webcamT[1] <<  tab << webcamT[2] << endl;
						//cout << euler[0] << tab << euler[1] << tab << euler[2] << endl;
						//cout << webcamT[0] <<  tab << webcamT[1] <<  tab << webcamT[2] << endl;

						vecMMs->at(idx)->Show(webcamA, webcamR, webcamT);
						//vecMMs->at(idx)->Show(euler[0], euler[1], euler[2],	-webcamT[0], -webcamT[2], -webcamT[1]);
						//vecMMs->at(idx)->Show(euler[0], euler[1], euler[2],	-webcamT[0], -webcamT[2], -webcamT[1]);
						//vecMMs->at(idx)->Show();
						vecMMs->at(idx)->Animate(twodeeWebcam, threedeeRefByWebcam, webcamA, webcamR, webcamT, widthWebcam, heightWebcam);

						PaintFace(imgWebcam, arrFaces[idx]);
						PaintOriginal(imgWebcam, threedeeRefByWebcam, webcamA, webcamR, webcamT, widthWebcam, heightWebcam);
					}
					
					if(cfg->ShowInput())
					{
						cv::imshow("Input", imgWebcam);
					}

					r->frame();
					//r->frame(0,0,0,0,0,0);
					//r->frame(webcamA, webcamR, webcamT);

					if(!strOutput.empty() || cfg->TunnelRenderer())
					{
						cv::Mat frame = RgbToMat(osgFrame, widthWebcam, heightWebcam);

						if(cfg->TunnelRenderer())
						{
							double resize = cfg->Resize();
							if(resize == 1)
							{
								cv::imshow("Renderer", frame);
							}
							else if(resize > 0)
							{
								cv::Mat rszTunnel;
								cv::resize(frame, rszTunnel, cvSize(widthWebcam*resize, heightWebcam*resize));
								cv::imshow("Renderer", rszTunnel);
							}
						}

						if(!strOutput.empty())
						{
							if(cfg->OutputCorner())
							{
								cv::Mat rszWebcam;
								cv::resize(imgWebcam, rszWebcam, cvSize(widthWebcam/4, heightWebcam/4));
								rszWebcam.copyTo(frame(cv::Rect((3*widthWebcam/4)-1, (3*heightWebcam/4)-1, widthWebcam/4, heightWebcam/4)));
							}
							outFile.write(frame);
						}
					}

					key = cv::waitKey(1);
				}
			}
			
			if(!strOutput.empty())
			{
				outFile.release();
			}
			r->stop();
		}
	}
	catch (char const *e)
	{
		cout << endl << "something went wrong:" << endl << e << endl;
		cout << '\a'; //beep
		getchar();
	}
	exit(0);
	return 0;
}
