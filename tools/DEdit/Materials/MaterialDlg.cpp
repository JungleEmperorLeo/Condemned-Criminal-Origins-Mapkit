//------------------------------------------------------------------
//
//	FILE	  : MaterialDlg.cpp
//
//	PURPOSE	  : See MaterialDlg.h
//
//------------------------------------------------------------------

#include "bdefs.h"
#include <vector>
#include "dedit.h"
#include "resource.h"
#include "MaterialDlg.h"
#include "MaterialFile.h"
#include "MaterialTexture.h"
#include "MaterialPropsDlg.h"
#include "projectbar.h"
#include "edithelpers.h"
#include "d_filemgr.h"
#include "texture.h"

enum
{
	CMD_MAT_PROPERTIES = 1,
	CMD_MAT_PALETTE,
	CMD_MAT_REFRESH,
	CMD_MAT_NEWINSTANCE,
	CMD_MAT_COPY,
	CMD_MAT_EDITSELECTED,
	CMD_MAT_WRITABLE,
	CMD_MAT_RENAME,
	CMD_MAT_FINDINWORLD,
	CMD_MAT_FINDFILE,
	CMD_MAT_SC1, CMD_MAT_SC2, CMD_MAT_SC3, CMD_MAT_SC4	// source control entries (not available here)
};

// A tiny "type a name" dialog, built in code.
class CMatNameDlg : public CDialog
{
public:
	CMatNameDlg(const char *pTitle, const char *pPrompt, const char *pText) : m_Title(pTitle), m_Prompt(pPrompt), m_Text(pText) {}
	CString m_Title, m_Prompt, m_Text;

	INT_PTR Run(CWnd *pParent)
	{
		m_T.clear();
		DWORD style = DS_MODALFRAME | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
		m_T.push_back((WORD)(style & 0xFFFF)); m_T.push_back((WORD)(style >> 16));
		m_T.push_back(0); m_T.push_back(0); m_T.push_back(0);
		m_T.push_back(0); m_T.push_back(0); m_T.push_back(220); m_T.push_back(60);
		m_T.push_back(0); m_T.push_back(0);
		for(const char *p = (LPCTSTR)m_Title; *p; p++) m_T.push_back((WORD)(unsigned char)*p);
		m_T.push_back(0);
		m_T.push_back(8);
		const char *f = "MS Shell Dlg";
		for(; *f; f++) m_T.push_back((WORD)*f);
		m_T.push_back(0);
		while(m_T.size() & 1) m_T.push_back(0);
		InitModalIndirect((LPCDLGTEMPLATE)&m_T[0], pParent);
		return DoModal();
	}

protected:
	virtual BOOL OnInitDialog()
	{
		CDialog::OnInitDialog();
		CFont *pFont = GetFont();
		CRect r;
		r = CRect(6, 6, 214, 16);   MapDialogRect(&r);
		m_st.Create(m_Prompt, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX, r, this, (UINT)-1);
		r = CRect(6, 20, 214, 34);  MapDialogRect(&r);
		m_ed.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, r, this, 101);
		r = CRect(100, 40, 154, 54); MapDialogRect(&r);
		m_ok.Create("OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, r, this, IDOK);
		r = CRect(158, 40, 214, 54); MapDialogRect(&r);
		m_cancel.Create("Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, r, this, IDCANCEL);
		if(pFont) { m_st.SetFont(pFont); m_ed.SetFont(pFont); m_ok.SetFont(pFont); m_cancel.SetFont(pFont); }
		m_ed.SetWindowText(m_Text);
		m_ed.SetSel(0, -1);
		m_ed.SetFocus();
		CenterWindow();
		return FALSE;
	}
	virtual void OnOK()
	{
		m_ed.GetWindowText(m_Text);
		m_Text.Trim();
		CDialog::OnOK();
	}
private:
	std::vector<WORD> m_T;
	CStatic m_st; CEdit m_ed; CButton m_ok, m_cancel;
};

// Full path without its extension, folder, and bare name.
static void SplitMatPath(const CString &full, CString &dir, CString &stem)
{
	int slash = full.ReverseFind('\\');
	dir = (slash >= 0) ? full.Left(slash) : CString();
	CString file = (slash >= 0) ? full.Mid(slash + 1) : full;
	int dot = file.ReverseFind('.');
	stem = (dot >= 0) ? file.Left(dot) : file;
}

static bool ValidMatName(const CString &name)
{
	if(name.IsEmpty()) return false;
	return name.FindOneOf("\\/:*?\"<>|") < 0;
}

BEGIN_MESSAGE_MAP(CMaterialDlg, CTextureDlg)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDC_BASEREZ_LIST, OnOpenMaterial)
END_MESSAGE_MAP()

CMaterialDlg::CMaterialDlg()
{
	// The texture dialog set itself up for .dtx files; point it at materials instead.
	InitBaseRezDlg("Mat00", &m_TextureTree, &m_TextureList, RESTYPE_MATERIAL);
}

CMaterialDlg::~CMaterialDlg()
{
}

void CMaterialDlg::FixColumnTitle()
{
	if(!::IsWindow(m_TextureList.m_hWnd))
		return;

	LVCOLUMN col;
	memset(&col, 0, sizeof(col));
	col.mask = LVCF_TEXT;
	col.pszText = (LPSTR)"Shader";
	m_TextureList.SetColumn(1, &col);
}

void CMaterialDlg::UpdateDirectories()
{
	// Textures named in materials are relative to the project folder.
	MaterialTexture_SetResourceRoot(dfm_GetBaseDir(GetFileMgr()));

	CTextureDlg::UpdateDirectories();
	FixColumnTitle();
}

// The "Size" column of a texture list doesn't make sense here: show the shader instead.
void CMaterialDlg::SetListItemText(int nItem, CString &relativeFilename)
{
	if(nItem == 0)
		FixColumnTitle();

	CString full = dfm_GetFullFilename(GetFileMgr(), relativeFilename);
	MatFile::Material mat;
	std::string err;
	if(MatFile::LoadFile((LPCTSTR)full, mat, &err))
	{
		std::string shader = mat.shader;
		size_t slash = shader.find_last_of("\\/");
		if(slash != std::string::npos)
			shader = shader.substr(slash + 1);
		size_t dot = shader.find_last_of('.');
		if(dot != std::string::npos)
			shader = shader.substr(0, dot);
		m_TextureList.SetItemText(nItem, 1, shader.c_str());
	}
	else
	{
		m_TextureList.SetItemText(nItem, 1, "(unreadable)");
	}
}

DFileIdent* CMaterialDlg::GetSelectedIdent()
{
	int nItem = m_TextureList.GetNextItem(-1, LVNI_SELECTED);
	if(nItem < 0)
		return NULL;
	return (DFileIdent*)m_TextureList.GetItemData(nItem);
}

void CMaterialDlg::OnMaterialChanged(DFileIdent *pIdent)
{
	// Throw away every cached picture so the new diffuse map / settings show up.
	dib_FreeAllTextures();

	int nItem = m_TextureList.GetNextItem(-1, LVNI_SELECTED);
	if(nItem >= 0 && pIdent)
	{
		CString name = pIdent->m_Filename;
		SetListItemText(nItem, name);
	}

	if(GetProjectBar()->m_bShowThumbnails)
		UpdateThumbnails();

	RenderLargeImage();
	m_TextureList.Invalidate();
	RedrawAllDocuments();
}

BOOL CMaterialDlg::DoMaterialProperties()
{
	DFileIdent *pIdent = GetSelectedIdent();
	if(!pIdent || !pIdent->m_Filename)
		return FALSE;

	CString full = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
	CMaterialPropsDlg dlg((LPCTSTR)full, pIdent->m_Filename, AfxGetMainWnd());
	if(dlg.RunModal() == IDOK)
		OnMaterialChanged(pIdent);

	return TRUE;
}

void CMaterialDlg::OnOpenMaterial(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT point;
	UINT nFlags;

	GetCursorPos(&point);
	m_TextureList.ScreenToClient(&point);

	int nItem = m_TextureList.HitTest(point, &nFlags);
	if(nItem >= 0)
	{
		ChangeSelection(nItem, TRUE);
		DoMaterialProperties();
	}

	*pResult = 0;
}

void CMaterialDlg::OnContextMenu(CWnd *pWnd, CPoint point)
{
	if(!GetProjectBar()->IsProjectOpen())
		return;
	if(pWnd->m_hWnd != m_TextureList.m_hWnd)
		return;

	CMenu menu;
	menu.CreatePopupMenu();
	bool bHaveSel = GetSelectedIdent() != NULL;
	UINT sel = bHaveSel ? 0 : MF_GRAYED;
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_NEWINSTANCE, "New Material Instance");
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_COPY, "Create Copy");
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_EDITSELECTED, "Edit Selected Materials");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_WRITABLE, "Make Selection Writable");
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_PALETTE, "Add To Palette");
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_RENAME, "Rename Material...");
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_FINDINWORLD, "Find Material In World");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_FINDFILE, "Find Local File");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | MF_GRAYED, CMD_MAT_SC1, "Submit Pending Changes");
	menu.AppendMenu(MF_STRING | MF_GRAYED, CMD_MAT_SC2, "Undo Pending Changes");
	menu.AppendMenu(MF_STRING | MF_GRAYED, CMD_MAT_SC3, "Get Latest Version");
	menu.AppendMenu(MF_STRING | MF_GRAYED, CMD_MAT_SC4, "History");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | sel, CMD_MAT_PROPERTIES, "Material Properties...");
	menu.AppendMenu(MF_STRING, CMD_MAT_REFRESH, "Refresh");

	int cmd = (int)menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, this);
	switch(cmd)
	{
	case CMD_MAT_PROPERTIES:	DoMaterialProperties();		break;

	case CMD_MAT_NEWINSTANCE:
	case CMD_MAT_COPY:
		{
			DFileIdent *pIdent = GetSelectedIdent();
			if(!pIdent || !pIdent->m_Filename)
				break;
			CString full = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
			CString dir, stem;
			SplitMatPath(full, dir, stem);
			bool bInst = (cmd == CMD_MAT_NEWINSTANCE);
			CMatNameDlg dlg(bInst ? "New Material Instance" : "Create Copy", "Name of the new material:", stem + (bInst ? "_inst" : "_copy"));
			if(dlg.Run(this) != IDOK)
				break;
			if(!ValidMatName(dlg.m_Text))
			{
				AfxMessageBox("That is not a valid material name.", MB_ICONEXCLAMATION);
				break;
			}
			CString name = dlg.m_Text;
			if(name.GetLength() > 6 && name.Right(6).CompareNoCase(".Mat00") == 0)
				name = name.Left(name.GetLength() - 6);
			CString dest = dir + "\\" + name + ".Mat00";
			if(!CopyFile(full, dest, TRUE))
			{
				AfxMessageBox("Could not create " + dest + "\n(does it already exist?)", MB_ICONEXCLAMATION);
				break;
			}
			SetFileAttributes(dest, FILE_ATTRIBUTE_NORMAL);
			dib_FreeAllTextures();
			UpdateDirectories();
			break;
		}

	case CMD_MAT_EDITSELECTED:
		{
			std::vector<CString> files;
			int iItem = -1;
			while((iItem = m_TextureList.GetNextItem(iItem, LVNI_SELECTED)) != -1)
			{
				DFileIdent *pItem = (DFileIdent*)m_TextureList.GetItemData(iItem);
				if(pItem && pItem->m_Filename)
					files.push_back(CString(pItem->m_Filename));
			}
			for(size_t i = 0; i < files.size(); i++)
			{
				CString full = dfm_GetFullFilename(GetFileMgr(), files[i]);
				CMaterialPropsDlg dlg((LPCTSTR)full, (LPCTSTR)files[i], AfxGetMainWnd());
				if(dlg.RunModal() == IDOK)
					OnMaterialChanged(GetSelectedIdent());
			}
			break;
		}

	case CMD_MAT_WRITABLE:
		{
			int iItem = -1;
			while((iItem = m_TextureList.GetNextItem(iItem, LVNI_SELECTED)) != -1)
			{
				DFileIdent *pItem = (DFileIdent*)m_TextureList.GetItemData(iItem);
				if(!pItem || !pItem->m_Filename)
					continue;
				CString full = dfm_GetFullFilename(GetFileMgr(), pItem->m_Filename);
				DWORD attr = GetFileAttributes(full);
				if(attr != INVALID_FILE_ATTRIBUTES)
					SetFileAttributes(full, attr & ~FILE_ATTRIBUTE_READONLY);
			}
			break;
		}

	case CMD_MAT_RENAME:
		{
			DFileIdent *pIdent = GetSelectedIdent();
			if(!pIdent || !pIdent->m_Filename)
				break;
			CString full = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
			CString dir, stem;
			SplitMatPath(full, dir, stem);
			CMatNameDlg dlg("Rename Material", "New name (levels that use the old name will keep pointing at it):", stem);
			if(dlg.Run(this) != IDOK)
				break;
			if(!ValidMatName(dlg.m_Text))
			{
				AfxMessageBox("That is not a valid material name.", MB_ICONEXCLAMATION);
				break;
			}
			CString name = dlg.m_Text;
			if(name.GetLength() > 6 && name.Right(6).CompareNoCase(".Mat00") == 0)
				name = name.Left(name.GetLength() - 6);
			CString dest = dir + "\\" + name + ".Mat00";
			if(!MoveFile(full, dest))
			{
				AfxMessageBox("Could not rename to " + dest + "\n(does it already exist?)", MB_ICONEXCLAMATION);
				break;
			}
			dib_FreeAllTextures();
			UpdateDirectories();
			break;
		}

	case CMD_MAT_FINDINWORLD:
		// The clicked material is the current texture: select every polygon using it in the active world.
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_POPUP_SELECTTEXTURE, 0);
		break;

	case CMD_MAT_FINDFILE:
		{
			DFileIdent *pIdent = GetSelectedIdent();
			if(pIdent && pIdent->m_Filename)
			{
				CString full = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
				CString args = "/select,\"" + full + "\"";
				ShellExecute(m_hWnd, "open", "explorer.exe", args, NULL, SW_SHOWNORMAL);
			}
			break;
		}
	case CMD_MAT_PALETTE:
		{
			// The palette window belongs to the texture tab.
			CTextureDlg *pTexDlg = GetTextureDlg();
			if(pTexDlg && ::IsWindow(pTexDlg->m_AllTextureDlg.m_hWnd))
			{
				int iItem = -1;
				while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED)) != -1)
				{
					DFileIdent *pItem = (DFileIdent*)m_TextureList.GetItemData(iItem);
					if(pItem)
						pTexDlg->m_AllTextureDlg.AddTextureToPalette(pItem->m_Filename);
				}
			}
			break;
		}
	case CMD_MAT_REFRESH:
		dib_FreeAllTextures();
		UpdateDirectories();
		break;
	}
}
