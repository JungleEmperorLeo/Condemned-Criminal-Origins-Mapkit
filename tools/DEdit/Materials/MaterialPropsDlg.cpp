//------------------------------------------------------------------
//
//	FILE	  : MaterialPropsDlg.cpp
//
//	PURPOSE	  : See MaterialPropsDlg.h
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "dedit.h"
#include "edithelpers.h"
#include "d_filemgr.h"
#include "MaterialPropsDlg.h"
#include "MaterialSurfaces.h"
#include "MaterialTexture.h"
#include "DDSLoad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ole2.h>
#include <olectl.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

enum
{
	IDC_MP_INFOGROUP = 9501,
	IDC_MP_INFO,
	IDC_MP_PREVIEW,
	IDC_MP_HELP,
	IDC_MP_SHADERBTN,
	IDC_MP_ROW_MAIN		= 9600,		// + row index (buttons / edits / combos)
	IDC_MP_ROW_SWATCH	= 9700,		// + row index (owner-drawn swatch)
	IDC_MP_ROW_EXTRA	= 9800		// + row index (small "..." button)
};

static const int kMaxRows = 90;

// One row of the dialog.
struct CMaterialPropsDlg::RowCtl
{
	RowCtl() : type(MatFile::PT_FLOAT), paramIndex(-1), infoIndex(-1), isSurface(false), useEdit(false),
		useCombo(false), useButton(false), hasSwatch(false), hasExtra(false), origSel(-1), edited(false),
		nVec(0), thumbW(0), thumbH(0)
	{ vec[0] = vec[1] = vec[2] = vec[3] = 0.0f; }

	CString		name;
	uint32_t	type;
	int			paramIndex;		// index into m_Mat.params, -1 = not stored in the material (default shown)
	int			infoIndex;		// index into m_Shader.params, -1 = none
	bool		isSurface;
	bool		useEdit, useCombo, useButton, hasSwatch, hasExtra;

	CStatic		label;
	CEdit		edit;
	CComboBox	combo;
	CButton		button;
	CStatic		swatch;
	CButton		extra;

	CString		origText;
	int			origSel;
	bool		edited;

	CString		texPath;
	float		vec[4];
	int			nVec;

	int						thumbW, thumbH;
	std::vector<uint8_t>	thumb;		// BGRA
};

BEGIN_MESSAGE_MAP(CMaterialPropsDlg, CDialog)
	ON_WM_DRAWITEM()
	ON_CONTROL_RANGE(BN_CLICKED, IDC_MP_ROW_MAIN, IDC_MP_ROW_MAIN + kMaxRows - 1, OnRowClicked)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_MP_ROW_EXTRA, IDC_MP_ROW_EXTRA + kMaxRows - 1, OnRowExtra)
	ON_BN_CLICKED(IDC_MP_HELP, OnHelpLink)
	ON_BN_CLICKED(IDC_MP_SHADERBTN, OnChangeShader)
END_MESSAGE_MAP()

//--------------------------------------------------------------------
// Small helpers
//--------------------------------------------------------------------

static CString FloatText(float f)
{
	CString s;
	s.Format("%g", (double)f);
	return s;
}

// What the F.E.A.R. editor shows: two decimals, unless that would lose the value.
static CString FloatDisplay(float f)
{
	CString s;
	s.Format("%.2f", (double)f);
	if((float)atof((LPCTSTR)s) != f)
		s.Format("%.9g", (double)f);
	return s;
}

static int ParseNumbers(const char *pText, float *pOut, int nMax)
{
	int n = 0;
	const char *p = pText;
	while(*p && n < nMax)
	{
		while(*p == ' ' || *p == ',' || *p == ';' || *p == '\t')
			p++;
		if(!*p)
			break;
		char *pEnd = 0;
		double d = strtod(p, &pEnd);
		if(pEnd == p)
			return -1;
		pOut[n++] = (float)d;
		p = pEnd;
	}
	while(*p == ' ' || *p == ',' || *p == ';' || *p == '\t')
		p++;
	if(*p)
		return -1;
	return n;
}

static bool IsSurfaceFlags(const char *pName)
{
	return pName && stricmp(pName, "SurfaceFlags") == 0;
}

static std::string RelativeTo(const std::string &root, const std::string &full)
{
	if(full.size() > root.size() + 1 && strnicmp(full.c_str(), root.c_str(), root.size()) == 0 &&
		(full[root.size()] == '\\' || full[root.size()] == '/'))
		return full.substr(root.size() + 1);
	return std::string();
}

static int ToByte(float f)
{
	int b = (int)(f * 255.0f + 0.5f);
	return b < 0 ? 0 : (b > 255 ? 255 : b);
}

//--------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------

CMaterialPropsDlg::CMaterialPropsDlg(const char *pFullPath, const char *pRelName, CWnd *pParent)
	: CDialog()
{
	m_FullPath = pFullPath;
	m_RelName = pRelName;
	m_bHaveShader = false;
	m_bDirty = false;
	m_pParentForModal = pParent;
	m_pPreview = NULL;
}

CMaterialPropsDlg::~CMaterialPropsDlg()
{
	DestroyRows();
	if(m_pPreview)
	{
		m_pPreview->Release();
		m_pPreview = NULL;
	}
}

CString CMaterialPropsDlg::ResourceRoot() const
{
	CString root = dfm_GetBaseDir(GetFileMgr());
	while(!root.IsEmpty() && (root[root.GetLength()-1] == '\\' || root[root.GetLength()-1] == '/'))
		root = root.Left(root.GetLength() - 1);
	// Resolve "..\\" segments so the prefix test against picked files works.
	char full[MAX_PATH];
	if(!root.IsEmpty() && _fullpath(full, (LPCTSTR)root, MAX_PATH))
	{
		root = full;
		while(!root.IsEmpty() && (root[root.GetLength()-1] == '\\' || root[root.GetLength()-1] == '/'))
			root = root.Left(root.GetLength() - 1);
	}
	return root;
}

CRect CMaterialPropsDlg::D(int x, int y, int w, int h)
{
	CRect r(x, y, x + w, y + h);
	MapDialogRect(&r);
	return r;
}

static void PutWord(std::vector<WORD> &t, WORD w) { t.push_back(w); }

static void PutWideString(std::vector<WORD> &t, const char *s)
{
	while(*s)
		t.push_back((WORD)(unsigned char)*s++);
	t.push_back(0);
}

void CMaterialPropsDlg::BuildTemplate()
{
	m_Template.clear();
	DWORD style = DS_MODALFRAME | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	PutWord(m_Template, (WORD)(style & 0xFFFF));
	PutWord(m_Template, (WORD)(style >> 16));
	PutWord(m_Template, 0);
	PutWord(m_Template, 0);
	PutWord(m_Template, 0);		// no controls in the template
	PutWord(m_Template, 0);
	PutWord(m_Template, 0);
	PutWord(m_Template, 320);	// cx (dialog units), fixed up in Rebuild()
	PutWord(m_Template, 260);	// cy
	PutWord(m_Template, 0);		// menu
	PutWord(m_Template, 0);		// class
	PutWideString(m_Template, "Material Properties");
	PutWord(m_Template, 8);
	PutWideString(m_Template, "MS Shell Dlg");
	while(m_Template.size() & 1)
		m_Template.push_back(0);
}

INT_PTR CMaterialPropsDlg::RunModal()
{
	std::string err;
	if(!MatFile::LoadFile((LPCTSTR)m_FullPath, m_Mat, &err))
	{
		CString msg;
		msg.Format("Unable to read the material:\n%s\n\n%s", (LPCTSTR)m_FullPath, err.c_str());
		AfxMessageBox(msg, MB_ICONEXCLAMATION | MB_OK);
		return IDCANCEL;
	}

	BuildTemplate();
	InitModalIndirect((LPCDLGTEMPLATE)&m_Template[0], m_pParentForModal);
	return DoModal();
}

//--------------------------------------------------------------------
// Init
//--------------------------------------------------------------------

BOOL CMaterialPropsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc(0, 0, 10, 10);
	m_grpInfo.Create("Shader Information", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, rc, this, IDC_MP_INFOGROUP);
	m_edInfo.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, rc, this, IDC_MP_INFO);
	m_stPreview.Create("", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rc, this, IDC_MP_PREVIEW);
	m_btnHelp.Create("?", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, rc, this, IDC_MP_HELP);
	m_btnShader.Create("", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, rc, this, IDC_MP_SHADERBTN);
	m_btnOK.Create("OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, rc, this, IDOK);
	m_btnCancel.Create("Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, rc, this, IDCANCEL);

	CWnd *pFixed[] = { &m_grpInfo, &m_edInfo, &m_btnHelp, &m_btnShader, &m_btnOK, &m_btnCancel };
	for(int i = 0; i < (int)(sizeof(pFixed) / sizeof(pFixed[0])); i++)
		SetFixedControlFont(*pFixed[i]);

	CMaterialSurfaces::Get().TryAutoLoad();
	LoadShaderInfo();
	Rebuild();
	CenterWindow();
	return TRUE;
}

void CMaterialPropsDlg::SetFixedControlFont(CWnd &w)
{
	CFont *pFont = GetFont();
	if(pFont)
		w.SetFont(pFont);
}

void CMaterialPropsDlg::LoadShaderInfo()
{
	CString root = ResourceRoot();
	std::string err;
	m_Shader = ShaderInfo();
	m_bHaveShader = ShaderInfo_Load((LPCTSTR)root, m_Mat.shader.c_str(), m_Shader, &err);

	m_btnShader.SetWindowText(m_Mat.shader.c_str());

	CString info;
	if(m_bHaveShader && !m_Shader.description.empty())
		info = m_Shader.description.c_str();
	else if(m_bHaveShader)
		info = "(This shader has no description.)";
	else
		info = "(Shader information is not available for this shader.)";
	info.Replace("\r\n", "\n");
	info.Replace("\n", "\r\n");
	m_edInfo.SetWindowText(info);

	m_btnHelp.EnableWindow((m_bHaveShader && !m_Shader.docLink.empty()) ? TRUE : FALSE);
	LoadPreview();
}

// Looks for a picture next to the shader's documentation page (jpg/bmp/gif).
void CMaterialPropsDlg::LoadPreview()
{
	if(m_pPreview)
	{
		m_pPreview->Release();
		m_pPreview = NULL;
	}
	if(!m_bHaveShader || m_Shader.docLink.empty())
	{
		if(m_stPreview.m_hWnd) m_stPreview.Invalidate();
		return;
	}

	CString root = ResourceRoot();
	CString link = CString(m_Shader.docLink.c_str());
	int slash = link.ReverseFind('\\');
	if(slash < 0) slash = link.ReverseFind('/');
	CString dir = root + "\\" + (slash >= 0 ? link.Left(slash) : CString());

	static const char *masks[] = { "*.jpg", "*.jpeg", "*.bmp", "*.gif" };
	CString found;
	for(int m = 0; m < 4 && found.IsEmpty(); m++)
	{
		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(dir + "\\" + masks[m], &fd);
		if(h != INVALID_HANDLE_VALUE)
		{
			found = dir + "\\" + fd.cFileName;
			FindClose(h);
		}
	}
	if(!found.IsEmpty())
	{
		HANDLE hFile = CreateFile(found, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			unsigned long size = (unsigned long)GetFileSize(hFile, NULL);
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
			if(hMem)
			{
				void *p = GlobalLock(hMem);
				unsigned long got = 0;
				ReadFile(hFile, p, size, (LPDWORD)&got, NULL);
				GlobalUnlock(hMem);
				IStream *pStream = NULL;
				if(got == size && SUCCEEDED(CreateStreamOnHGlobal(hMem, TRUE, &pStream)))
				{
					OleLoadPicture(pStream, size, FALSE, IID_IPicture, (void**)&m_pPreview);
					pStream->Release();
				}
				else
					GlobalFree(hMem);
			}
			CloseHandle(hFile);
		}
	}
	if(m_stPreview.m_hWnd)
		m_stPreview.Invalidate();
}

const ShaderParamInfo* CMaterialPropsDlg::FindInfo(const char *pName) const
{
	if(!m_bHaveShader)
		return 0;
	for(size_t i = 0; i < m_Shader.params.size(); i++)
		if(MatFile::NameEquals(m_Shader.params[i].name, pName))
			return &m_Shader.params[i];
	return 0;
}

//--------------------------------------------------------------------
// Rows
//--------------------------------------------------------------------

void CMaterialPropsDlg::DestroyRows()
{
	for(size_t i = 0; i < m_Rows.size(); i++)
	{
		RowCtl *r = m_Rows[i];
		if(r->label.m_hWnd)  r->label.DestroyWindow();
		if(r->edit.m_hWnd)   r->edit.DestroyWindow();
		if(r->combo.m_hWnd)  r->combo.DestroyWindow();
		if(r->button.m_hWnd) r->button.DestroyWindow();
		if(r->swatch.m_hWnd) r->swatch.DestroyWindow();
		if(r->extra.m_hWnd)  r->extra.DestroyWindow();
		delete r;
	}
	m_Rows.clear();
}

void CMaterialPropsDlg::LoadThumb(RowCtl &r)
{
	r.thumb.clear();
	r.thumbW = r.thumbH = 0;
	if(r.texPath.IsEmpty())
		return;

	std::string full;
	if(!MaterialTexture_ResolveTexture((LPCTSTR)m_FullPath, (LPCTSTR)r.texPath, full))
		return;

	DDS::Image img;
	if(!DDS::LoadFile(full.c_str(), img, 0, 0) || img.mips.empty())
		return;

	// The smallest mip that is still at least 32 pixels wide, else the last one.
	size_t m = 0;
	while(m + 1 < img.mips.size() && img.MipWidth(m + 1) >= 32)
		m++;
	r.thumbW = (int)img.MipWidth(m);
	r.thumbH = (int)img.MipHeight(m);
	r.thumb = img.mips[m];
	if(r.thumb.size() < (size_t)r.thumbW * r.thumbH * 4)
	{
		r.thumb.clear();
		r.thumbW = r.thumbH = 0;
	}
}

void CMaterialPropsDlg::Rebuild()
{
	DestroyRows();

	// Which parameters get a row: the shader's per-material ones, then anything else the material holds.
	struct Item { int paramIndex; int infoIndex; };
	std::vector<Item> items;
	std::vector<bool> used(m_Mat.params.size(), false);

	if(m_bHaveShader)
	{
		for(size_t i = 0; i < m_Shader.params.size(); i++)
		{
			if(!m_Shader.params[i].isInstance)
				continue;
			Item it;
			it.infoIndex = (int)i;
			it.paramIndex = -1;
			for(size_t k = 0; k < m_Mat.params.size(); k++)
				if(MatFile::NameEquals(m_Mat.params[k].name, m_Shader.params[i].name.c_str()))
				{
					it.paramIndex = (int)k;
					used[k] = true;
					break;
				}
			items.push_back(it);
		}
	}
	for(size_t k = 0; k < m_Mat.params.size(); k++)
	{
		if(used[k])
			continue;
		Item it;
		it.paramIndex = (int)k;
		it.infoIndex = -1;
		const ShaderParamInfo *pInfo = FindInfo(m_Mat.params[k].name.c_str());
		if(pInfo)
			for(size_t s = 0; s < m_Shader.params.size(); s++)
				if(&m_Shader.params[s] == pInfo) it.infoIndex = (int)s;
		items.push_back(it);
	}
	if((int)items.size() > kMaxRows)
		items.resize(kMaxRows);

	const int rowH = 17;
	int y = 8;

	for(size_t n = 0; n < items.size(); n++)
	{
		RowCtl *r = new RowCtl;
		m_Rows.push_back(r);

		const MatFile::Param *pParam = (items[n].paramIndex >= 0) ? &m_Mat.params[items[n].paramIndex] : 0;
		const ShaderParamInfo *pInfo = (items[n].infoIndex >= 0) ? &m_Shader.params[items[n].infoIndex] : 0;

		r->paramIndex = items[n].paramIndex;
		r->infoIndex = items[n].infoIndex;
		r->name = pParam ? CString(pParam->name.c_str()) : CString(pInfo->name.c_str());
		r->type = pParam ? pParam->type : pInfo->type;
		r->isSurface = (r->type == MatFile::PT_UINT) && IsSurfaceFlags(r->name);

		UINT idMain = IDC_MP_ROW_MAIN + (UINT)n;
		UINT idSwatch = IDC_MP_ROW_SWATCH + (UINT)n;
		UINT idExtra = IDC_MP_ROW_EXTRA + (UINT)n;

		r->label.Create(r->name, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX, D(4, y + 3, 82, 10), this, (UINT)-1);
		SetFixedControlFont(r->label);

		// Current value: from the material, else the shader's default.
		float f = 0.0f; uint32_t u = 0; float v[4] = { 0, 0, 0, 0 }; CString tex;
		if(pParam)
		{
			f = pParam->f; u = pParam->u; tex = pParam->str.c_str();
			memcpy(v, pParam->v, sizeof(v));
		}
		else if(pInfo && pInfo->hasDefault)
		{
			f = pInfo->defaultVec[0]; u = pInfo->defaultUInt; tex = pInfo->defaultString.c_str();
			memcpy(v, pInfo->defaultVec, sizeof(v));
		}

		switch(r->type)
		{
		case MatFile::PT_FLOAT:
			r->useEdit = true;
			r->origText = FloatDisplay(f);
			break;

		case MatFile::PT_UINT:
			if(r->isSurface && CMaterialSurfaces::Get().IsLoaded())
			{
				r->useCombo = true;
				r->combo.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST, D(90, y, 180, 140), this, idMain);
				SetFixedControlFont(r->combo);
				const std::vector<MaterialSurface> &list = CMaterialSurfaces::Get().Surfaces();
				int sel = -1;
				for(size_t i = 0; i < list.size(); i++)
				{
					int idx = r->combo.AddString(list[i].name.c_str());
					r->combo.SetItemData(idx, (DWORD_PTR)list[i].id);
					if(list[i].id == u)
						sel = idx;
				}
				if(sel < 0)
				{
					CString s;
					s.Format("(value %u)", (unsigned)u);
					r->combo.InsertString(0, s);
					r->combo.SetItemData(0, (DWORD_PTR)u);
					sel = 0;
				}
				r->combo.SetCurSel(sel);
				r->origSel = sel;
			}
			else
			{
				r->useEdit = true;
				r->origText.Format("%u", (unsigned)u);
				if(r->isSurface)
					r->hasExtra = true;		// "..." to locate GameServer.dll for the names
			}
			break;

		case MatFile::PT_TEXTURE:
			r->useButton = true;
			r->hasSwatch = true;
			r->texPath = tex;
			LoadThumb(*r);
			break;

		case MatFile::PT_VECTOR3:
		case MatFile::PT_VECTOR4:
			r->useButton = true;
			r->hasSwatch = true;
			r->nVec = (r->type == MatFile::PT_VECTOR3) ? 3 : 4;
			memcpy(r->vec, v, sizeof(v));
			break;
		}

		if(r->useEdit)
		{
			int w = r->hasExtra ? 156 : 194;
			r->edit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, D(90, y, w, 14), this, idMain);
			SetFixedControlFont(r->edit);
			r->edit.SetWindowText(r->origText);
		}
		if(r->hasExtra)
		{
			r->extra.Create("...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, D(250, y, 34, 14), this, idExtra);
			SetFixedControlFont(r->extra);
		}
		if(r->useButton)
		{
			CString text;
			if(r->type == MatFile::PT_TEXTURE)
				text = r->texPath;
			else
			{
				for(int i = 0; i < r->nVec; i++)
				{
					if(i) text += ", ";
					text += FloatDisplay(r->vec[i]);
				}
			}
			r->button.Create(text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, D(90, y, 172, 14), this, idMain);
			SetFixedControlFont(r->button);
		}
		if(r->hasSwatch)
			r->swatch.Create("", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, D(266, y - 1, 18, 16), this, idSwatch);

		// Hint with the parameter's description.
		if(pInfo && !pInfo->description.empty())
		{
			// (shown as the label's tooltip text via the window text of the label would change its look;
			//  the description is included in the Shader Information instead when there is room)
		}

		y += rowH;
	}

	// Shader Information group, shader button, OK / Cancel.
	y += 4;
	int yGroup = y;
	m_grpInfo.MoveWindow(D(6, yGroup, 278, 74));
	m_edInfo.MoveWindow(D(12, yGroup + 11, 190, 58));
	m_stPreview.MoveWindow(D(208, yGroup + 11, 54, 46));
	m_btnHelp.MoveWindow(D(266, yGroup + 53, 14, 14));
	y = yGroup + 74 + 8;

	m_btnShader.MoveWindow(D(6, y, 150, 15));
	m_btnOK.MoveWindow(D(164, y, 58, 15));
	m_btnCancel.MoveWindow(D(226, y, 58, 15));
	y += 15 + 8;

	// Resize the dialog to its content.
	CRect client = D(0, 0, 290, y);
	CRect wr(0, 0, client.Width(), client.Height());
	AdjustWindowRectEx(&wr, GetStyle(), FALSE, GetExStyle());
	SetWindowPos(NULL, 0, 0, wr.Width(), wr.Height(), SWP_NOMOVE | SWP_NOZORDER);
	Invalidate();
}

//--------------------------------------------------------------------
// Drawing (thumbnails, colour swatches, shader preview)
//--------------------------------------------------------------------

void CMaterialPropsDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	if(nIDCtl == IDC_MP_PREVIEW)
	{
		CDC *pDC = CDC::FromHandle(lpDIS->hDC);
		CRect rc(lpDIS->rcItem);
		pDC->FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
		if(m_pPreview)
		{
			OLE_XSIZE_HIMETRIC cx = 0; OLE_YSIZE_HIMETRIC cy = 0;
			m_pPreview->get_Width(&cx);
			m_pPreview->get_Height(&cy);
			m_pPreview->Render(lpDIS->hDC, rc.left, rc.top, rc.Width(), rc.Height(), 0, cy, cx, -cy, NULL);
		}
		return;
	}

	if(nIDCtl >= IDC_MP_ROW_SWATCH && nIDCtl < IDC_MP_ROW_SWATCH + kMaxRows)
	{
		size_t n = (size_t)(nIDCtl - IDC_MP_ROW_SWATCH);
		if(n >= m_Rows.size())
			return;
		RowCtl *r = m_Rows[n];
		CDC *pDC = CDC::FromHandle(lpDIS->hDC);
		CRect rc(lpDIS->rcItem);

		if(r->type == MatFile::PT_TEXTURE)
		{
			pDC->FillSolidRect(rc, RGB(0, 0, 0));
			if(!r->thumb.empty())
			{
				BITMAPINFO bi;
				memset(&bi, 0, sizeof(bi));
				bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biWidth = r->thumbW;
				bi.bmiHeader.biHeight = -r->thumbH;		// top-down
				bi.bmiHeader.biPlanes = 1;
				bi.bmiHeader.biBitCount = 32;
				bi.bmiHeader.biCompression = BI_RGB;
				int old = SetStretchBltMode(lpDIS->hDC, HALFTONE);
				SetBrushOrgEx(lpDIS->hDC, 0, 0, NULL);
				StretchDIBits(lpDIS->hDC, rc.left + 1, rc.top + 1, rc.Width() - 2, rc.Height() - 2,
					0, 0, r->thumbW, r->thumbH, &r->thumb[0], &bi, DIB_RGB_COLORS, SRCCOPY);
				SetStretchBltMode(lpDIS->hDC, old);
			}
		}
		else
		{
			pDC->FillSolidRect(rc, RGB(ToByte(r->vec[0]), ToByte(r->vec[1]), ToByte(r->vec[2])));
		}
		pDC->DrawEdge(rc, EDGE_SUNKEN, BF_RECT);
		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDIS);
}

//--------------------------------------------------------------------
// Row buttons
//--------------------------------------------------------------------

void CMaterialPropsDlg::OnRowClicked(UINT nID)
{
	size_t n = (size_t)(nID - IDC_MP_ROW_MAIN);
	if(n >= m_Rows.size() || !m_Rows[n]->useButton)
		return;
	RowCtl *r = m_Rows[n];

	if(r->type == MatFile::PT_TEXTURE)
	{
		CString root = ResourceRoot();
		CString startDir = root;
		int slash = r->texPath.ReverseFind('\\');
		if(slash > 0)
			startDir = root + "\\" + r->texPath.Left(slash);

		CFileDialog dlg(TRUE, "dds", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
			"DDS textures (*.dds)|*.dds|All files (*.*)|*.*||", this);
		dlg.m_ofn.lpstrInitialDir = (LPCTSTR)startDir;
		if(dlg.DoModal() != IDOK)
			return;

		std::string rel = RelativeTo((LPCTSTR)root, (LPCTSTR)dlg.GetPathName());
		if(rel.empty())
		{
			AfxMessageBox("The texture has to be inside the project folder:\n" + root, MB_ICONEXCLAMATION);
			return;
		}
		r->texPath = rel.c_str();
		r->edited = true;
		r->button.SetWindowText(r->texPath);
		LoadThumb(*r);
		r->swatch.Invalidate();
	}
	else
	{
		COLORREF cur = RGB(ToByte(r->vec[0]), ToByte(r->vec[1]), ToByte(r->vec[2]));
		CColorDialog dlg(cur, CC_FULLOPEN | CC_RGBINIT, this);
		if(dlg.DoModal() != IDOK)
			return;
		COLORREF c = dlg.GetColor();
		r->vec[0] = GetRValue(c) / 255.0f;
		r->vec[1] = GetGValue(c) / 255.0f;
		r->vec[2] = GetBValue(c) / 255.0f;
		r->edited = true;

		CString text;
		for(int i = 0; i < r->nVec; i++)
		{
			if(i) text += ", ";
			text += FloatDisplay(r->vec[i]);
		}
		r->button.SetWindowText(text);
		r->swatch.Invalidate();
	}
}

void CMaterialPropsDlg::OnRowExtra(UINT nID)
{
	size_t n = (size_t)(nID - IDC_MP_ROW_EXTRA);
	if(n >= m_Rows.size())
		return;
	if(m_Rows[n]->isSurface)
	{
		const std::string &le = CMaterialSurfaces::Get().m_LoadError;
		if(!le.empty())
			AfxMessageBox(CString("The game's own GameServer.dll could not give the surface names:\n") + le.c_str() + "\n\nYou can pick a GameServer.dll by hand next.", MB_ICONINFORMATION);
		OnSurfaceNames();
	}
}

void CMaterialPropsDlg::OnSurfaceNames()
{
	CFileDialog dlg(TRUE, "dll", "GameServer.dll", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		"GameServer.dll|GameServer.dll|DLL files (*.dll)|*.dll||", this);
	dlg.m_ofn.lpstrTitle = "Locate the game's GameServer.dll (for the surface names)";
	if(dlg.DoModal() != IDOK)
		return;

	std::string err;
	if(!CMaterialSurfaces::Get().Load((LPCTSTR)dlg.GetPathName(), &err))
	{
		CString msg;
		msg.Format("Could not get surface names from that file: %s.\n\nSurfaceFlags stays a plain number.", err.c_str());
		AfxMessageBox(msg, MB_ICONEXCLAMATION);
		return;
	}
	AfxGetApp()->WriteProfileString("Materials", "GameServerDll", dlg.GetPathName());

	if(CommitRows())
		Rebuild();
}

//--------------------------------------------------------------------
// Writing the rows back into the material
//--------------------------------------------------------------------

bool CMaterialPropsDlg::CommitRows()
{
	for(size_t n = 0; n < m_Rows.size(); n++)
	{
		RowCtl *r = m_Rows[n];
		const char *pName = (LPCTSTR)r->name;

		if(r->useCombo)
		{
			int sel = r->combo.GetCurSel();
			if(sel >= 0 && sel != r->origSel)
			{
				m_Mat.SetUInt(pName, (uint32_t)r->combo.GetItemData(sel));
				m_bDirty = true;
			}
			continue;
		}

		if(r->useEdit)
		{
			CString text;
			r->edit.GetWindowText(text);
			text.Trim();
			if(text == r->origText)
				continue;

			if(r->type == MatFile::PT_FLOAT)
			{
				float v[1];
				if(ParseNumbers(text, v, 1) != 1)
				{
					AfxMessageBox("\"" + r->name + "\": please enter a single number.", MB_ICONEXCLAMATION);
					r->edit.SetFocus();
					return false;
				}
				m_Mat.SetFloat(pName, v[0]);
			}
			else	// PT_UINT
			{
				char *pEnd = 0;
				unsigned long v = strtoul((LPCTSTR)text, &pEnd, 0);
				if(text.IsEmpty() || (pEnd && *pEnd))
				{
					AfxMessageBox("\"" + r->name + "\": please enter a whole number (decimal, or hexadecimal like 0x10).", MB_ICONEXCLAMATION);
					r->edit.SetFocus();
					return false;
				}
				m_Mat.SetUInt(pName, (uint32_t)v);
			}
			m_bDirty = true;
			continue;
		}

		if(r->useButton && r->edited)
		{
			if(r->type == MatFile::PT_TEXTURE)
				m_Mat.SetTexture(pName, (LPCTSTR)r->texPath);
			else if(r->type == MatFile::PT_VECTOR3)
				m_Mat.SetVector3(pName, r->vec[0], r->vec[1], r->vec[2]);
			else
				m_Mat.SetVector4(pName, r->vec[0], r->vec[1], r->vec[2], r->vec[3]);
			m_bDirty = true;
		}
	}
	return true;
}

//--------------------------------------------------------------------
// Buttons
//--------------------------------------------------------------------

void CMaterialPropsDlg::OnHelpLink()
{
	if(!m_bHaveShader || m_Shader.docLink.empty())
		return;
	CString path = ResourceRoot() + "\\" + CString(m_Shader.docLink.c_str());
	if(GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		AfxMessageBox("The shader's documentation page was not found:\n" + path, MB_ICONINFORMATION);
		return;
	}
	ShellExecute(m_hWnd, "open", path, NULL, NULL, SW_SHOWNORMAL);
}

void CMaterialPropsDlg::OnChangeShader()
{
	if(!CommitRows())
		return;

	CString root = ResourceRoot();
	CString startDir = root + "\\shaders";
	if(GetFileAttributes(startDir) == INVALID_FILE_ATTRIBUTES)
		startDir = root;

	CFileDialog dlg(TRUE, "fx", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		"Shaders (*.fx;*.fxo)|*.fx;*.fxo|All files (*.*)|*.*||", this);
	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)startDir;
	if(dlg.DoModal() != IDOK)
		return;

	std::string rel = RelativeTo((LPCTSTR)root, (LPCTSTR)dlg.GetPathName());
	if(rel.empty())
	{
		AfxMessageBox("The shader has to be inside the project folder:\n" + root, MB_ICONEXCLAMATION);
		return;
	}
	size_t n = rel.size();
	if(n > 4 && stricmp(rel.c_str() + n - 4, ".fxo") == 0)
		rel = rel.substr(0, n - 1);

	m_Mat.shader = rel;
	m_bDirty = true;
	LoadShaderInfo();
	Rebuild();
}

void CMaterialPropsDlg::OnOK()
{
	if(!CommitRows())
		return;

	if(!m_bDirty)
	{
		CDialog::OnCancel();
		return;
	}

	DWORD attr = GetFileAttributes(m_FullPath);
	if(attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_READONLY))
	{
		if(AfxMessageBox("This material file is read-only. Make it writable and save?", MB_YESNO | MB_ICONQUESTION) != IDYES)
			return;
		SetFileAttributes(m_FullPath, attr & ~FILE_ATTRIBUTE_READONLY);
	}

	// Keep the very first version of the file next to it as .bak.
	if(attr != INVALID_FILE_ATTRIBUTES)
		CopyFile(m_FullPath, m_FullPath + ".bak", TRUE);

	std::string err;
	if(!MatFile::SaveFile((LPCTSTR)m_FullPath, m_Mat, &err))
	{
		CString msg;
		msg.Format("Unable to save the material:\n%s\n\n%s", (LPCTSTR)m_FullPath, err.c_str());
		AfxMessageBox(msg, MB_ICONEXCLAMATION);
		return;
	}

	CDialog::OnOK();
}

void CMaterialPropsDlg::OnCancel()
{
	CDialog::OnCancel();
}
