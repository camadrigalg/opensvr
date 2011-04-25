#pragma once

#include "geos/index/quadtree/Quadtree.h"
#include "geos.h"
#include <vector>

class GlobalData
{
public:
	struct UserData {
		COLORREF c;
		int w;//width
		unsigned int l;//class label
	};

	typedef std::vector<geos::geom::Point*> PointArr;
	typedef std::vector<geos::geom::LineString*> LineSegmentArr;

	static GlobalData& Instance();
	~GlobalData();

	inline bool IsLocked() { return _lock; }
	inline void Lock() { _lock = true; }
	inline void UnLock() { _lock = false; }

	inline PointArr& GetSignPts() { return signPts; }
	const geos::geom::Point* NewSign(double x, double y, COLORREF c=RGB(0,255,0));
	bool DeleteLastSign();
	bool SaveSignPts(const char* path);

	inline LineSegmentArr& GetLineSegments() { return lss; }
	bool NewLineSeg(double sx, double sy, double ex, double ey, COLORREF c=RGB(255,255,255), unsigned int l=-1);
	bool DeleteLastLineSeg();
	bool SaveLineSeg(const char* path);

	inline LineSegmentArr& GetDrawSegments() { return dss; }
	bool NewDrawSeg(double sx, double sy, double ex, double ey, COLORREF c=RGB(0,255,0));
	bool DeleteLastDrawSeg();

	bool Polygonize();

	unsigned int classNum;

	const geos::geom::Geometry* SearchGeometry(double cx, double cy,
		double w, double h);

	std::vector<std::vector<double> > vps;

	double W;
	double H;
	double f;

protected:
	GlobalData();
	PointArr signPts;
	LineSegmentArr lss;
	LineSegmentArr dss;
	bool _lock;

	geos::index::quadtree::Quadtree tree;
	geos::geom::Geometry* lastGeom;
	std::vector<geos::geom::Polygon*>* polygons;
};