//------------------------------------------------------------------
//
//	FILE	  : MaterialDlg.h
//
//	PURPOSE	  : The Materials tab of the project bar. It is the texture tab
//	            pointed at .Mat00 files: each material shows up with a preview
//	            of its diffuse map (see MaterialTexture.h), picking one makes it
//	            the "current texture" so it is applied to brush polygons exactly
//	            like a texture, and double clicking opens the Material
//	            Properties editor (MaterialPropsDlg.h).
//
//------------------------------------------------------------------

#ifndef __MATERIALDLG_H__
#define __MATERIALDLG_H__

#include "TextureDlg.h"

class CMaterialDlg : public CTextureDlg
{
public:
	CMaterialDlg();
	virtual ~CMaterialDlg();

	// Opens the Material Properties editor for the selected material.
	BOOL		DoMaterialProperties();

	// Selected material in the list (NULL if none).
	DFileIdent*	GetSelectedIdent();

	// Call after a material file changed on disk so previews and views refresh.
	void		OnMaterialChanged(DFileIdent *pIdent);

	// Overrides.
	virtual void UpdateDirectories();
	void		SetListItemText(int nItem, CString &relativeFilename);

protected:
	void		FixColumnTitle();

	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	afx_msg void OnOpenMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};

#endif // __MATERIALDLG_H__
