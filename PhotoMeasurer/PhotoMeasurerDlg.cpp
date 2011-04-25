// PhotoMeasurerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PhotoMeasurer.h"
#include "PhotoMeasurerDlg.h"
#include "..\cximage-6.0.0\demo\memdc.h"

#include "..\Console\CharUtils.h"
#include "..\Console\Console.h"
#include "..\Console\updator.h"
#include "GlobalData.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <process.h>
#include <set>
#include <map>

#include "../lsd/LineSegDetector.h"
#include "../JLinkageLib/JlnkSample.h"
#include "../JLinkageLib/JlnkCluster.h"

#define BKCOLOR_DEFAULT RGB(55,174,238)
#define BKCOLOR_PROGRESSING RGB(244,78,43)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

	
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPhotoMeasurerDlg


CPhotoMeasurerDlg::CPhotoMeasurerDlg(CString imgPath, CWnd* pParent /*=NULL*/)
: CDialog(CPhotoMeasurerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_imgPath = imgPath;
	m_rect.SetRectEmpty();
	m_dragging = false;
	m_scale = 1.25;
	m_img = NULL;
	hThread = NULL;
	m_bkColor = BKCOLOR_DEFAULT;
	for(int i=0; i<PAINTNUM; ++i)
		bPaint[i] = true;
	bDrawSegTobeDone = false;
	bSearch = true;
	bSelSignNotSeg = true;
	cmd = NOTHING;
}

CPhotoMeasurerDlg::~CPhotoMeasurerDlg()
{
	NewColorMap(0);
	if(m_img) delete m_img;
	m_img = NULL;
}

void CPhotoMeasurerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPhotoMeasurerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


CString FindExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return name.Mid(i+1);
		}
	}
	return CString(_T(""));
}

BOOL CPhotoMeasurerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	ShowWindow(SW_MAXIMIZE);

	this->BeginWaitCursor();

	CString filename(m_imgPath);
	CString ext(FindExtension(filename));
	ext.MakeLower();
	if (ext == _T("")) return FALSE;

	int type = CxImage::GetTypeIdFromName(ext);
	m_img = new CxImage(filename, type);

	if (!m_img->IsValid()){
		CString s = ::MCBS_2_LOCAL(m_img->GetLastError());
		AfxMessageBox(s);
		delete m_img;
		m_img = NULL;
		return FALSE;
	} else {
		m_rect.SetRect(0,0,m_img->GetWidth(),m_img->GetHeight());
	}
	this->EndWaitCursor();


	return TRUE;
}

void CPhotoMeasurerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CPhotoMeasurerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		dc.SetBkColor(m_bkColor);

		//double frame
		CMemDC* pMemDC = new CMemDC(&dc);

		if(bPaint[PIC])
			m_img->Draw(pMemDC->GetSafeHdc(), m_rect);
		if(bPaint[TIP])
			PaintTips(pMemDC);
		PaintGlobalData(pMemDC);

		delete pMemDC;

		CDialog::OnPaint();
	}
}

HCURSOR CPhotoMeasurerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPhotoMeasurerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	this->Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
// Manipulate image
bool CPhotoMeasurerDlg::SelectWorldOnScreen(const CPoint& point, double& xo, double& yo)
{
	if(!bSearch || GlobalData::Instance().IsLocked()) return false;

	double x=point.x,y=point.y;
	ScreenToWorld(x,y);

	//search whether hit or not
	GlobalData& g = GlobalData::Instance();
	double cx = point.x, cy = point.y, x1=cx+4, y1=cy+4;
	ScreenToWorld(cx,cy);
	ScreenToWorld(x1,y1);
	double w = max(x1-cx,4);
	double h = max(y1-cy,4);
	const geos::geom::Point* pt = dynamic_cast<const geos::geom::Point*>(
		g.SearchGeometry( cx, cy, w, h ) );
	if(pt) {
		x = pt->getCoordinate()->x;
		y = pt->getCoordinate()->y;
	}

	Console::me(__FUNCTION__) << "(" << x << "," << y << ")" << std::endl;
	xo=x, yo=y;

	this->Invalidate(FALSE);

	return pt?true:false;
}

void CPhotoMeasurerDlg::MoveScreen(const CPoint& from, const CPoint& to)
{
	int sx=from.x, sy=from.y;
	int ex=to.x, ey=to.y;

	int nx = ex-sx+m_dragO.x, ny=ey-sy+m_dragO.y;
	int w=m_rect.Width(),h=m_rect.Height();
	m_rect.SetRect(nx,ny,nx+w,ny+h);
}

void CPhotoMeasurerDlg::ZoomScreen(bool isZoomIn, const CPoint& pt)
{
	double mx = pt.x, my = pt.y;
	ScreenToWorld(mx,my);//get current cursor coordinate in the image

	double x0 = m_rect.left;
	double y0 = m_rect.top;
	double w = m_rect.Width();
	double h = m_rect.Height();

	double scale = isZoomIn?(m_scale):(1.0/m_scale);
	if( !( (isZoomIn && getScaleX()>10) ||
		(!isZoomIn && getScaleX()<0.1) ) ) {
			w *= scale;
			h = w / getAspectRatio();
			m_rect.SetRect((int)x0,(int)y0,(int)(x0+w),(int)(y0+h));//scale

			WorldToScreen(mx,my);

			x0 += pt.x-mx;
			y0 += pt.y-my;

			m_rect.SetRect((int)x0,(int)y0,(int)(x0+w),(int)(y0+h));

			this->Invalidate(FALSE);
	}
}

void CPhotoMeasurerDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	updatePt(point);
	cmd = END;
	double xo,yo;
	bool found = SelectWorldOnScreen(point, xo, yo);
	curX = xo;
	curY = yo;
	WorldToScreen(xo,yo);
	curPt.x=(long)xo, curPt.y=(long)yo;
	cmd = NOTHING;

	GlobalData& g = GlobalData::Instance();
	if(bSelSignNotSeg) {
		if(!found) g.NewSign(curX,curY);
	} else {
		double sx=lastX, sy=lastY, ex=curX, ey=curY;
		if(!found) g.NewSign(curX,curY);
		g.NewDrawSeg(sx,sy,ex,ey);
	}
}

void CPhotoMeasurerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	updatePt(point,true);
	double xo,yo;
	bool found = SelectWorldOnScreen(point, xo, yo);
	curX = lastX = xo;
	curY = lastY = yo;
	WorldToScreen(xo,yo);
	curPt.x=(long)xo, curPt.y=(long)yo;
	cmd = BEGIN;

	GlobalData& g = GlobalData::Instance();
	if(!bSelSignNotSeg) {
		if(!found) g.NewSign(curX,curY);
	}
}

void CPhotoMeasurerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	updatePt(point);

	if(cmd==BEGIN)	cmd=EDITING;
	if(m_dragging)	MoveScreen(m_dragS, point);
	if(bSearch || !GlobalData::Instance().IsLocked()) {
		double x = curPt.x, y = curPt.y, x1=x+4, y1=y+4;
		ScreenToWorld(x,y);
		ScreenToWorld(x1,y1);
		double w = max(x1-x,4);
		double h = max(y1-y,4);
		GlobalData::Instance().SearchGeometry( x, y, w, h );
	}

	this->Invalidate(FALSE);
}

BOOL CPhotoMeasurerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//IMPORTANT!!! OnMouseWheel: pt in screen coordinate system, change it in client coordinate
	ScreenToClient(&pt);

	ZoomScreen((zDelta<0), pt);

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CPhotoMeasurerDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	MoveScreen(m_dragS, point);

	m_dragging = false;

	this->Invalidate(FALSE);
}

void CPhotoMeasurerDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_dragS = point;
	m_dragO = m_rect.TopLeft();
	m_dragging = true;
}

BOOL CPhotoMeasurerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	static HCURSOR hnewCur = LoadCursor( NULL , IDC_CROSS);
	::SetCursor(hnewCur);
	return TRUE;
}

BOOL CPhotoMeasurerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN) {
		int dx=0,dy=0;
		switch(pMsg->wParam)
		{
		case VK_RETURN:
			OnLButtonUp(0,curPt);
			return TRUE;
		case VK_ESCAPE:
			return TRUE;
		case VK_ADD:
			ZoomScreen(true, curPt); break;
		case VK_SUBTRACT:
			ZoomScreen(false, curPt); break;
		case VK_DELETE:
			{
				if(bSelSignNotSeg) {
					GlobalData::Instance().DeleteLastSign();
				} else {
					GlobalData::Instance().DeleteLastLineSeg();
				}
				//GlobalData::Instance().DeleteLastLineSeg();
				break;
			}
		case VK_UP:
			dy = 1; break;
		case VK_DOWN:
			dy = -1; break;
		case VK_LEFT:
			dx = 1; break;
		case VK_RIGHT:
			dx = -1; break;
		}

		if( GetKeyState(VK_LCONTROL) & 0x8000 )
			dx *= 20, dy *= 20;

		m_rect.left += dx; m_rect.right += dx;
		m_rect.top += dy; m_rect.bottom += dy;
		this->Invalidate(FALSE);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CPhotoMeasurerDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case 'S':
		{
			if(GetKeyState(VK_LCONTROL) & 0x8000) {
				CFileDialog fdlg(TRUE, 0, 0
					, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
					, _T("ALL FILES(*.*)|*.*||")
					, 0);
				fdlg.m_ofn.lpstrTitle = _T("Choose File Saving Path");
				if(fdlg.DoModal()!=IDOK)
					return;

				CString path = fdlg.GetPathName();
				GlobalData::Instance().SaveSignPts(LOCAL_2_MCBS(path));
			} else {
				updatePt(curPt, true);
				bSelSignNotSeg = !bSelSignNotSeg;
				Console::info()<<"Select "<<(bSelSignNotSeg?"Sign":"Seg")<<Console::endl;
			}
			return;
		}
	case 'A':
		{
			bSearch=!bSearch;
			return;
		}
	case 'D':
		{
			if(GetKeyState(VK_LCONTROL) & 0x8000)
				On_Ctrl_D();
			return;
		}
	case 'J':
		{
			if(GetKeyState(VK_LCONTROL) & 0x8000)
				On_Ctrl_J();
			return;
		}
	case 'N':
		{
			if(GetKeyState(VK_LCONTROL) & 0x8000)
				On_Ctrl_N();
			return;
		}
	default:
		{
			if('0'<=nChar && nChar<'0'+PAINTNUM)
				bPaint[nChar-'0']=!bPaint[nChar-'0'];
			return;
		}
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void RunLSDJlnk(void *lpParam)
{
	GlobalData& g = GlobalData::Instance();
	CPhotoMeasurerDlg* dlg = (CPhotoMeasurerDlg*)lpParam;
	if(dlg==NULL) return;
	if(dlg->m_img==NULL || g.IsLocked()) {
		dlg->hThread = 0;
		return;
	}
	dlg->m_bkColor = BKCOLOR_PROGRESSING;
	dlg->Invalidate(FALSE);
	g.Lock();

	Console::me(0,true);

	std::vector< std::vector<float>* > tmpls;
	{//1. Line Seg Detect
		Updator::InitializeWaitbar("Line Segment Detector ");

		unsigned int W = dlg->m_img->GetWidth();
		unsigned int H = dlg->m_img->GetHeight();
		double* pimg = new double[W*H];
		for(unsigned int ih = 0; ih<H; ++ih) {
			for(unsigned int iw = 0; iw<W; ++iw) {
				pimg[iw +ih*W] = dlg->m_img->GetPixelGray(iw,H-ih);
			}
		}

		LineSegDetectorI lsd;
		lsd.RunOn(pimg, W, H);

		const unsigned int num = lsd.GetLineSegNum();
		printf("%u line segments found\n", num);

		for(unsigned int i=0;i<num;i++)
		{
			Updator::UpdateWaitbar((float)i/(float)num);
			double sx,sy,ex,ey,width;
			lsd.GetLineSeg(i,sx,sy,ex,ey,width);
			double dx = fabs(sx-ex), dy = fabs(sy-ey);
			if(dx*dx+dy*dy>400) {
				std::vector<float>* newls = new std::vector<float>(4);
				(*newls)[0] = (float)sx; (*newls)[1] = (float)sy;
				(*newls)[2] = (float)ex; (*newls)[3] = (float)ey;
				tmpls.push_back(newls);
			}
		}

		delete [] pimg;
		Updator::CloseWaitbar();
	}

	{//1.1 add draw segments
		const GlobalData::LineSegmentArr& dss = g.GetDrawSegments();
		const unsigned int dnum = (unsigned int)dss.size();
		for(unsigned int i=0; i<dnum; ++i) {
			geos::geom::LineString* ls = dss[i];
			if(ls->getNumPoints()!=2) continue;
			std::vector<float>* newls = new std::vector<float>(4);
			(*newls)[0] = (float)ls->getCoordinateN(0).x;
			(*newls)[1] = (float)ls->getCoordinateN(0).y;
			(*newls)[2] = (float)ls->getCoordinateN(1).x;
			(*newls)[3] = (float)ls->getCoordinateN(1).y;
			tmpls.push_back(newls);
		}
		std::cout<<"Add "<<dnum<<" DrawSegments!"<<std::endl;
	}

	std::vector<unsigned int> Lables;
	std::vector<unsigned int> LableCount;
	{//2. Jlnk cluster
		std::vector<std::vector<float> *> *mModels = 
			JlnkSample::run(&tmpls, 5000, 2, 0, 3);
		g.classNum = JlnkCluster::run(Lables, LableCount, &tmpls, mModels, 2, 2);
		std::cout<<"Jlnk found "<<g.classNum<<" classes!"<<std::endl;

		//2.1. release other resource
		for(unsigned int i=0; i < mModels->size(); ++i)
			delete (*mModels)[i];
		delete mModels;
	}

	std::vector<COLORREF> colormap;
	{//2.2 colormap TODO pseudo color
		std::vector<COLORREF> tmp;
		int num = (int)ceil(g.classNum/3.0)*3.0;
		double h = 0, dh = 255;
		if(num>1) dh /= (num-1);
		for(int i=0; i<num; ++i, h+=dh) {
			tmp.push_back( CxImage::RGBQUADtoRGB( CxImage::HSLtoRGB(RGB(h,255,255/2)) ) );
		}
		int basis[3] = {0,num/3,2*num/3};
		colormap.resize(num);
		for(int i=0; i<num; ++i) {
			colormap[i] = tmp[ basis[i%3]+((int)i/3) ];
		}
	}

	unsigned int lnum = Lables.size();
	{//3. finally, vp calibration and save data to GlobalData
		std::ofstream out("lsdjlnk.txt");
		std::ofstream vout("vpcalib.txt");
		out << std::setiosflags(std::ios::scientific);
		vout << std::setiosflags(std::ios::scientific);
		if(lnum<2) vout << "Only find "<<lnum
			<<" classes of line segments, not enough for vp calibration, exit!"<<std::endl;
		std::vector< std::vector<std::vector<float>* > > lineclass(LableCount.size());

		for(unsigned int i=0; i<lnum; ++i) {
			std::vector<float>* s = tmpls[i];
			lineclass[ Lables[i] ].push_back( s );
			g.NewLineSeg(s->at(0),s->at(1),s->at(2),s->at(3), colormap[Lables[i]], Lables[i]);
			out<<s->at(0)<<" "<<s->at(1)<<" "
				<<s->at(2)<<" "<<s->at(3)<<" "<<Lables[i]<<std::endl;
			//delete s;//release it
		}
		out.close();

// 		for(unsigned int i=0; i<g.classNum; ++i) {
// 			calibrate::ImagePoint ip;
// 			calibrate::Estimate_VP(lineclass[i],ip);
// 			vout<<ip[0]<<" "<<ip[1]<<std::endl;
// 			std::vector<double> vp;
// 			vp.push_back(ip[0]);
// 			vp.push_back(ip[1]);
// 			g.vps.push_back(vp);
// 		}
// 
// 		std::vector<double>& vp0=g.vps[0], &vp1=g.vps[1];
// 		g.f = sqrt( abs( (vp0[0]-g.W/2)*(vp1[0]-g.W/2) + (vp0[1]-g.H/2)*(vp1[1]-g.H/2) ) );
// 		vout<<"focal length = "<<g.f<<std::endl;

		for(unsigned int i=0; i<lnum; ++i) {
			std::vector<float>* s = tmpls[i];
			delete s;//release it
		}
		vout.close();
		tmpls.clear();
	}

	g.UnLock();
	dlg->m_bkColor = BKCOLOR_DEFAULT;
	dlg->Invalidate(FALSE);
	dlg->hThread=0;
	_endthread();
	return;
}

void CPhotoMeasurerDlg::On_Ctrl_D()
{
	static bool done = false;
	if(!done && !hThread) {
		done = true;
		hThread=(HANDLE)_beginthread(RunLSDJlnk,0,this);
	}
}

void CPhotoMeasurerDlg::PaintGlobalData( CDC* pDC )
{
	GlobalData& g = GlobalData::Instance();
	if(g.IsLocked()) return;

	HPEN oldpen = (HPEN)pDC->GetCurrentPen()->GetSafeHandle();
	if(bPaint[DRAWSEG])
	{//draw drawsegment
		GlobalData::LineSegmentArr& dss = g.GetDrawSegments();
		int dnum = dss.size();
		for(int i=0; i<dnum; ++i) {
			geos::geom::LineString* lstr = dss[i];
			if(lstr->getNumPoints()!=2) continue;

			GlobalData::UserData* ud=0;
			if(ud = (GlobalData::UserData*)lstr->getUserData() ) {
				HPEN newpen = CreatePen(PS_DASH,ud->w,ud->c);
				pDC->SelectObject( newpen );
			}

			const geos::geom::Coordinate &c0=lstr->getCoordinateN(0),
				&c1=lstr->getCoordinateN(1);
			double xs=c0.x, ys=c0.y,xe=c1.x, ye=c1.y;
			WorldToScreen(xs,ys);
			WorldToScreen(xe,ye);
			pDC->MoveTo(xs,ys);
			pDC->LineTo(xe,ye);

			if(ud) {
				HPEN pen = (HPEN)pDC->SelectObject(oldpen);
				DeleteObject(pen);
			}
		}//end of for
	}

	if(bPaint[LINESEG])
	{//draw lineSegment
		GlobalData::LineSegmentArr& lss = g.GetLineSegments();
// 		if(g.classNum!=colormap.size())
// 			NewColorMap(g.classNum);
		int lnum = lss.size();
		for(int i=0; i<lnum; ++i) {
			geos::geom::LineString* lstr = lss[i];
			if(lstr->getNumPoints()!=2) continue;

			GlobalData::UserData* ud=0;
			if(ud = (GlobalData::UserData*)lstr->getUserData() ) {
				HPEN newpen = CreatePen(PS_SOLID,ud->w,ud->c);
				pDC->SelectObject( newpen );
			}

			const geos::geom::Coordinate &c0=lstr->getCoordinateN(0),
				&c1=lstr->getCoordinateN(1);
			double xs=c0.x, ys=c0.y,xe=c1.x, ye=c1.y;
			WorldToScreen(xs,ys);
			WorldToScreen(xe,ye);
			pDC->MoveTo(xs,ys);
			pDC->LineTo(xe,ye);

			if(ud) {
				HPEN pen = (HPEN)pDC->SelectObject(oldpen);
				DeleteObject(pen);
			}
		}//end of for
	}

	if(cmd==BEGIN || cmd==EDITING) {
		HPEN newpen = CreatePen(PS_SOLID,5,RGB(255,0,0));
		pDC->SelectObject(newpen);

		if(!bSelSignNotSeg)
		{
			double sx=lastX,sy=lastY,ex=curX,ey=curY;
			WorldToScreen(sx,sy);
			WorldToScreen(ex,ey);
			pDC->MoveTo(sx,sy);
			pDC->LineTo(ex,ey);
		}

		{//draw curpt
			double x = curPt.x, y = curPt.y;
			pDC->MoveTo(x-5,y);
			pDC->LineTo(x+5,y);
			pDC->MoveTo(x,y-5);
			pDC->LineTo(x,y+5);
		}
		HPEN pen = (HPEN)pDC->SelectObject(oldpen);
		DeleteObject(pen);
	}

	if(bPaint[SIGN])
	{//draw signPts
		CString str;
		GlobalData::PointArr& pts = g.GetSignPts();
		int num = pts.size();
		for(int i=0; i<num; ++i) {
			geos::geom::Point* pt = pts.at(i);

			GlobalData::UserData* ud=0;
			if(ud = (GlobalData::UserData*)pt->getUserData()) {
				HPEN newpen = CreatePen(PS_SOLID,ud->w,ud->c);
				pDC->SelectObject(newpen);
			}

			double x=pt->getX(),
				y=pt->getY();
			WorldToScreen(x,y);
			pDC->MoveTo(x-5,y);
			pDC->LineTo(x+5,y);
			pDC->MoveTo(x,y-5);
			pDC->LineTo(x,y+5);
			str.Format(_T("%d"),i);
			pDC->TextOut(x,y,str);

			if(ud) {
				HPEN pen = (HPEN)pDC->SelectObject(oldpen);
				DeleteObject(pen);
			}
		}//end of for
	}
}

void CPhotoMeasurerDlg::PaintTips( CDC* pDC )
{
	CRect rect;
	this->GetClientRect(rect);

	int orgROP2 = pDC->GetROP2();
	pDC->SetROP2(R2_NOT);//draw inverse color line

	//draw axis
	double x=curPt.x, y=curPt.y;
	ScreenToWorld(x,y);

	CString str;
	//vertical axis
	pDC->MoveTo(curPt.x,0);
	pDC->LineTo(curPt.x,rect.Height());
	str.Format(_T("%lf"), x);
	pDC->TextOut(curPt.x, 10, str);

	//horizontal axis
	pDC->MoveTo(0,curPt.y);
	pDC->LineTo(rect.Width(),curPt.y);
	str.Format(_T("%lf"), y);
	pDC->TextOut(10, curPt.y, str);

	pDC->SetROP2(orgROP2);
}

void CPhotoMeasurerDlg::On_Ctrl_J()
{
	GlobalData& g = GlobalData::Instance();
	double sx = rand()%m_img->GetWidth();
	double sy = rand()%m_img->GetHeight();
	double ex = rand()%m_img->GetWidth();
	double ey = rand()%m_img->GetHeight();
	g.NewDrawSeg(sx,sy,ex,ey);
}

void CPhotoMeasurerDlg::NewColorMap( int num )
{
// 	colormap.clear();
// 	double h = 0, dh = 255;
// 	if(num>1) dh /= (num-1);
// 	for(int i=0; i<num; ++i, h+=dh) {
// 		colormap.push_back( CxImage::RGBQUADtoRGB( CxImage::HSLtoRGB(RGB(h,255,255/2)) ) );
// 	}
// 	int cnt = rand()%10;
// 	while(cnt--)
// 		std::next_permutation(colormap.begin(), colormap.end());
}

void CPhotoMeasurerDlg::On_Ctrl_N()
{
	NewColorMap(GlobalData::Instance().classNum);
}