// PhotoMeasurerDlg.h : 头文件
//

#pragma once

#include "ximajpg.h"
#include <vector>

class CxImage;

// CPhotoMeasurerDlg 对话框
class CPhotoMeasurerDlg : public CDialog
{
// 构造
public:
	CPhotoMeasurerDlg(CString imgPath, CWnd* pParent = NULL);	// 标准构造函数
	~CPhotoMeasurerDlg();

// 对话框数据
	enum { IDD = IDD_PHOTOMEASURER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	CString m_imgPath;

	enum CmdType {
		BEGIN=0,
		EDITING,
		END,
		NOTHING,
		CMDNUM
	};
	unsigned int cmd;

	enum PaintType {
		PIC=0,
		SIGN,
		LINESEG,
		DRAWSEG,
		TIP,
		PAINTNUM
	};
	bool bPaint[PAINTNUM];
	bool bSelSignNotSeg;
	bool bSearch;
	bool bDrawSegTobeDone;

	CRect m_rect;//draw rect
	bool m_dragging;
	CPoint m_dragS;//start point of draging
	CPoint m_dragO;//org
	double m_scale;

	CPoint curPt;
	double lastX,lastY,curX,curY;//world coordinate
	inline void updatePt(const CPoint& pt, bool remember=false)
	{
		curPt=pt; curX=pt.x; curY=pt.y;
		ScreenToWorld(curX,curY);
		if(remember) lastX=curX, lastY=curY;
	}

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	void PaintGlobalData(CDC* pDC);
	void PaintTips(CDC* pDC);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CxImage *m_img;
	HANDLE	hThread;	//elaboration thread
	volatile COLORREF m_bkColor;

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	//return xo,yo in world coordinate
	//return true is found point otherwise false
	bool SelectWorldOnScreen(const CPoint& point, double& xo, double& yo);
	void MoveScreen(const CPoint& from, const CPoint& to);
	void ZoomScreen(bool isZoomIn, const CPoint& pt);

	inline double getScaleX() { return (double)m_rect.Width()/m_img->GetWidth(); }
	inline double getScaleY() { return (double)m_rect.Height()/m_img->GetHeight(); }
	inline double getAspectRatio() { return (double)m_img->GetWidth()/m_img->GetHeight(); }

	template<class T>
	void ScreenToWorld(T& x, T& y)
	{
		double x0 = m_rect.left;
		double y0 = m_rect.top;
		x-=x0; y-=y0;
		x/=getScaleX(); y/=getScaleY();
	}

	template<class T>
	void WorldToScreen(T& x, T& y)
	{
		double x0 = m_rect.left;
		double y0 = m_rect.top;
		x*=getScaleX(); y*=getScaleY();
		x+=x0; y+=y0;
	}
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	void On_Ctrl_D();
	void On_Ctrl_J();

	void NewColorMap(int num);
	void On_Ctrl_N();
};
