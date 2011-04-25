#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include "updator.h"
#include "JlnkCluster.h"
#include "JlnkSample.h"

std::vector< std::vector<float> *> pts;

inline void Read_Line(std::istream& in)
{
	while(in) {
		std::string str;
		std::getline(in,str,'\n');
		if(str.length()==0) continue;

		double x0,y0,x1,y1;
		sscanf(str.c_str(), 
			"%lf%*[^0-9-+.eE]%lf%*[^0-9-+.eE]%lf%*[^0-9-+.eE]%lf", &x0,&y0,&x1,&y1);
		double dx=x0-x1,dy=y0-y1;
		if( dx*dx+dy*dy<400 )
			continue;

		std::vector<float>* p = new std::vector<float>(4);
		pts.push_back(p);
		(*p)[0]=(float)x0;
		(*p)[1]=(float)y0;
		(*p)[2]=(float)x1;
		(*p)[3]=(float)y1;
	}
	std::cout<<"Read Line Done!"<<std::endl;
}

// exe infilename MT M I SigmaExp outputfilename
// MT model type 0 for plane, 1 for line, 2 for vanishing point
// M number of MSS
// I inlier threshold
// SigmaExp
int main(int argc, const char* argv[])
{
	if(argc<7) {
		std::cout<<argv[0]<<" infilename Modeltype Mss InlierThreshold SigmaExp outfilename"<<std::endl;
		return 1;
	}

	int MT=atoi(argv[2]);
	int M =atoi(argv[3]);
	double I=atof(argv[4]);
	double SigmaExp=atof(argv[5]);

	std::ifstream ifile(argv[1]);
	Read_Line(ifile);

	std::vector<unsigned int> Lables;
	std::vector<unsigned int> LableCount;

	std::vector<std::vector<float> *> *mModels = 
		JlnkSample::run(&pts, M, MT, 0, 3/*, SigmaExp*/);
	unsigned int num = JlnkCluster::run(Lables, LableCount, &pts, mModels, I, MT);

	std::ofstream ofile(argv[6]);
	unsigned int len = (unsigned int)Lables.size();
	for(unsigned int i=0; i<len; ++i) {
		ofile<<(*pts[i])[0]<<" "<<(*pts[i])[1]<<" "
			<<(*pts[i])[2]<<" "<<(*pts[i])[3]<<" "<<Lables.at(i)<<std::endl;
	}
	ofile.close();

	for(unsigned int i=0; i<pts.size(); i++)
		delete pts[i];

	for(unsigned int i=0; i < mModels->size(); i++)
		delete (*mModels)[i];
	delete mModels;

	return 0;
}
