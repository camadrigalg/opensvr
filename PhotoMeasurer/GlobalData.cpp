#include "stdafx.h"

#include "GlobalData.h"
#include "geos/operation/polygonize/Polygonizer.h"
#include "../Console/Console.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

using namespace geos::geom;

namespace G {
	GeometryFactory* factory=NULL;
	inline void destroy(Geometry* g) {
		GlobalData::UserData* ud = (GlobalData::UserData*)g->getUserData();
		if(ud) delete ud;
		factory->destroyGeometry(g);
	}
}

///////////////////////////////////////////////////////
GlobalData::GlobalData()
{
	Console::info()<<"[CxImage] version 6.0.0.0"<<Console::endl;
	Console::info()<<"[GEOS] version "<<GEOS_VERSION<<Console::endl;
	PrecisionModel pm(PrecisionModel::FLOATING_SINGLE);
	G::factory = new GeometryFactory(&pm);
	classNum = 0;
	lastGeom = 0;

	UnLock();
}

GlobalData::~GlobalData()
{
	while(DeleteLastSign());
	while(DeleteLastLineSeg());
	while(DeleteLastDrawSeg());
	if(G::factory) delete G::factory;
	G::factory = NULL;
}

GlobalData& GlobalData::Instance()
{
	static GlobalData sGlobalData;
	return sGlobalData;
}

const geos::geom::Point* GlobalData::NewSign( double x, double y, COLORREF c )
{
	Point* p = G::factory->createPoint( Coordinate(x,y) );
	this->signPts.push_back( p );
	tree.insert( p->getEnvelopeInternal(), p );
	UserData* ud = new UserData;
	ud->c = c;
	ud->w = 1;
	p->setUserData(ud);
	return p;
}

bool GlobalData::DeleteLastSign()
{
	int num = signPts.size();
	if(num<=0) return false;
	Point* p = signPts.back();
	if(lastGeom==p) lastGeom = 0;
	tree.remove( p->getEnvelopeInternal(), p );
	G::destroy(p);
	signPts.pop_back();
	return true;
}

bool GlobalData::SaveSignPts( const char* path )
{
	std::ofstream out(path);
	out << std::setiosflags(std::ios::scientific);

	int num = signPts.size();
	for(int i=0; i<num; ++i) {
		out << signPts.at(i)->getX() << "\t" << signPts.at(i)->getY() << std::endl;
	}

	return true;
}

bool GlobalData::NewLineSeg( double sx, double sy, double ex, double ey, COLORREF c, unsigned int l )
{
	LineSegment ls(sx,sy,ex,ey);
	LineString* lstr = ls.toGeometry(*G::factory).release();
	UserData* ud = new UserData;
	ud->c = c;
	ud->w = 1;
	ud->l = l;
	lstr->setUserData(ud);
	lss.push_back( lstr );
	tree.insert( lstr->getEnvelopeInternal(), lstr );
	return true;
}

bool GlobalData::DeleteLastLineSeg()
{
	int num = lss.size();
	if(num<=0) return false;
	LineString* lstr = lss.back();
	if(lastGeom==lstr) lastGeom = 0;
	tree.remove( lstr->getEnvelopeInternal(), lstr );
	G::destroy(lstr);
	lss.pop_back();
	return true;
}

bool GlobalData::SaveLineSeg( const char* path )
{
	std::ofstream out(path);

	int num = lss.size();
	for(int i=0; i<num; ++i) {
		LineString* lstr = lss[i];
		if(lstr->getNumPoints()!=2) continue;
		const Coordinate& c0 = lstr->getCoordinateN(0),
			&c1 = lstr->getCoordinateN(1);
		out<<c0.x<<" "<<c0.y<<" "
			<<c1.x<<" "<<c1.y;
		if(lstr->getUserData()) {
			out<<" "<<((UserData*)lstr->getUserData())->c;
		}
		out<<std::endl;
	}

	return true;
}

bool GlobalData::NewDrawSeg( double sx, double sy, double ex, double ey, COLORREF c )
{
	LineSegment ls(sx,sy,ex,ey);
	LineString* lstr = ls.toGeometry(*G::factory).release();
	UserData* ud = new UserData;
	ud->c = c;
	ud->w = 2;
	lstr->setUserData(ud);
	dss.push_back( lstr );
	//tree.insert( lstr->getEnvelopeInternal(), lstr );
	return true;
}

bool GlobalData::DeleteLastDrawSeg()
{
	int num = dss.size();
	if(num<=0) return false;
	LineString* lstr = dss.back();
	if(lastGeom==lstr) lastGeom = 0;
	//tree.remove( lstr->getEnvelopeInternal(), lstr );
	G::destroy(lstr);
	dss.pop_back();
	return true;
}

const geos::geom::Geometry* GlobalData::SearchGeometry( double cx, double cy,
	double w, double h )
{
	geos::util::GeometricShapeFactory gsf(G::factory);
	gsf.setCentre( Coordinate(cx,cy) );
	gsf.setHeight(h);
	gsf.setWidth(w);
	gsf.setNumPoints(4);
	geos::geom::Polygon* poly = gsf.createRectangle();
	geos::geom::Point* cpoint = G::factory->createPoint( Coordinate(cx,cy) );

	std::vector<void*> matches;
	double mind = DBL_MAX;
	geos::geom::Geometry* ret = 0;
	tree.query( poly->getEnvelopeInternal(), matches );
	int num = matches.size();
	for(int i=0; i<num; ++i) {
		geos::geom::Geometry* g = (geos::geom::Geometry*)(matches[i]);
		double dist = g->distance( cpoint );
		if(dist < mind) {
			mind = dist;
			ret = g;
		}
	}
	if(mind > w+h) ret=0;
	std::cout<<"Win("<<w<<","<<h<<") Find@("<<cx<<","<<cy<<"): "
		<<num<<" obj, minD="<<(ret==0?-1:mind)
		<<" @ "<<ret<<std::endl;

	if(lastGeom && lastGeom!=ret) {
		UserData* ud = (UserData*)lastGeom->getUserData();
		ud->w = 1;
	}
	if(ret) {
		UserData* ud = (UserData*)ret->getUserData();
		ud->w = 5;
	}
	lastGeom = ret;

	G::factory->destroyGeometry( poly );
	G::factory->destroyGeometry( cpoint );

	return ret;
}

bool GlobalData::Polygonize()
{
	geos::operation::polygonize::Polygonizer polygonizer;
	for(unsigned int i=0, n=dss.size(); i<n; ++i)
	{
		const Geometry *geometry=dss[i];
		polygonizer.add(geometry);
	}
	polygons = polygonizer.getPolygons();
	//geos::geom::Polygon* p = polygons[0];
}