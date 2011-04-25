// JLinkageLibRandomSamplerMex.cpp : mex-function interface implentation file

#include "updator.h"
#include "RandomSampler.h"
#include "JLinkage.h"
#include "PrimitiveFunctions.h"
#include "JlnkCluster.h"

namespace JlnkCluster {
	using namespace Updator;

	void PrintHelp(){

		printf("\n\n *** JLinkageLibClusterizeMex v.1.0 *** Part of the SamantHa Project");
		printf("\n     Author roberto.toldo@univr.it - Vips Lab - Department of Computer Science - University of Verona(Italy) ");
		printf("\n ***********************************************************************");
		printf("\n Usage: [Labels, PS] = JLnkClusterize(Points, Models, InlierThreshold, ModelType, *KDTreeRange = -1*, *ExistingClusters = []*)");
		printf("\n Input:");
		printf("\n        Points - Input dataset (Dimension x NumberOfPoints)");
		printf("\n        Models - Hypotesis generated by the JLinkageLibRandomSamplerMex(Dimension x NumberOfModels)");
		printf("\n        InlierThreshold - maximum inlier distance point-model ");
		printf("\n        ModelType - type of models extracted. Currently the model supported are: 0 - Planes 1 - 2dLines");
		printf("\n        KDTreeRange(facultative) - Select the number of neighboards to use with in the agglomerative clustering stage. ( if <= 0 all the points are used; n^2 complexity)");
		printf("\n        ExistingClusters(facultative) - Already existing clusters, Logical Matrix containing Pts X NCluster");
		printf("\n Output:");
		printf("\n        Labels - Belonging cluster for each point");
		printf("\n        PS - Preference set of resulting clusters");
		printf("\n");

	}


	// Function pointers
	std::vector<float>  *(*mGetFunction)(const std::vector<sPt *> &nDataPtXMss, const std::vector<unsigned int>  &nSelectedPts);
	float (*mDistanceFunction)(const std::vector<float>  &nModel, const std::vector<float>  &nDataPt);

	//// Output arguments
	// Arg 0, NPoints x NSample logical preference set matrix
	//bool *mPSMatrix;

	unsigned int run(
		//// OUTPUT
		std::vector<unsigned int>& Lables,
		std::vector<unsigned int>& LableCount,
		//// Input arguments
		// Arg 0, points
		std::vector<std::vector<float> *> *mDataPoints,
		// Arg 1, Models
		std::vector<std::vector<float> *> *mModels,
		// Arg 2, InliersThreshold
		float mInlierThreshold,
		// Arg 3, type of model: 0 - Planes 1 - 2dLines
		unsigned int mModelType,
		// ----- facultatives
		// Arg 4, Select the KNN number of neighboards that can be merged. ( = 0 if all the points are used; n^2 complexity)
		int mKDTreeRange,
		// Arg 5, Already existing clusters, Logical Matrix containing Pts X NCluster");
		std::vector<std::vector<unsigned int> > mExistingClusters
		) 
	{
		// arg2 : InlierThreshold
		if(mInlierThreshold <= 0.0f) {
			printf("Invalid InlierThreshold");
			return 0;
		}

		// arg3 : modelType;
		switch(mModelType){
		case MT_PLANE: 
			mGetFunction = GetFunction_Plane;
			mDistanceFunction = DistanceFunction_Plane;
			break;
		case MT_LINE: 
			mGetFunction = GetFunction_Line;
			mDistanceFunction = DistanceFunction_Line;
			break;
		case MT_VP:
			mGetFunction = GetFunction_VP;
			mDistanceFunction = DistanceFunction_VP;
			break;
		default: printf("Invalid model type"); return 0; break;
		}

		printf("Initializing Data... \n");
		// Compute the jLinkage Clusterization
		JLinkage mJLinkage(mDistanceFunction, mInlierThreshold, mModels->size(), true, ((*mDataPoints)[0])->size(), mKDTreeRange);

		std::vector<const sPtLnk *> mPointMap(mDataPoints->size());	

		std::list<sClLnk *> mClustersList;

		unsigned int counter = 0;
		InitializeWaitbar("Loading Models ");
		for(unsigned int nModelCount = 0; nModelCount < mModels->size(); nModelCount++){
			mJLinkage.AddModel(((*mModels)[nModelCount]));
			++counter;
			UpdateWaitbar((float)counter/(float)mModels->size());
		}
		CloseWaitbar();

		counter = 0;
		InitializeWaitbar("Loading Points ");
		for(std::vector<std::vector<float> *>::iterator iterPts = mDataPoints->begin(); iterPts != mDataPoints->end(); ++iterPts ){
			mPointMap[counter] = mJLinkage.AddPoint(*iterPts);
			++counter;
			UpdateWaitbar((float)counter/(float)mDataPoints->size());
		}
		CloseWaitbar();

		if(mExistingClusters.size() > 0){
			printf("\tLoading Existing Models \n");
			for(int i=0; i < (int)mExistingClusters.size(); ++i){
				if(!mJLinkage.ManualClusterMerge(mExistingClusters[i])) {
					printf("Invalid Existing cluster matrix");
					return 0;
				}
			}
		}

		InitializeWaitbar("J-Clusterizing ");
		mClustersList = mJLinkage.DoJLClusterization(UpdateWaitbar);
		CloseWaitbar();

		// Write output
		// 	plhs[0] = mxCreateDoubleMatrix(1,mDataPoints->size(), mxREAL);
		// 	double *mTempUintPointer = (double *)mxGetPr(plhs[0]);
		struct LabelStat {
			unsigned int cnt;
			unsigned int id;
			bool operator<(const LabelStat& rhs)
			{
				if(cnt!=rhs.cnt) return cnt>rhs.cnt;
				return id>rhs.id;
			}
		};
		std::vector<LabelStat> stats;

		unsigned int counterCl = 0;
		Lables.clear();
		Lables.resize(mDataPoints->size(),10);
		for(std::list<sClLnk *>::iterator iterCl = mClustersList.begin(); iterCl != mClustersList.end(); ++iterCl){
			unsigned int cnt=0;
			for(std::list<sPtLnk *>::iterator iterPt = (*iterCl)->mBelongingPts.begin(); iterPt != (*iterCl)->mBelongingPts.end(); ++iterPt){
				unsigned int counterPt = 0;
				for(std::vector<const sPtLnk *>::iterator iterPtIdx = mPointMap.begin(); iterPtIdx != mPointMap.end(); ++iterPtIdx){
					if((*iterPt) == (*iterPtIdx)){
						//mTempUintPointer[counterPt] = counterCl;
						Lables[counterPt] = counterCl;
						++cnt;
						break;
					}
					++counterPt;
				}
			}
			LabelStat s;
			s.cnt = cnt;
			s.id = counterCl;
			stats.push_back(s);
			++counterCl;
		}

		std::sort(stats.begin(),stats.end());
		std::vector<unsigned int> Idx(stats.size());
		LableCount.clear();
		for(int i=0; i<stats.size(); ++i) {
			Idx[ stats[i].id ] = i;
			LableCount.push_back(stats[i].cnt);
		}

		unsigned int lsize = Lables.size();
		for(int i=0; i<lsize; ++i) {
			unsigned int old = Lables[i];
			Lables[i] = Idx[old];
		}

		return stats.size();

	}

}