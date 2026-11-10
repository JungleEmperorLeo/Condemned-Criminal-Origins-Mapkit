#ifndef __PROPPAGECOLORSCHEMES_H__
#define __PROPPAGECOLORSCHEMES_H__

#if _MSC_VER > 1000
#pragma once
#endif

// Options page: pick a colour scheme, or let the editor cycle through all of them.
class CPropPageColorSchemes : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageColorSchemes)

public:
	CPropPageColorSchemes();
	~CPropPageColorSchemes();

	enum { IDD = IDD_PROPPAGE_COLORSCHEMES };

	CComboBox	m_comboScheme;
	BOOL		m_bCycle;
	int			m_nSeconds;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	afx_msg void OnApplyScheme();
	afx_msg void OnNextScheme();
	afx_msg void OnCycleChanged();

	DECLARE_MESSAGE_MAP()

	void UpdateCycle();
};

#endif
