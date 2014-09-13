// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "renderer\Camera.h"
#include <opencv/cv.h>



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/**
 *
 * \return 
 *
 * Class constructor 
 */
CCamera::CCamera()
{
	m_fIntrinsic[0] = m_fIntrinsic[4] = m_fIntrinsic[7] = 1.f ;
	m_fIntrinsic[1] = m_fIntrinsic[2] = m_fIntrinsic[3] = 0.f ;
	m_fIntrinsic[5] = m_fIntrinsic[6] = m_fIntrinsic[8] = 0.f ;

	m_fExtrinsic[0] = m_fExtrinsic[5] = m_fExtrinsic[10] = 1.f ;
	m_fExtrinsic[1] = m_fExtrinsic[2] = m_fExtrinsic[3] = m_fExtrinsic[4] = 0.f ;
	m_fExtrinsic[6] = m_fExtrinsic[7] = m_fExtrinsic[8] = m_fExtrinsic[9] = m_fExtrinsic[11] = 0.f ;

	m_iViewingDirection = CAM_VIEW_PLUS_Z ;
//	m_iViewingDirection = CAM_VIEW_MINUS_Z ;
	
	// Set default values 
	m_fNearPlane = 0.01 ;
	m_fFarPlane =  1000;
//	m_fNearPlane = 0.001 ;
//	m_fFarPlane =  10000;
}

/**
 *
 * \return 
 *
 * Class destructor
 */
CCamera::~CCamera()
{

}



/**
*
* \param *filename 
* \return Returns true if the file loading is successful. Or return false. 
* 
* 
* LoadFile() reads the intrinsic and extrinsic parameters of a camera from a file 
*/
bool CCamera::LoadFile(int imgWidth, int imgHeight,	double intrinsic[9],double  extrinsic[12] ) 
{

	m_iImageWidth = imgWidth ;
	m_iImageHeight = imgHeight ;

	for(int q=0; q<9; q++)		m_fIntrinsic[q] = intrinsic[q] ;
	for(int q=0; q<12; q++) 		m_fExtrinsic[q] = extrinsic[q] ;
   


	// calculate projection matrix 

		CvMat *ART = cvCreateMat(3,4,CV_64F) ;
		CvMat *A = cvCreateMat(3,3,CV_64F) ;
		CvMat *RT = cvCreateMat(3,4,CV_64F) ;

		cvmSet(A, 0,0, double(m_fIntrinsic[0])) ;
		cvmSet(A, 0,1, double(m_fIntrinsic[1])) ;
		cvmSet(A, 0,2, double(m_fIntrinsic[2])) ;
		cvmSet(A, 1,0, double(m_fIntrinsic[3])) ;
		cvmSet(A, 1,1, double(m_fIntrinsic[4])) ;
		cvmSet(A, 1,2, double(m_fIntrinsic[5]));
		cvmSet(A, 2,0, double(m_fIntrinsic[6])) ;
		cvmSet(A, 2,1, double(m_fIntrinsic[7])) ;
		cvmSet(A, 2,2, double(m_fIntrinsic[8])) ;

		cvmSet(RT, 0,0, double(m_fExtrinsic[0])) ;
		cvmSet(RT, 0,1, double(m_fExtrinsic[1])) ;
		cvmSet(RT, 0,2, double(m_fExtrinsic[2])) ;
		cvmSet(RT, 0,3, double(m_fExtrinsic[3])) ;
		cvmSet(RT, 1,0, double(m_fExtrinsic[4])) ;
		cvmSet(RT, 1,1, double(m_fExtrinsic[5])) ;
		cvmSet(RT, 1,2, double(m_fExtrinsic[6])) ;
		cvmSet(RT, 1,3, double(m_fExtrinsic[7])) ;
		cvmSet(RT, 2,0, double(m_fExtrinsic[8])) ;
		cvmSet(RT, 2,1, double(m_fExtrinsic[9])) ;
		cvmSet(RT, 2,2, double(m_fExtrinsic[10])) ;
		cvmSet(RT, 2,3, double(m_fExtrinsic[11])) ;
	
		cvMatMul(A, RT, ART) ;
		
		m_fProjection[0]	= cvmGet(ART, 0,0) ;
		m_fProjection[1]	= cvmGet(ART, 0,1) ;
		m_fProjection[2]	= cvmGet(ART, 0,2) ;
		m_fProjection[3]	= cvmGet(ART, 0,3) ;
		
		m_fProjection[4]	= cvmGet(ART, 1,0) ;
		m_fProjection[5]	= cvmGet(ART, 1,1) ;
		m_fProjection[6]	= cvmGet(ART, 1,2) ;
		m_fProjection[7]	= cvmGet(ART, 1,3) ;
		
		m_fProjection[8]	= cvmGet(ART, 2,0) ;
		m_fProjection[9]	= cvmGet(ART, 2,1) ;
		m_fProjection[10]	= cvmGet(ART, 2,2) ;
		m_fProjection[11]	= cvmGet(ART, 2,3) ;

		cvReleaseMat(&A) ;
		cvReleaseMat(&RT) ;
		cvReleaseMat(&ART) ;
	
	m_fProjection[12]	= 0 ;
	m_fProjection[13]	= 0 ;
	m_fProjection[14]	= 0 ;
	m_fProjection[15]	= 1 ;

	//original
	m_fFocalLength = m_fIntrinsic[4] ;
	m_fFov = 2 * atan(m_iImageHeight/(2*m_fFocalLength)) * 180 / CAMERA_PI ;
//	m_fFov =abs(m_fFov );

//	double f=m_fIntrinsic[0]/m_iImageWidth;
//	double s= (m_fIntrinsic[4]*m_iImageWidth)/(2*m_fIntrinsic[0]*m_iImageHeight);
//	m_fFov = 2 * atan(s/f) * 180 / CAMERA_PI ;


	return true ;
}






void CCamera::ProjectPointToCamera(CvPoint3D64f vertex, CvPoint2D64f &projected) 
{
	CvPoint3D64f temp ;
	ProjectPointToCamera(vertex, temp) ;
	
	projected.x = temp.x ;
	projected.y = temp.y ;
}

void CCamera::ProjectPointToCamera(CvPoint3D64f vertex, CvPoint3D64f &projected) 
{
	
	CvMat *ART = cvCreateMat(3,4,CV_64F) ;
	CvMat *proj = cvCreateMat(3,1, CV_64F) ;
	CvMat *XYZ1 = cvCreateMat(4,1, CV_64F) ;



	CvMat *A = cvCreateMat(3,3,CV_64F) ;
	CvMat *RT = cvCreateMat(3,4,CV_64F) ;


	cvmSet(A, 0,0, double(m_fIntrinsic[0])) ;
	cvmSet(A, 0,1, double(m_fIntrinsic[1])) ;
	cvmSet(A, 0,2, double(m_fIntrinsic[2])) ;

	cvmSet(A, 1,0, double(m_fIntrinsic[3])) ;
	cvmSet(A, 1,1, double(m_fIntrinsic[4])) ;
	cvmSet(A, 1,2, double(m_fIntrinsic[5]));

	cvmSet(A, 2,0, double(m_fIntrinsic[6])) ;
	cvmSet(A, 2,1, double(m_fIntrinsic[7])) ;
	cvmSet(A, 2,2, double(m_fIntrinsic[8])) ;



	cvmSet(RT, 0,0, double(m_fExtrinsic[0])) ;
	cvmSet(RT, 0,1, double(m_fExtrinsic[1])) ;
	cvmSet(RT, 0,2, double(m_fExtrinsic[2])) ;
	cvmSet(RT, 0,3, double(m_fExtrinsic[3])) ;

	cvmSet(RT, 1,0, double(m_fExtrinsic[4])) ;
	cvmSet(RT, 1,1, double(m_fExtrinsic[5])) ;
	cvmSet(RT, 1,2, double(m_fExtrinsic[6])) ;
	cvmSet(RT, 1,3, double(m_fExtrinsic[7])) ;

	cvmSet(RT, 2,0, double(m_fExtrinsic[8])) ;
	cvmSet(RT, 2,1, double(m_fExtrinsic[9])) ;
	cvmSet(RT, 2,2, double(m_fExtrinsic[10])) ;
	cvmSet(RT, 2,3, double(m_fExtrinsic[11])) ;
	
	cvMatMul(A, RT, ART) ;

	CvPoint3D64f tempVertex ;
		
	cvmSet(XYZ1, 0,0, (double)vertex.x) ;
	cvmSet(XYZ1, 1,0, (double)vertex.y) ;
	cvmSet(XYZ1, 2,0, (double)vertex.z) ;
	cvmSet(XYZ1, 3,0, 1) ;

	cvMatMul(ART, XYZ1, proj) ;

	tempVertex.x = (double)cvmGet(proj,0,0) ;
	tempVertex.y = (double)cvmGet(proj,1,0) ;
	tempVertex.z = (double)cvmGet(proj,2,0) ;

	tempVertex.x /= tempVertex.z ;
	tempVertex.y /= tempVertex.z ;
	
	projected.x = tempVertex.x ;
	projected.y = tempVertex.y ;
	projected.z = tempVertex.z ;

	cvReleaseMat(&ART) ;
	cvReleaseMat(&proj) ;
	cvReleaseMat(&XYZ1) ;

	cvReleaseMat(&RT) ;
	cvReleaseMat(&A) ;

}




/**
 * \param vertex : The vertex in 3D
 * \param projected : The projected point with floating values 
 *
 * \returns void
 *
 * The ProjectPointToCamera() projects a 3D vertex onto an image plane with a camera's projection matrix. 
 * The projected point is calculated with sub-pixel accuracy. 
 */
void CCamera::ProjectPointToCamera(CvPoint3D32f vertex, CvPoint2D32f &projected) 
{
	CvPoint3D32f temp ;
	ProjectPointToCamera(vertex, temp) ;
	
	projected.x = temp.x ;
	projected.y = temp.y ;
}


/**
 * \param vertex : The vertex in 3D
 * \param projected : The projected point with floating values 
 *
 * \returns void
 *
 * The ProjectPointToCamera() projects a 3D vertex onto an image plane \n
 * with a camera's projection matrix. The projected point is calculated \n
 * with sub-pixel accuracy. The z value of projected corresponds to \n
 * the normalizing factor of the homogeneous coordinates. 
 */
void CCamera::ProjectPointToCamera(CvPoint3D32f vertex, CvPoint3D32f &projected) 
{
	
	CvMat *ART = cvCreateMat(3,4,CV_64F) ;
	CvMat *proj = cvCreateMat(3,1, CV_64F) ;
	CvMat *XYZ1 = cvCreateMat(4,1, CV_64F) ;



	CvMat *A = cvCreateMat(3,3,CV_64F) ;
	CvMat *RT = cvCreateMat(3,4,CV_64F) ;


	cvmSet(A, 0,0, double(m_fIntrinsic[0])) ;
	cvmSet(A, 0,1, double(m_fIntrinsic[1])) ;
	cvmSet(A, 0,2, double(m_fIntrinsic[2])) ;

	cvmSet(A, 1,0, double(m_fIntrinsic[3])) ;
	cvmSet(A, 1,1, double(m_fIntrinsic[4])) ;
	cvmSet(A, 1,2, double(m_fIntrinsic[5]));

	cvmSet(A, 2,0, double(m_fIntrinsic[6])) ;
	cvmSet(A, 2,1, double(m_fIntrinsic[7])) ;
	cvmSet(A, 2,2, double(m_fIntrinsic[8])) ;



	cvmSet(RT, 0,0, double(m_fExtrinsic[0])) ;
	cvmSet(RT, 0,1, double(m_fExtrinsic[1])) ;
	cvmSet(RT, 0,2, double(m_fExtrinsic[2])) ;
	cvmSet(RT, 0,3, double(m_fExtrinsic[3])) ;

	cvmSet(RT, 1,0, double(m_fExtrinsic[4])) ;
	cvmSet(RT, 1,1, double(m_fExtrinsic[5])) ;
	cvmSet(RT, 1,2, double(m_fExtrinsic[6])) ;
	cvmSet(RT, 1,3, double(m_fExtrinsic[7])) ;

	cvmSet(RT, 2,0, double(m_fExtrinsic[8])) ;
	cvmSet(RT, 2,1, double(m_fExtrinsic[9])) ;
	cvmSet(RT, 2,2, double(m_fExtrinsic[10])) ;
	cvmSet(RT, 2,3, double(m_fExtrinsic[11])) ;
	
	cvMatMul(A, RT, ART) ;

	CvPoint3D32f tempVertex ;
		
	cvmSet(XYZ1, 0,0, (double)vertex.x) ;
	cvmSet(XYZ1, 1,0, (double)vertex.y) ;
	cvmSet(XYZ1, 2,0, (double)vertex.z) ;
	cvmSet(XYZ1, 3,0, 1) ;

	cvMatMul(ART, XYZ1, proj) ;

	tempVertex.x = (double)cvmGet(proj,0,0) ;
	tempVertex.y = (double)cvmGet(proj,1,0) ;
	tempVertex.z = (double)cvmGet(proj,2,0) ;

	tempVertex.x /= tempVertex.z ;
	tempVertex.y /= tempVertex.z ;
	
	projected.x = tempVertex.x ;
	projected.y = tempVertex.y ;
	projected.z = tempVertex.z ;

	cvReleaseMat(&ART) ;
	cvReleaseMat(&proj) ;
	cvReleaseMat(&XYZ1) ;

	cvReleaseMat(&RT) ;
	cvReleaseMat(&A) ;
	


}


/**
 *
 * \param vertices : a set of 3D vertices stored in a vector 
 * \param &projected : a set of projected points 
 *
 * ProjectedPointToCamera() projects a set of 3D vertices 
 */
void CCamera::ProjectPointToCamera(std::vector<CvPoint3D32f> vertices, std::vector<CvPoint3D32f> &projected) 
{
	CvMat *ART = cvCreateMat(3,4,CV_64F) ;
	CvMat *XYZ1 = cvCreateMat(4,1, CV_64F) ;
	CvMat *proj = cvCreateMat(3,1, CV_64F) ;

	projected.clear() ;
	
	cvmSet(ART, 0,0, m_fProjection[0]) ;
	cvmSet(ART, 0,1, m_fProjection[1]) ;
	cvmSet(ART, 0,2, m_fProjection[2]) ;
	cvmSet(ART, 0,3, m_fProjection[3]) ;
	cvmSet(ART, 1,0, m_fProjection[4]) ;
	cvmSet(ART, 1,1, m_fProjection[5]) ;
	cvmSet(ART, 1,2, m_fProjection[6]) ;
	cvmSet(ART, 1,3, m_fProjection[7]) ;
	cvmSet(ART, 2,0, m_fProjection[8]) ;
	cvmSet(ART, 2,1, m_fProjection[9]) ;
	cvmSet(ART, 2,2, m_fProjection[10]) ;
	cvmSet(ART, 2,3, m_fProjection[11]) ;

	int size = vertices.size() ;
	for(int i=0; i<size; i++) 
	{
		CvPoint3D32f tempVertex ;

		cvmSet(XYZ1, 0,0, (double)vertices[i].x) ;
		cvmSet(XYZ1, 1,0, (double)vertices[i].y) ;
		cvmSet(XYZ1, 2,0, (double)vertices[i].z) ;
		cvmSet(XYZ1, 3,0, 1) ;

		cvMatMul(ART, XYZ1, proj) ;

		tempVertex.x = (double)cvmGet(proj,0,0) ;
		tempVertex.y = (double)cvmGet(proj,1,0) ;
		tempVertex.z = (double)cvmGet(proj,2,0) ;

		tempVertex.x /= tempVertex.z ;
		tempVertex.y /= tempVertex.z ;

		projected.push_back(tempVertex) ;
	}

	cvReleaseMat(&proj) ;
	cvReleaseMat(&XYZ1) ;
	cvReleaseMat(&ART) ;

}


/**
 *
 * \param ext : A 3x4 matrix of Extrinsic parameter 
 *
 * Return the extrinsic parameters of the camera as CvMat form
 */
void CCamera::GetExtrinsicMat(CvMat* ext) 
{
	
	cvmSet(ext, 0,0, m_fExtrinsic[0]) ;
	cvmSet(ext, 0,1, m_fExtrinsic[1]) ;
	cvmSet(ext, 0,2, m_fExtrinsic[2]) ;
	cvmSet(ext, 0,3, m_fExtrinsic[3]) ;
	cvmSet(ext, 1,0, m_fExtrinsic[4]) ;
	cvmSet(ext, 1,1, m_fExtrinsic[5]) ;
	cvmSet(ext, 1,2, m_fExtrinsic[6]) ;
	cvmSet(ext, 1,3, m_fExtrinsic[7]) ;
	cvmSet(ext, 2,0, m_fExtrinsic[8]) ;
	cvmSet(ext, 2,1, m_fExtrinsic[9]) ;
	cvmSet(ext, 2,2, m_fExtrinsic[10]) ;
	cvmSet(ext, 2,3, m_fExtrinsic[11]) ;
}


/**
 *
 * \param proj : 3x4 projection matrix 
 *
 * Return the projection matrix of the camera as CvMat
 */
void CCamera::GetProjectionMat(CvMat* proj)
{
	cvmSet(proj, 0,0, m_fProjection[0]) ;
	cvmSet(proj, 0,1, m_fProjection[1]) ;
	cvmSet(proj, 0,2, m_fProjection[2]) ;
	cvmSet(proj, 0,3, m_fProjection[3]) ;
	cvmSet(proj, 1,0, m_fProjection[4]) ;
	cvmSet(proj, 1,1, m_fProjection[5]) ;
	cvmSet(proj, 1,2, m_fProjection[6]) ;
	cvmSet(proj, 1,3, m_fProjection[7]) ;
	cvmSet(proj, 2,0, m_fProjection[8]) ;
	cvmSet(proj, 2,1, m_fProjection[9]) ;
	cvmSet(proj, 2,2, m_fProjection[10]) ;
	cvmSet(proj, 2,3, m_fProjection[11]) ;
}


/**
 *
 * \param intr : 3x3 intrinsic matrix
 *
 * Return the intrinsic parameter of the camera 
 */
void CCamera::GetIntrinsicMat(CvMat* intr) 
{
	
	cvmSet(intr, 0,0, m_fIntrinsic[0]) ;
	cvmSet(intr, 0,1, m_fIntrinsic[1]) ;
	cvmSet(intr, 0,2, m_fIntrinsic[2]) ;

	cvmSet(intr, 1,0, m_fIntrinsic[3]) ;
	cvmSet(intr, 1,1, m_fIntrinsic[4]) ;
	cvmSet(intr, 1,2, m_fIntrinsic[5]) ;

	cvmSet(intr, 2,0, m_fIntrinsic[6]) ;
	cvmSet(intr, 2,1, m_fIntrinsic[7]) ;
	cvmSet(intr, 2,2, m_fIntrinsic[8]) ;

}

/**
 *
 * \return The principal point of the camera
 *
 * Returns the principal point of the camera. 
 * The coordinates are stored to 0th and 1st value of the CvScalar
 */
CvScalar CCamera::GetPrincipalPoint() 
{
	CvScalar pp ;

	pp.val[0] = m_fIntrinsic[2] ;
	pp.val[1] = m_fIntrinsic[5] ;

	return pp ;
}





/**
 *
 * \param dir : The initial viewing direction of the camera 
 *
 * Set the camera's initial viewing direction. dir should be 
 * CAM_VIEW_MINUS_Z or CAM_VIEW_PLUS_Z
 */
void CCamera::SetCamViewDir(int dir)
{
	m_iViewingDirection = dir ;
}




/**
 *
 * \param inv[16] : matrix array 
 *
 * Return the inverse of the extrinsic matrix as OpenGL modelview matrix (Column major order)
 */
void CCamera::GetInvExtrinsicOGL(double inv[16]) 
{
		
	CvMat *RT = cvCreateMat(4,4, CV_64F) ;
	CvMat *InvRT = cvCreateMat(4,4, CV_64F) ;
	
	cvmSet(RT, 0,0, m_fExtrinsic[0]) ;
	cvmSet(RT, 0,1, m_fExtrinsic[1]) ;
	cvmSet(RT, 0,2, m_fExtrinsic[2]) ;
	cvmSet(RT, 0,3, m_fExtrinsic[3]) ;
	cvmSet(RT, 1,0, m_fExtrinsic[4]) ;
	cvmSet(RT, 1,1, m_fExtrinsic[5]) ;
	cvmSet(RT, 1,2, m_fExtrinsic[6]) ;
	cvmSet(RT, 1,3, m_fExtrinsic[7]) ;
	cvmSet(RT, 2,0, m_fExtrinsic[8]) ;
	cvmSet(RT, 2,1, m_fExtrinsic[9]) ;
	cvmSet(RT, 2,2, m_fExtrinsic[10]) ;
	cvmSet(RT, 2,3, m_fExtrinsic[11]) ;
	cvmSet(RT, 3,0, 0) ;
	cvmSet(RT, 3,1, 0) ;
	cvmSet(RT, 3,2, 0) ;
	cvmSet(RT, 3,3, 1) ;

	cvInvert(RT, InvRT) ;

	inv[0] = cvmGet(InvRT, 0,0) ; 
	inv[1] = cvmGet(InvRT, 1,0) ; 
	inv[2] = cvmGet(InvRT, 2,0) ; 
	inv[3] = cvmGet(InvRT, 3,0) ; 
	inv[4] = cvmGet(InvRT, 0,1) ; 
	inv[5] = cvmGet(InvRT, 1,1) ; 
	inv[6] = cvmGet(InvRT, 2,1) ; 
	inv[7] = cvmGet(InvRT, 3,1) ; 
	inv[8] = cvmGet(InvRT, 0,2) ; 
	inv[9] = cvmGet(InvRT, 1,2) ; 
	inv[10] = cvmGet(InvRT, 2,2) ; 
	inv[11] = cvmGet(InvRT, 3,2) ; 
	inv[12] = cvmGet(InvRT, 0,3) ; 
	inv[13] = cvmGet(InvRT, 1,3) ; 
	inv[14] = cvmGet(InvRT, 2,3) ; 
	inv[15] = cvmGet(InvRT, 3,3) ; 

	cvReleaseMat(&InvRT) ;
	cvReleaseMat(&RT) ;
}



/**
 *
 * \param intrinsic[9] : intrinsic parameters with row major order
 *
 * Set intrinsic parameters of the camera 
 */
void CCamera::SetIntrinsicParam(double intrinsic[9]) 
{
	for(int i=0; i<9; i++) 
		m_fIntrinsic[i] = intrinsic[9] ;

}



/**
 *
 * \param extrinsic[12] : Extrinsic parameters with row major order
 *
 * Set the extrinsic parameters of the camera 
 */
void CCamera::SetExtrinsicParam(double extrinsic[12]) 
{
	for(int i=0; i<12; i++) 
		m_fExtrinsic[i] = extrinsic[i] ;

}



/**
 *
 * \param *viewDir : viewing vector in the world coordinate system
 *
 * Calculates the camera's normalized viewing vector in the world coordinate system. 
 */
void CCamera:: GetViewingDirectionInWorldCoord(double viewDir[3])
{
	CvMat *zDir		= cvCreateMat(4,1, CV_64F) ;	// viewing direction in the camera coord
	CvMat *wDir		= cvCreateMat(4,1, CV_64F) ;	// viewing direction in the world coord
	CvMat *RT		= cvCreateMat(4,4, CV_64F) ;
	CvMat *InvRT	= cvCreateMat(4,4, CV_64F) ;
	

	cvmSet(zDir, 0,0, 0) ;
	cvmSet(zDir, 1,0, 0) ;

	if(m_iViewingDirection == CAM_VIEW_MINUS_Z) 
		cvmSet(zDir, 2,0, -1) ;
	else 
		cvmSet(zDir, 2,0, 1) ;

	cvmSet(zDir, 3,0, 1) ;

	// calculate Inverse of RT 
	cvmSet(RT, 0,0, m_fExtrinsic[0]) ;
	cvmSet(RT, 0,1, m_fExtrinsic[1]) ;
	cvmSet(RT, 0,2, m_fExtrinsic[2]) ;
	cvmSet(RT, 0,3, m_fExtrinsic[3]) ;
	cvmSet(RT, 1,0, m_fExtrinsic[4]) ;
	cvmSet(RT, 1,1, m_fExtrinsic[5]) ;
	cvmSet(RT, 1,2, m_fExtrinsic[6]) ;
	cvmSet(RT, 1,3, m_fExtrinsic[7]) ;
	cvmSet(RT, 2,0, m_fExtrinsic[8]) ;
	cvmSet(RT, 2,1, m_fExtrinsic[9]) ;
	cvmSet(RT, 2,2, m_fExtrinsic[10]) ;
	cvmSet(RT, 2,3, m_fExtrinsic[11]) ;
	cvmSet(RT, 3,0, 0) ;
	cvmSet(RT, 3,1, 0) ;
	cvmSet(RT, 3,2, 0) ;
	cvmSet(RT, 3,3, 1) ;

	cvInvert(RT, InvRT) ;

	cvMatMul(InvRT, zDir, wDir) ;

	viewDir[0] = cvmGet(wDir, 0,0) ;
	viewDir[1] = cvmGet(wDir, 1,0) ;
	viewDir[2] = cvmGet(wDir, 2,0) ;

	// normalize
	double norm = sqrt(viewDir[0]*viewDir[0] + viewDir[1]*viewDir[1] + viewDir[2]*viewDir[2]) ;
	viewDir[0] /= norm ;
	viewDir[1] /= norm ;
	viewDir[2] /= norm ;

	cvReleaseMat(&zDir) ;
	cvReleaseMat(&wDir) ;
	cvReleaseMat(&InvRT) ;
	cvReleaseMat(&RT) ;
}





/**
 *
 * \param t : trnaslation vector as CvMat type
 *
 * Retrieve the translation vector from extrinsic parameters
 */
void CCamera::GetTranslationVector(CvMat* t) 
{
	// assume t is a column vector
	cvmSet(t, 0,0, m_fExtrinsic[3]) ;
	cvmSet(t, 1,0, m_fExtrinsic[7]) ;
	cvmSet(t, 2,0, m_fExtrinsic[11]) ;

	return ;
}



/**
 *
 * \param t[3] : Trnaslation vector as an array
 *
 * Retrieve the translation vector from extrinsic parameters 
 */
void CCamera::GetTranslationVector(double t[3])
{
	t[0] = m_fExtrinsic[3] ;
	t[1] = m_fExtrinsic[7] ;
	t[2] = m_fExtrinsic[11] ;
}




/**
 *
 * \param *rot : Rotation matrix as CvMat type
 *
 * Retrieve the rotation matrix from extrinsic parameters 
 */
void CCamera::GetRotationMatrix(CvMat *rot) 
{
	cvmSet(rot, 0,0, m_fExtrinsic[0]) ;
	cvmSet(rot, 0,1, m_fExtrinsic[1]) ;
	cvmSet(rot, 0,2, m_fExtrinsic[2]) ;
	
	cvmSet(rot, 1,0, m_fExtrinsic[4]) ;
	cvmSet(rot, 1,1, m_fExtrinsic[5]) ;
	cvmSet(rot, 1,2, m_fExtrinsic[6]) ;

	cvmSet(rot, 2,0, m_fExtrinsic[8]) ;
	cvmSet(rot, 2,1, m_fExtrinsic[9]) ;
	cvmSet(rot, 2,2, m_fExtrinsic[10]) ;
}



/**
 *
 * \param *rot : Rotation matrix as an array 
 *
 * Retrieve the rotation matrix from extrinsic parameters 
 */
void CCamera::GetRotationMatrix(double rot[9]) 
{
	rot[0] = m_fExtrinsic[0] ;
	rot[1] = m_fExtrinsic[1] ;
	rot[2] = m_fExtrinsic[2] ;

	rot[3] = m_fExtrinsic[4] ;
	rot[4] = m_fExtrinsic[5] ;
	rot[5] = m_fExtrinsic[6] ;

	rot[6] = m_fExtrinsic[8] ;
	rot[7] = m_fExtrinsic[9] ;
	rot[8] = m_fExtrinsic[10] ;
}


/**
 *
 * \return Quternion corresponding to the rotation matrix
 *
 * Calculates a quaternion which corresponds to the rotation matrix of the camera. 
 */
CvScalar CCamera::GetRotationMatrixAsQuaternion() 
{

	CvScalar quat ;
	double S,X,Y,Z,W ;
    double trace = m_fExtrinsic[0] + m_fExtrinsic[5] + m_fExtrinsic[10] ;
	
	if(trace > 0) 
	{
		S = sqrt(trace) * 2 ;
		X = (m_fExtrinsic[9] - m_fExtrinsic[6]) / S ;
		Y = (m_fExtrinsic[2] - m_fExtrinsic[8]) / S ;
		Z = (m_fExtrinsic[4] - m_fExtrinsic[1]) / S ;
		W = 0.25 * S ;
	}
	else
	{
		if ( m_fExtrinsic[0] > m_fExtrinsic[5] && m_fExtrinsic[0] > m_fExtrinsic[10] )  
		{	
			// Column 0: 
			S  = sqrt( 1.0 + m_fExtrinsic[0] - m_fExtrinsic[5] - m_fExtrinsic[10] ) * 2;
			X = 0.25 * S;
			Y = (m_fExtrinsic[4] + m_fExtrinsic[1] ) / S;
			Z = (m_fExtrinsic[2] + m_fExtrinsic[8] ) / S;
			W = (m_fExtrinsic[9] - m_fExtrinsic[6] ) / S;
		} 
		else if ( m_fExtrinsic[5] > m_fExtrinsic[10] ) 
		{
			// Column 1: 
			S  = sqrt( 1.0 + m_fExtrinsic[5] - m_fExtrinsic[0] - m_fExtrinsic[10] ) * 2;
			X = (m_fExtrinsic[4] + m_fExtrinsic[1] ) / S;
			Y = 0.25 * S;
			Z = (m_fExtrinsic[9] + m_fExtrinsic[6] ) / S;
			W = (m_fExtrinsic[2] - m_fExtrinsic[8] ) / S;
		}
		else 
		{
			// Column 2:
			S  = sqrt( 1.0 + m_fExtrinsic[10] - m_fExtrinsic[0] - m_fExtrinsic[5] ) * 2;
			X = (m_fExtrinsic[2] + m_fExtrinsic[8] ) / S;
			Y = (m_fExtrinsic[9] + m_fExtrinsic[6] ) / S;
			Z = 0.25 * S;
			W = (m_fExtrinsic[4] - m_fExtrinsic[1] ) / S;
		}
	}

			
	quat.val[0] = X ;
	quat.val[1] = Y ;
	quat.val[2] = Z ;
	quat.val[3] = W ;

	return quat ; 
}





/**
 *
 * \param viewport[4] 
 *
 * Description here... ^^
 */
void CCamera:: GetOpenGLViewportParam(int viewport[4]) 
{

	
	CvScalar pp = GetPrincipalPoint() ;
	
	int startx = (int)(pp.val[0] - m_iImageWidth / 2) ;
	int starty = (int)(pp.val[1] - m_iImageHeight / 2) ;

	viewport[0] = startx ;
	viewport[1] = -starty ;
	viewport[2] = m_iImageWidth ;
	viewport[3] = m_iImageHeight ;

}

/**
 *
 * \param &fovy : Field of view in y direction 
 * \param &aspect : aspect ratio
 * \param near : Near plane  
 * \param far : Far plane 
 * GetOpenGLPerspectiveParam() gives the parameters for the OpenGL function 'gluPerspective()'  
 */

void CCamera:: GetOpenGLPerspectiveParam(float& fovy, float& aspect, float& fnear, float& ffar) 
{

	fovy = m_fFov ;
    
    float fx = m_fIntrinsic[0] ;
	float fy = m_fIntrinsic[4] ;

	aspect	= m_iImageWidth/(float)m_iImageHeight * (fy / fx) ;
	
	fnear	= m_fNearPlane ;
	
	ffar		= m_fFarPlane ;
	
}


/**
 *
 * \param near : Near plane
 * \param far : Far plane 
 *
 * Set the near and far parameters for the OpenGL function 'gluPerspective()'
 */
void CCamera::SetNearFarPlane(float fnear, float ffar) 
{

	if(fnear <= 0) 
		m_fNearPlane = 0.001 ;
	else 
		m_fNearPlane = fnear ; 



	if(ffar <= 0) 
		m_fFarPlane = 500 ;
	else 
		m_fFarPlane = ffar ;	

}



/**
 *
 * \param modelview[16] : OpenGL modelview parameter corresponding to the extrinsic parameter
 *
 * GetOpenGLModelviewParam() gives OpenGL modelview matrix for rendering a scene from the camera's viewpoint. 
 */
void CCamera:: GetOpenGLModelviewParam(double modelview[16]) 
{
	
	CvMat* mat = cvCreateMat(4,4, CV_64F) ;
	CvMat* result = cvCreateMat(4,4, CV_64F) ;
	
	cvSetIdentity(mat) ;

	GetExtrinsicMat(mat) ;
	
	if(m_iViewingDirection == CAM_VIEW_PLUS_Z)
	{
        CvMat* scale = cvCreateMat(4,4, CV_64F) ;
		cvSetIdentity(scale) ;
		cvmSet(scale, 1,1, -1.0) ;
		cvmSet(scale, 2,2, -1.0) ;

		cvMatMul(scale, mat, result) ;
		cvReleaseMat(&scale) ;
	} // if(m_iViewingDirection == CAM_VIEW_PLUS_Z)
	else 
	{
		cvCopy(mat, result) ;
	}

	modelview[0] = cvmGet(result, 0,0 ) ;
	modelview[1] = cvmGet(result, 1,0) ;
	modelview[2] = cvmGet(result, 2,0) ;
	modelview[3] = cvmGet(result, 3,0) ;

	modelview[4] = cvmGet(result, 0,1) ;
	modelview[5] = cvmGet(result, 1,1) ;
	modelview[6] = cvmGet(result, 2,1) ;
	modelview[7] = cvmGet(result, 3,1) ;

	modelview[8] = cvmGet(result, 0,2) ;
	modelview[9] = cvmGet(result, 1,2) ;
	modelview[10] = cvmGet(result, 2,2) ;
	modelview[11] = cvmGet(result, 3,2) ;

	modelview[12] = cvmGet(result, 0,3) ;
	modelview[13] = cvmGet(result, 1,3) ;
	modelview[14] = cvmGet(result, 2,3) ;
	modelview[15] = cvmGet(result, 3,3) ;
}


/**
 *
 * \param w : Camera image width 
 * \param h : Camera image height 
 *
 * Set the image size of the camera. 
 */
void CCamera::SetImageSize(int w, int h) 
{
	m_iImageWidth = w ;
	m_iImageHeight = h ;
    
} 
