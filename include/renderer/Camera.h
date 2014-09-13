// Camera.h: interface for the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERA_H__641EA604_B28A_48B1_AFF1_C2C17A794F22__INCLUDED_)
#define AFX_CAMERA_H__641EA604_B28A_48B1_AFF1_C2C17A794F22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include <math.h> 
#include <vector>
using namespace std ;



//#define NUM_OF_CHANNELS 3  
#define CAMERA_PI				3.1415926535897932384626433832795

// define the camera's viewing direction 
#define CAM_VIEW_MINUS_Z 0 
#define CAM_VIEW_PLUS_Z	1 



/**
 * \class CCamera
 *
 * \par requirements
 * Library dependencies : OpenCV, OpenGL\n
 * 
 * \date 2006-09-01
 *
 * \author Wonwoo Lee (e-mail: wlee@gist.ac.kr)
 * 
 * This camera class is only for non-commercial applications and researches. 
 * Modifying this class is free. But if you made Improvement, please let me
 * know it. I'll appreciate your contribution. 
 * Feel free to contact me, if you find any errors in this class. 
 * 
 */
  
class CCamera  
{

public :
	CCamera();
	virtual ~CCamera();



	bool LoadFile(int imgWidth, int imgHeight,	double intrinsic[9],double  extrinsic[12] ); // Reads camera parameters from a file.

	void ProjectPointToCamera(CvPoint3D64f vertex, CvPoint2D64f &projected) ; 
	void ProjectPointToCamera(CvPoint3D64f vertex, CvPoint3D64f &projected) ; // Project 3D points to a camera 
	void ProjectPointToCamera(CvPoint3D32f vertex, CvPoint2D32f &projected) ; // Project 3D points to a camera 
	void ProjectPointToCamera(CvPoint3D32f vertex, CvPoint3D32f &projected) ; // Project 3D points to a camera
	void ProjectPointToCamera(std::vector<CvPoint3D32f> vertices, std::vector<CvPoint3D32f> &projected) ;		// Project 3D points onto the image plane
	
	
	void GetIntrinsicMat(CvMat* intr)  ;	// return extrinsic params as CvMat type
	void GetExtrinsicMat(CvMat* ext)  ;		// return intrinsic params as CvMat type
	void GetProjectionMat(CvMat* proj) ;	// return projection matrix 

	void GetInvExtrinsicOGL(double inv[16]) ;	// return inverse of extrinsic parameters as OpenGL modelview matrix form (column major order)
	
	void GetTranslationVector(CvMat* t) ;		// Get translation vector of the extrinsic parameters as CvMat type
	void GetTranslationVector(double t[3]) ;	// Get translation vector of the extrinsic parameters 

	void		GetRotationMatrix(CvMat *rot) ;		// Get rotation matrix of the extrinsic parameters
	void		GetRotationMatrix(double rot[9]) ;
	CvScalar	GetRotationMatrixAsQuaternion() ;	// Return rotation matrix as quaternion form.

	CvScalar	GetPrincipalPoint() ;			// return the principal point 
	
	void GetViewingDirectionInWorldCoord(double viewDir[3]) ;	

	void SetIntrinsicParam(double intrinsic[9]) ;
	void SetExtrinsicParam(double extrinsic[12]) ;
	void SetImageSize(int w, int h) ;
	void SetNearFarPlane(float near, float far) ;
	void SetCamViewDir(int dir);


	void GetOpenGLViewportParam(int viewport[4]) ;
	void GetOpenGLPerspectiveParam(float& fovy, float& aspect, float& fnear, float& ffar) ;
	void GetOpenGLModelviewParam(double modelview[16]) ;


public :
	double	m_fIntrinsic[9]  ;	/**< Intrinsic parameters of the camera */
	double	m_fExtrinsic[12] ;	/**< Extrinsic parameters of the camera */
	double	m_fProjection[16] ;	/**< Projection matrix of the camera */
	double	m_fFocalLength ;	/**< focal length in pixels */
	double	m_fFov ;			/**< field of view in degree */

	int		m_iViewingDirection ;	/**< Initial viewing direction in local camera coordinate */
	
	CvPoint3D32f	m_vViewingFrustum[4] ;		/**< 4 vertices which forms viewing frustum (for drawing) */
	CvScalar		m_pViewingPlanes[4] ;		/**< 4 parameters of viewing planes of the camera in world coordinates */
												
	float m_fNearPlane ;	/**< Far plane of the camera */ 
	float m_fFarPlane ;		/**< Near plane of the camera */

	int m_iImageWidth ;		/**< Image width of this camera */
	int m_iImageHeight ;	/**< Image height of this camera */

/*
// obsolute functions 
	void FindInsideVertices(vector<CvPoint3D32f> &vertices, vector<int> &insideVertices) ;	// find the vertices that are inside the viewing frusutum
*/

};

#endif // !defined(AFX_CAMERA_H__641EA604_B28A_48B1_AFF1_C2C17A794F22__INCLUDED_)
