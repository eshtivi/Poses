#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "ModelInfo.h"
#include "AnimInfo.h"

#import <msxml6.dll> rename_namespace(_T("MSXML"))

using namespace std;

class ConfigHelper
{
private:
	MSXML::IXMLDOMDocument2Ptr xmlDoc;
	MSXML::IXMLDOMNodePtr xmlPreProc;

	int mFacePoints;
	bool mOK;

	bool mIsPreProcessing;
	ModelInfo* mProcessingModel;

	int mMaxConcurrent;

	string mInputType;
	string mInputData;

	bool mCopyBackground;

	bool mShowRenderer;
	bool mShowInput;
	bool mTunnelRenderer;
	double mResize;
	
	string mOutputFile;
	bool mOutputCorner;

	void InitArrFromString(double* arr, int length, BSTR bstr)
	{
		wstringstream ss(bstr);
		for(int i=0; i<length; i++)
		{
			ss >> arr[i];
		}
	}

	bool GetBoolAtt(MSXML::IXMLDOMNodePtr xmlNode, _bstr_t attName, bool notFoundValue)
	{
		bool ret = notFoundValue;
		
		MSXML::IXMLDOMNodePtr xmlAtt = xmlNode->Getattributes()->getNamedItem(attName);
		
		if(xmlAtt)
		{
			ret = strncmp(xmlAtt->Gettext(), "Y", 1) == 0;
		}

		return ret;
	}

	double GetDoubleAtt(MSXML::IXMLDOMNodePtr xmlNode, _bstr_t attName, double notFoundValue)
	{
		double ret = notFoundValue;
		
		MSXML::IXMLDOMNodePtr xmlAtt = xmlNode->Getattributes()->getNamedItem(attName);
		
		if(xmlAtt)
		{
			ret = atof(xmlAtt->Gettext());
		}

		return ret;
	}

	int GetIntAtt(MSXML::IXMLDOMNodePtr xmlNode, _bstr_t attName, int notFoundValue)
	{
		int ret = notFoundValue;
		
		MSXML::IXMLDOMNodePtr xmlAtt = xmlNode->Getattributes()->getNamedItem(attName);
		
		if(xmlAtt)
		{
			ret = atoi(xmlAtt->Gettext());
		}

		return ret;
	}

public:
	ConfigHelper(const string filename)
	{
		mOK = false;

		HRESULT hr = CoInitialize(NULL); 
		if (SUCCEEDED(hr))
		{
			try
			{
				hr = xmlDoc.CreateInstance(__uuidof(MSXML::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

				if (FAILED(hr))
				{
					throw ("Failed to create instance of MSXML doc");
				}
				else if (xmlDoc->load(filename.c_str()) != VARIANT_TRUE)
				{
					throw ("Failed to load " + filename);
				}
				else
				{
					xmlDoc->setProperty("SelectionLanguage", "XPath");
					
					mFacePoints = atoi(xmlDoc->selectSingleNode("config/facepoints")->Getattributes()->getNamedItem("value")->Gettext());

					xmlPreProc = xmlDoc->selectSingleNode("/config/preprocessing");
					mIsPreProcessing = GetBoolAtt(xmlPreProc, "enabled", false);
					
					if(!mIsPreProcessing)
					{
						MSXML::IXMLDOMNodePtr xmlProc = xmlDoc->selectSingleNode("/config/processing/model");

						mProcessingModel = new ModelInfo(string(xmlProc->Getattributes()->getNamedItem("file")->Gettext()),
							GetBoolAtt(xmlProc,		"lighting",		false),
							GetBoolAtt(xmlProc,		"hideinterior",	false),
							GetBoolAtt(xmlProc,		"center",		false),
							GetDoubleAtt(xmlProc,	"pi_azimuth",	0),
							GetDoubleAtt(xmlProc,	"pi_elevation",	0),
							GetDoubleAtt(xmlProc,	"pi_yaw",		0),
							GetDoubleAtt(xmlProc,	"axisx",		0),
							GetDoubleAtt(xmlProc,	"axisy",		0),
							GetDoubleAtt(xmlProc,	"axisz",		0));
					}

					mMaxConcurrent = GetIntAtt(xmlDoc->selectSingleNode("config/models"), "max", -1);

					mInputType = string(xmlDoc->selectSingleNode("/config/input")->Getattributes()->getNamedItem("type")->Gettext());
					mInputData = string(xmlDoc->selectSingleNode("/config/input")->Gettext());

					MSXML::IXMLDOMNodePtr xmlOutput = xmlDoc->selectSingleNode("/config/output");
					mCopyBackground = GetBoolAtt(xmlOutput, "copybg", true);

					MSXML::IXMLDOMNodePtr xmlOutputScreen = xmlDoc->selectSingleNode("/config/output/screen");
					if(GetBoolAtt(xmlOutputScreen, "enabled", true))
					{
						mShowRenderer = GetBoolAtt(xmlOutputScreen, "renderer", true);
						mShowInput = GetBoolAtt(xmlOutputScreen, "input", true);
						mTunnelRenderer = GetBoolAtt(xmlOutputScreen, "tunnel", false);
						mResize = GetDoubleAtt(xmlOutputScreen, "resize", 1);
					}
					else
					{
						mShowInput = false;
						mShowRenderer = false;
						mTunnelRenderer = false;
					}
					
					MSXML::IXMLDOMNodePtr xmlOutputFile = xmlDoc->selectSingleNode("/config/output/file");
					if(GetBoolAtt(xmlOutputFile, "enabled", false))
					{
						mOutputFile = xmlOutputFile->Gettext();
						mOutputCorner = GetBoolAtt(xmlOutputFile, "corner", true);
					}
					else
					{
						mOutputFile = "";
						mOutputCorner = false;
					}

					mOK = true;
				}
			}
			catch (exception &e)
			{
				cout << "ERROR: " << e.what();
			}
			//CoUninitialize();
		}
	}

	bool IsInitialized()
	{
		return mOK;
	}

	bool IsPreProcessing()
	{
		return mIsPreProcessing;
	}

	ModelInfo* GetProcessingModel()
	{
		return mProcessingModel;
	}

	vector<ModelInfo*>* getModels()
	{
		vector<ModelInfo*>* vec = new vector<ModelInfo*>();

		MSXML::IXMLDOMNodeListPtr xmlModelList = xmlDoc->selectNodes("/config/models/model");
		MSXML::IXMLDOMNodePtr xmlCurModel;

		for(int i=0; i<xmlModelList->Getlength(); i++)
		{
			xmlCurModel = xmlModelList->Getitem(i);
			
			MSXML::IXMLDOMNodeListPtr xmlAnimList = xmlCurModel->selectNodes("anim");

			AnimInfo** arrAnim = NULL;
			int nAnimCount = xmlAnimList->Getlength();

			if(nAnimCount > 0)
			{
				arrAnim = new AnimInfo*[nAnimCount];
				for(int j=0; j<nAnimCount; j++)
				{
					MSXML::IXMLDOMNodePtr xmlCurAnim = xmlAnimList->Getitem(j);
					arrAnim[j] = new AnimInfo(
						string(xmlCurAnim->Getattributes()->getNamedItem("target")->Gettext()),
						GetBoolAtt(xmlCurAnim, "isbone", true),
						GetIntAtt(xmlCurAnim, "point", -1),
						GetIntAtt(xmlCurAnim, "anchor", -1),
						GetDoubleAtt(xmlCurAnim, "weightx", 1),
						GetDoubleAtt(xmlCurAnim, "weighty", 1),
						GetDoubleAtt(xmlCurAnim, "weightz", 1));
				}
			}


			ModelInfo* info = new ModelInfo(
				string(xmlCurModel->Getattributes()->getNamedItem("file")->Gettext()),
				GetBoolAtt(xmlCurModel,		"lighting",		false),
				GetBoolAtt(xmlCurModel,		"hideinterior",	false),
				GetBoolAtt(xmlCurModel,		"center",		false),
				GetDoubleAtt(xmlCurModel,	"pi_azimuth",	0),
				GetDoubleAtt(xmlCurModel,	"pi_elevation",	0),
				GetDoubleAtt(xmlCurModel,	"pi_yaw",		0),
				GetDoubleAtt(xmlCurModel,	"axisx",		0),
				GetDoubleAtt(xmlCurModel,	"axisy",		0),
				GetDoubleAtt(xmlCurModel,	"axisz",		0),
				nAnimCount, arrAnim);

			vec->push_back(info);
		}

		return vec;
	}

	int MaxConcurrent()
	{
		return mMaxConcurrent;
	}

	string GetInputType()
	{
		return mInputType;
	}

	int GetInputWebcam()
	{
		return atoi(mInputData.c_str());
	}

	string GetInputPath()
	{
		return mInputData;
	}

	bool ShowRenderer()
	{
		return mShowRenderer;
	}

	bool ShowInput()
	{
		return mShowInput;
	}

	bool TunnelRenderer()
	{
		return mTunnelRenderer;
	}

	double Resize()
	{
		return mResize;
	}

	bool CopyBackground()
	{
		return mCopyBackground;
	}

	string GetOutputFile()
	{
		return mOutputFile;
	}

	bool OutputCorner()
	{
		return mOutputCorner;
	}

	void GetPreProcessingData(double *A, double *R, double *T, double* threedee)
	{
		 MSXML::IXMLDOMNodePtr mat = xmlPreProc->selectSingleNode("//matrices");

		 InitArrFromString(A, 9, mat->selectSingleNode("//a")->Gettext());
		 InitArrFromString(R, 9, mat->selectSingleNode("//r")->Gettext());
		 InitArrFromString(T, 3, mat->selectSingleNode("//t")->Gettext());

		 MSXML::IXMLDOMNodePtr points = xmlPreProc->selectSingleNode("//points");

		 double triple[3];
		 for(int i=0; i<mFacePoints; i++)
		 {
			 InitArrFromString(triple, 3, points->selectSingleNode(_bstr_t("//point[@id='") + _bstr_t(_variant_t(i+1)) + _bstr_t("']"))->Gettext());
			 threedee[i*3] = triple[0];
			 threedee[i*3+1] = triple[1];
			 threedee[i*3+2] = triple[2];
		 }
	}
};