//
// FXDlg.cpp : the FX project tab.  See FXDlg.h.
//

#include "bdefs.h"
#include "dedit.h"
#include "fxdlg.h"
#include "d_filemgr.h"
#include "projectbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFXDlg::CFXDlg() : CBaseRezDlg()
{
	m_hPreview = NULL;
	InitBaseRezDlg("fx", &m_FXTree, &m_FXList, RESTYPE_FX);
}

CFXDlg::~CFXDlg()
{
	ClearPreview();
}

void CFXDlg::DoDataExchange(CDataExchange* pDX)
{
	CProjectTabControlBar::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_FXTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_FXList);
}


BEGIN_MESSAGE_MAP(CFXDlg, CBaseRezDlg)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_BASEREZ_LIST, OnListSelChanged)
END_MESSAGE_MAP()


BOOL CFXDlg::OnInitDialogBar()
{
	CMRCSizeDialogBar::OnInitDialogBar();

	CRect rect;
	m_FXList.GetClientRect( &rect );

	m_FXList.SetExtendedStyle(m_FXList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_FXList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	m_FXList.InsertColumn(1,"Size",LVCFMT_RIGHT,(rect.Width()-35)/3,-1);
	m_FXList.InsertColumn(2,"Modified",LVCFMT_LEFT,(rect.Width()-35)/3,-1);

	return TRUE;
}

void CFXDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;

	if(!GetProjectBar()->IsProjectOpen())
		return;

	// Only the folder tree has a menu (new/delete folder).
	if(pWnd->m_hWnd != m_FXTree.m_hWnd)
		return;

	VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetProjectBar());
}

void CFXDlg::OnSize(UINT nType, int cx, int cy)
{
	CProjectTabControlBar::OnSize(nType, cx, cy);
	RepositionControls();
}

void CFXDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	if(hItem)
	{
		DDirIdent *pIdent = (DDirIdent*)m_pTree->GetItemData(hItem);
		if(pIdent)
		{
			m_csCurrentDir = pIdent->m_Filename;
			m_hCurrentItem = hItem;
			PopulateList();
			ClearPreview();
		}
	}

	*pResult = 0;
}

void CFXDlg::OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
	if((pNM->uChanged & LVIF_STATE) && (pNM->uNewState & LVIS_SELECTED))
		ShowSelectedItem();

	*pResult = 0;
}

void CFXDlg::ClearPreview()
{
	CWnd *pPic = GetDlgItem(IDC_FX_PREVIEW);
	if(pPic && ::IsWindow(pPic->m_hWnd))
		pPic->SendMessage(STM_SETIMAGE, IMAGE_BITMAP, 0);

	if(m_hPreview)
	{
		::DeleteObject(m_hPreview);
		m_hPreview = NULL;
	}

	CWnd *pDesc = GetDlgItem(IDC_FX_DESC);
	if(pDesc && ::IsWindow(pDesc->m_hWnd))
		pDesc->SetWindowText("");
}

// Replaces the extension of a full filename.
static CString SwapExtension(const CString &fullName, const char *pNewExt)
{
	int dot = fullName.ReverseFind('.');
	int slash = max(fullName.ReverseFind('\\'), fullName.ReverseFind('/'));
	if(dot < 0 || dot < slash)
		return fullName + pNewExt;

	return fullName.Left(dot) + pNewExt;
}

void CFXDlg::ShowSelectedItem()
{
	int nItem = m_FXList.GetNextItem(-1, LVNI_SELECTED);
	if(nItem < 0)
	{
		ClearPreview();
		return;
	}

	DFileIdent *pIdent = (DFileIdent*)m_FXList.GetItemData(nItem);
	if(!pIdent)
		return;

	ClearPreview();

	CString fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);

	// Preview picture (.bmp next to the .fx file), scaled to the picture control.
	CWnd *pPic = GetDlgItem(IDC_FX_PREVIEW);
	if(pPic)
	{
		CRect rc;
		pPic->GetClientRect(&rc);
		m_hPreview = (HBITMAP)::LoadImage(NULL, SwapExtension(fullName, ".bmp"), IMAGE_BITMAP,
			rc.Width(), rc.Height(), LR_LOADFROMFILE);
		if(m_hPreview)
			pPic->SendMessage(STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_hPreview);
	}

	// Description (.txt next to the .fx file).
	CWnd *pDesc = GetDlgItem(IDC_FX_DESC);
	if(pDesc)
	{
		CString text;
		FILE *fp = fopen(SwapExtension(fullName, ".txt"), "rb");
		if(fp)
		{
			char buf[1024];
			size_t n;
			while((n = fread(buf, 1, sizeof(buf), fp)) > 0 && text.GetLength() < 8000)
				text += CString(buf, (int)n);
			fclose(fp);

			// Edit controls want CR/LF line breaks.
			text.Replace("\r\n", "\n");
			text.Replace("\n", "\r\n");
		}
		pDesc->SetWindowText(text);
	}
}

void CFXDlg::RepositionControls()
{
	if(!::IsWindow(m_hWnd))
		return;

	CRect rect;
	GetClientRect(&rect);
	int w = rect.Width(), h = rect.Height();

	int hTree = h * 25 / 100;
	int hList = h * 25 / 100;
	int hPic  = h * 28 / 100;
	int hDesc = h - hTree - hList - hPic;

	CWnd *p;
	if(p = GetDlgItem(IDC_BASEREZ_TREE))	p->MoveWindow(0, 0, w, hTree);
	if(p = GetDlgItem(IDC_BASEREZ_LIST))	p->MoveWindow(0, hTree, w, hList);
	if(p = GetDlgItem(IDC_FX_PREVIEW))		p->MoveWindow(0, hTree + hList, w, hPic);
	if(p = GetDlgItem(IDC_FX_DESC))			p->MoveWindow(0, hTree + hList + hPic, w, hDesc);

	if(m_pList && m_pList->m_hWnd)
	{
		CRect r;
		m_pList->GetClientRect(&r);
		for(int i=0; i < 3; i++)
			m_pList->SetColumnWidth(i, r.Width()/3);
	}
}
