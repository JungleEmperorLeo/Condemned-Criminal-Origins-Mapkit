//
// FXDlg.h : the FX project tab.
//
// Lists FX files ("*.fx") found in project folders marked as FX folders and shows
// a preview picture and a description next to the selected one.  For an item
// "Fire_Gas_10cm.fx" the editor looks for "Fire_Gas_10cm.bmp" (preview) and
// "Fire_Gas_10cm.txt" (description) in the same folder.
//
#ifndef __FXDLG_H__
#define __FXDLG_H__

#include "resourcemgr.h"
#include "baserezdlg.h"

class CFXDlg : public CBaseRezDlg
{
public:
	CFXDlg();
	virtual ~CFXDlg();

	enum { IDD = IDD_FX_TABDLG };
	CTreeCtrl	m_FXTree;
	CListCtrl	m_FXList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialogBar();
	virtual void RepositionControls();

	void ShowSelectedItem();
	void ClearPreview();

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	HBITMAP		m_hPreview;
};

#endif
