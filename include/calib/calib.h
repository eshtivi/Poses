#ifndef __CALIB__
#define __CALIB__

class Calib
{
public: void run(int width, int height, int numPoints, double *imgPointsIn, double *objPointsIn,
		double *AOutput, double *ROutput, double *TOutput, double *Ain, double max_mse,
		bool usePosit, bool onlyExtrinsic, int useExtrinsicGuess, double *Rin, double *Tin);

		void GetEuler(double* R, double* out);
};

#endif