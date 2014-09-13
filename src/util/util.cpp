#include "util\util.h"

ostringstream sout(ostringstream::out);


ostream& tab(ostream& output)
{
	return output << '\t';
}

void printMatrix(double *in, int m, int n)
{
	for (int i = 0;i < m;i++) {
		for (int j = 0;j < n;j++) {
			sout << in[i*n+j] << tab;
//			sout << in[i*n+j] << ",";
		}

		sout << ";" << endl;
	}
	sout <<tab<<"as array: ";

	for (int i = 0;i < m*n;i++) {
		sout<<in[i]<<",";
	}
	sout<<endl;
}

void printMatrix(float *in, int m, int n)
{
	for (int i = 0;i < m;i++) {
		for (int j = 0;j < n;j++) {
			sout << in[i*n+j] << tab;
//			sout << in[i*n+j] << ",";
		}

		sout << ";" << endl;
	}
	sout <<tab<<"as array: ";

	for (int i = 0;i < m*n;i++) {
		sout<<in[i]<<",";
	}
	sout<<endl;
}


//useful for converting from OpenGL images to Matlab matrices because
//OpenGL images are bottom to top. Plus, Matlab matrices are transposed
void transposeAndFlipY(double *in, int m, int n, double *out)
{
	for (int i = 0;i < n;i++) {
		for (int j = 0;j < m;j++) {
			out[i*m+j] = in[(m-1-j)*n+i];
		}
	}
}


void transpose(double *in, int m, int n, double *out)
{
	for (int i = 0;i < n;i++) {
		for (int j = 0;j < m;j++) {
			out[i*m+j] = in[j*n+i];
		}
	}

//	for (int i = 0;i < n*m;i++) {
//		sout << out[i] << tab;
//	}
//	sout << endl;
}



void transpose3dim(unsigned char *image, int gWidth , int gHeight,unsigned char *imageOutput) {
	if (NULL == imageOutput) {
		return;
	}

	for (int i = 0;i < gHeight;i++) {
		for (int j = 0;j < gWidth;j++) {
			imageOutput[(i*gWidth+j)*3] = (unsigned char)image[j*gHeight+i];
			imageOutput[(i*gWidth+j)*3+1] = (unsigned char)image[gHeight*gWidth+j*gHeight+i];
			imageOutput[(i*gWidth+j)*3+2] = (unsigned char)image[2*gHeight*gWidth+j*gHeight+i];
		}
	}
}

//transpose and convert RGB to BGR
void transpose3dimBGR(unsigned char *image, int gWidth , int gHeight,unsigned char *imageOutput) {
	if (NULL == imageOutput) {
		return;
	}

	for (int i = 0;i < gHeight;i++) {
		for (int j = 0;j < gWidth;j++) {
			imageOutput[(i*gWidth+j)*3+2] = (unsigned char)image[j*gHeight+i];
			imageOutput[(i*gWidth+j)*3+1] = (unsigned char)image[gHeight*gWidth+j*gHeight+i];
			imageOutput[(i*gWidth+j)*3] = (unsigned char)image[2*gHeight*gWidth+j*gHeight+i];
		}
	}
}

//combines http://opencv.willowgarage.com/wiki/Posit and http://old.uvr.gist.ac.kr/wlee/web/techReports/ar/Camera%20Models.html
void getOpenGLMatrices(double *A, double *R, double *T, int width, int height, double mv[16],
		double projectionMatrix[16]) {

	const double nearPlane = 0.01;
	const double farPlane = 1000;

	double fx = A[0 * TRIPLET + 0];
	double fy = A[1 * TRIPLET + 1];
	double px = A[0 * TRIPLET + 2];
	double py = A[1 * TRIPLET + 2];
	projectionMatrix[0] = 2.0 * fx / width;
	projectionMatrix[1] = 0.0;
	projectionMatrix[2] = 0.0;
	projectionMatrix[3] = 0.0;

	projectionMatrix[4] = 0.0;
	projectionMatrix[5] = 2.0 * fy / height;
	projectionMatrix[6] = 0.0;
	projectionMatrix[7] = 0.0;

	projectionMatrix[8] = 2.0 * (px / width) - 1.0;
	projectionMatrix[9] = 2.0 * (py / height) - 1.0;
	projectionMatrix[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	projectionMatrix[11] = -1.0;

	projectionMatrix[12] = 0.0;
	projectionMatrix[13] = 0.0;
	projectionMatrix[14] = -2.0 * farPlane * nearPlane / (farPlane - nearPlane);
	projectionMatrix[15] = 0.0;

	double correctR[9];
	for (int i = 0; i < 9; i++) {
		correctR[i] = R[i];
	}

	//OpenGL's Y and Z axis are opposite to the camera model (OpenCV)
	//same as RRz(180)*RRy(180)*R:
	//  1.0000    0.0000    0.0000
	//  0.0000   -1.0000    0.0000		*		R
	//  0.0000         0   -1.0000
	for (int i = 1; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			correctR[i * TRIPLET + j] *= -1;
		}
	}

	for (int f = 0; f < 3; f++) {
		for (int c = 0; c < 3; c++) {
			mv[c * 4 + f] = correctR[f * 3 + c]; //transposed
		}
	}
	mv[3] = 0.0;
	mv[7] = 0.0;
	mv[11] = 0.0;
	mv[12] = T[0];
	// also invert Y and Z of translation
	mv[13] = -T[1];
	mv[14] = -T[2];
	mv[15] = 1.0;

}

void getCameraMatricesFromOpenGL(double *A, double *R, double *T, int width, int height,
		double mv[16], double projectionMatrix[16]) {
	for (unsigned int i = 0; i < TRIPLET * TRIPLET; i++) {
		A[i] = 0;
	}

	A[0] = projectionMatrix[0] * width / 2.0;
	A[4] = projectionMatrix[5] * height / 2.0;
	A[2] = 1 * width / 2.0;
	A[5] = 1 * height / 2.0;
	A[8] = 1.;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			R[i * 3 + j] = mv[j * 4 + i];	//	transposed
		}
	}

	//OpenGL's Y and Z axis are opposite to the camera model (OpenCV)
	for (int i = 1; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			R[i * TRIPLET + j] *= -1;
		}
	}


	for (int i = 0; i < 3; i++) {
			T[i] = mv[3 * 4 + i];
	}
	// also invert Y and Z of translation
	T[1]*=-1;
	T[2]*=-1;
}


bool closeEnough(const double& a, const double& b, const double& epsilon) {
    return (epsilon > std::abs(a - b));
}

// http://stackoverflow.com/questions/18433801/converting-a-3x3-matrix-to-euler-tait-bryan-angles-pitch-yaw-roll

void eulerAngles(double* Rin, double* out)
{
	const double PI = 3.14159265358979323846264f;

	double R[3][3];
	//memcpy(R, Rin, sizeof(double)*9);
	transpose(Rin, 3, 3, &R[0][0]);

    //check for gimbal lock
    if (closeEnough(R[0][2], -1.0f)) 
	{
        double x = 0; //gimbal lock, value of x doesn't matter
        double y = PI / 2;
        double z = x + atan2(R[1][0], R[2][0]);
        //return { x, y, z };
		out[0] = x;
		out[1] = y;
		out[2] = z;
    }
	else if (closeEnough(R[0][2], 1.0f))
	{
        double x = 0;
        double y = -PI / 2;
        double z = -x + atan2(-R[1][0], -R[2][0]);
        //return { x, y, z };
		out[0] = x;
		out[1] = y;
		out[2] = z;
    }
	else
	{
		//two solutions exist
        double x1 = -asin(R[0][2]);
        double x2 = PI - x1;

        double y1 = atan2(R[1][2] / cos(x1), R[2][2] / cos(x1));
        double y2 = atan2(R[1][2] / cos(x2), R[2][2] / cos(x2));

        double z2 = atan2(R[0][1] / cos(x2), R[0][0] / cos(x2));
        double z1 = atan2(R[0][1] / cos(x1), R[0][0] / cos(x1));

        //choose one solution to return
        //for example the "shortest" rotation
        if ((std::abs(x1) + std::abs(y1) + std::abs(z1)) <= (std::abs(x2) + std::abs(y2) + std::abs(z2))) {
            //return { x1, y1, z1 };
			out[0] = x1;
			out[1] = y1;
			out[2] = z1;
        }
		else
		{
            //return { x2, y2, z2 };
			out[0] = x2;
			out[1] = y2;
			out[2] = z2;
        }
    }
	
	out[0] = PI-out[0];
	out[1] = (PI/2)-out[1];
	out[2] = -out[2];

	/*out[0] *= -1;
	out[1] *= -1;
	out[2] *= -1;*/
}