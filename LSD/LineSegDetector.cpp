#include "LineSegDetector.h"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

	#include "lsd.h"

#ifdef __cplusplus
}
#endif

namespace Global {
	ntuple_list out=0;
	//parameters
	double scale;
	double sigma_scale;
	double quant;
	double ang_th;
	double eps;
	double density_th;
	int n_bins;
	double max_grad;

	struct AutoServer {
		AutoServer() {
			out = 0;
			scale= 0.8,
			sigma_scale= 0.6,
			quant= 2.0,
			ang_th= 22.5,
			eps = 0.0,
			density_th= 0.7,
			n_bins = 1024,
			max_grad= 255.0;
		}
		~AutoServer() {
			//free out if exist
			if( Global::out ) {
				free_ntuple_list(Global::out);
				Global::out = 0;
			}
		}
	}_autoServer;

}

LineSegDetectorI::LineSegDetectorI(double scale/*= 0.8*/,
	double sigma_scale/*= 0.6*/,
	double quant/*= 2.0*/,
	double ang_th/*= 22.5*/,
	double eps /*= 0.0*/,
	double density_th/*= 0.7*/,
	int n_bins /*= 1024*/,
	double max_grad/*= 255.0*/)
{
	Global::ang_th = ang_th;
	Global::density_th = density_th;
	Global::eps = eps;
	Global::max_grad = max_grad;
	Global::n_bins = n_bins;
	Global::quant = quant;
	Global::scale = scale;
	Global::sigma_scale = sigma_scale;
}

void LineSegDetectorI::RunOn( double* image, unsigned int w, unsigned int h )
{
	struct image_double_s ids = { image, w, h };
	
	//free last out if exist
	if( Global::out ) {
		free_ntuple_list(Global::out);
		Global::out = 0;
	}

	Global::out = LineSegmentDetection(&ids,
		Global::scale,
		Global::sigma_scale,
		Global::quant,
		Global::ang_th,
		Global::eps,
		Global::density_th,
		Global::n_bins,
		Global::max_grad,
		0);
}

unsigned int LineSegDetectorI::GetLineSegNum()
{
	return Global::out->size;
}

void LineSegDetectorI::GetLineSeg(int i, double& xs, double& ys,
	double& xe, double& ye, double& width )
{
	if(!Global::out || Global::out->dim!=5) {
		std::cout<<"Error:No valid Global::out!"<<std::endl;
		return;
	}

	if(i<0 || i>=Global::out->size) {
		std::cout<<"Error:Invalid Index i!"<<std::endl;
		return;
	}

	xs = Global::out->values[i*Global::out->dim + 0];
	ys = Global::out->values[i*Global::out->dim + 1];
	xe = Global::out->values[i*Global::out->dim + 2];
	ye = Global::out->values[i*Global::out->dim + 3];
	width = Global::out->values[i*Global::out->dim + 4];
}

void LineSegDetectorI::Release()
{
	if( Global::out ) {
		free_ntuple_list(Global::out);
		Global::out = 0;
	}
}