//------------------------------------------------------------------
//
//	FILE	  : MaterialPropsDlg.h
//
//	PURPOSE	  : The Material Properties editor, laid out like the F.E.A.R. /
//	            Condemned WorldEdit one: one labelled row per shader parameter
//	            (number box, surface-name drop-down, texture button with a
//	            thumbnail, colour button), a "Shader Information" box with a
//	            preview and "?" help button, the shader button, OK and Cancel.
//	            OK writes the .Mat00 file.
//
//	            The dialog is built in code (no resource template).
//
//------------------------------------------------------------------

#ifndef __MATERIALPROPSDLG_H__
#define __MATERIALPROPSDLG_H__

#include <vector>
#include <string>
#include "MaterialFile.h"
#include "MaterialShaderInfo.h"

struct IPicture;

class CMaterialPropsDlg : public CDialog
{
public:
	// pFullPath: the .Mat00 on disk. pRelName: its name inside the project.
	CMaterialPropsDlg(const char *pFullPath, const char *pRelName, CWnd *pParent = NULL);
	virtual ~CMaterialPropsDlg();

	// Runs the dialog. Returns IDOK if the material was saved.
	INT_PTR	RunModal();

protected:
	virtual BOOL	OnInitDialog();
	virtual void	OnOK();
	virtual void	OnCancel();

	afx_msg void	OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	afx_msg void	OnRowClicked(UINT nID);
	afx_msg void	OnRowExtra(UINT nID);
	afx_msg void	OnHelpLink();
	afx_msg void	OnChangeShader();
	DECLARE_MESSAGE_MAP()

private:
	struct RowCtl;

	void	BuildTemplate();
	void	LoadShaderInfo();
	void	LoadPreview();
	void	Rebuild();				// (re)creates every row from m_Mat / m_Shader and lays the dialog out
	void	DestroyRows();
	void	LoadThumb(RowCtl &r);
	bool	CommitRows();			// false (after telling the user) if a value can't be understood
	void	SetFixedControlFont(CWnd &w);
	CRect	D(int x, int y, int w, int h);	// dialog units -> pixels
	CString	ResourceRoot() const;
	const ShaderParamInfo* FindInfo(const char *pName) const;
	void	OnSurfaceNames();

	CString		m_FullPath;
	CString		m_RelName;

	MatFile::Material	m_Mat;
	ShaderInfo			m_Shader;
	bool				m_bHaveShader;
	bool				m_bDirty;

	std::vector<RowCtl*>	m_Rows;

	std::vector<WORD>	m_Template;
	CWnd				*m_pParentForModal;
	IPicture			*m_pPreview;

	// Fixed controls.
	CButton		m_grpInfo;
	CEdit		m_edInfo;
	CStatic		m_stPreview;
	CButton		m_btnHelp;
	CButton		m_btnShader;
	CButton		m_btnOK;
	CButton		m_btnCancel;
};

#endif // __MATERIALPROPSDLG_H__
