// DLT_Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

//#include "CalibrateAPI.h"

#include "../lsd/LineSegDetector.h"
void testLSD()
{
	double* image;
	unsigned int x,y,i;
	unsigned int X = 128;  /* x image size */
	unsigned int Y = 128;  /* y image size */

	/* create a simple image: left half black, right half gray */
	image = new double[X*Y];
	for(x=0;x<X;x++)
		for(y=0;y<Y;y++)
			image[ x + y * X ] = x<X/2 ? 0.0 : 64.0; /* image(x,y) */

	/* call LSD */
	LineSegDetectorI lsd;
	lsd.RunOn(image, X, Y);

	/* print output */
	printf("%u line segments found:\n",lsd.GetLineSegNum());
	for(i=0;i<lsd.GetLineSegNum();i++)
	{
		double xs,ys,xe,ye,width;
		lsd.GetLineSeg(i, xs,ys,xe,ye,width);
		printf("%d:(%lf,%lf)<->(%lf,%lf) | %lf\n", i, xs, ys, xe, ye, width);
	}

	/* free memory */
	delete [] image;
};

int _tmain(int argc, _TCHAR* argv[])
{
	testLSD();
	return 0;

	return 0;
}

