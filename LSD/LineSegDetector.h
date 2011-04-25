#include <vector>

class LineSegDetectorI {
public:
	LineSegDetectorI(double scale= 0.8,
		double sigma_scale= 0.6,
		double quant= 2.0,
		double ang_th= 22.5,
		double eps = 0.0,
		double density_th= 0.7,
		int n_bins = 1024,
		double max_grad= 255.0);

	//memory ownership of image left to caller
	void RunOn(double* image, unsigned int w, unsigned int h);

	unsigned int GetLineSegNum();

	void GetLineSeg(int i, double& xs, double& ys, double& xe, double& ye, double& width);

	void Release();
};