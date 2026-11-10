#include "bdefs.h"
#include "..\dedit.h"
#include "proppagecolorschemes.h"
#include "colorschemes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPropPageColorSchemes, CPropertyPage)

CPropPageColorSchemes::CPropPageColorSchemes() : CPropertyPage(CPropPageColorSchemes::IDD)
{
	m_bCycle	= FALSE;
	m_nSeconds	= 30;
}

CPropPageColorSchemes::~CPropPageColorSchemes()
{
}

void CPropPageColorSchemes::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COLOR_SCHEME, m_comboScheme);
	DDX_Check(pDX, IDC_CHECK_CYCLE_SCHEMES, m_bCycle);
	DDX_Text(pDX, IDC_EDIT_CYCLE_SECONDS, m_nSeconds);
}

BEGIN_MESSAGE_MAP(CPropPageColorSchemes, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_SCHEME, OnApplyScheme)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_SCHEME, OnNextScheme)
	ON_BN_CLICKED(IDC_CHECK_CYCLE_SCHEMES, OnCycleChanged)
	ON_EN_KILLFOCUS(IDC_EDIT_CYCLE_SECONDS, OnCycleChanged)
END_MESSAGE_MAP()

BOOL CPropPageColorSchemes::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	for(int i=0; i < ColorSchemes::Count(); i++)
		m_comboScheme.AddString(ColorSchemes::Name(i));
	m_comboScheme.SetCurSel(ColorSchemes::Current());

	m_bCycle	= ColorSchemes::IsCycling() ? TRUE : FALSE;
	m_nSeconds	= ColorSchemes::GetCycleSeconds();
	UpdateData(FALSE);

	return TRUE;
}

void CPropPageColorSchemes::OnApplyScheme()
{
	int nSel = m_comboScheme.GetCurSel();
	if(nSel != CB_ERR)
		ColorSchemes::Apply(nSel);
}

void CPropPageColorSchemes::OnNextScheme()
{
	ColorSchemes::Next();
	m_comboScheme.SetCurSel(ColorSchemes::Current());
}

void CPropPageColorSchemes::OnCycleChanged()
{
	if(!UpdateData(TRUE))
		return;
	ColorSchemes::SetCycle(m_bCycle ? true : false, m_nSeconds);
	m_nSeconds = ColorSchemes::GetCycleSeconds();
	UpdateData(FALSE);
}
