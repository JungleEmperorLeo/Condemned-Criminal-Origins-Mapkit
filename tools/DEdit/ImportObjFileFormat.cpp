// ImportObjFileFormat.cpp : implementation file
//

// #include "stdafx.h"
#include "bdefs.h"
#include "dedit.h"
#include "ImportObjFileFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportObjFileFormat dialog


CImportObjFileFormat::CImportObjFileFormat(CWnd* pParent /*=NULL*/)
	: CDialog(CImportObjFileFormat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportObjFileFormat)
	m_reverse_normals = FALSE;
	m_fromBlender = TRUE;
	m_from3DSMax = FALSE;
	m_fromMaya = FALSE;
	m_fromSoftImage = FALSE;
	//}}AFX_DATA_INIT
}


void CImportObjFileFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportObjFileFormat)
	DDX_Check(pDX, IDC_IMPORT_OBJ_REVERSE_CB, m_reverse_normals);
	DDX_Check(pDX, IDC_IMPORT_OBJ_FROM_BLENDER_CHECK, m_fromBlender);
	DDX_Check(pDX, IDC_IMPORT_OBJ_FROM_3DSMAX_CHECK, m_from3DSMax);
	DDX_Check(pDX, IDC_IMPORT_OBJ_FROM_MAYA_CHECK, m_fromMaya);
	DDX_Check(pDX, IDC_IMPORT_OBJ_FROM_SOFTIMAGE_CHECK, m_fromSoftImage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportObjFileFormat, CDialog)
	//{{AFX_MSG_MAP(CImportObjFileFormat)
	ON_BN_CLICKED(IDC_IMPORT_OBJ_REVERSE_CB, OnImportObjReverseCb)
	ON_BN_CLICKED(IDC_IMPORT_OBJ_FROM_BLENDER_CHECK, OnImportObjFromBlenderCheck)
	ON_BN_CLICKED(IDC_IMPORT_OBJ_FROM_3DSMAX_CHECK, OnImportObjFrom3dsmaxCheck)
	ON_BN_CLICKED(IDC_IMPORT_OBJ_FROM_MAYA_CHECK, OnImportObjFromMayaCheck)
	ON_BN_CLICKED(IDC_IMPORT_OBJ_FROM_SOFTIMAGE_CHECK, OnImportObjFromSoftimageCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportObjFileFormat message handlers

void CImportObjFileFormat::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CImportObjFileFormat::OnImportObjReverseCb() 
{
	// TODO: Add your control notification handler code here
	
}

void CImportObjFileFormat::SelectSource(UINT id)
{
	m_fromBlender = (id == IDC_IMPORT_OBJ_FROM_BLENDER_CHECK);
	m_from3DSMax = (id == IDC_IMPORT_OBJ_FROM_3DSMAX_CHECK);
	m_fromMaya = (id == IDC_IMPORT_OBJ_FROM_MAYA_CHECK);
	m_fromSoftImage = (id == IDC_IMPORT_OBJ_FROM_SOFTIMAGE_CHECK);
	SetCheckBox(IDC_IMPORT_OBJ_FROM_BLENDER_CHECK, m_fromBlender);
	SetCheckBox(IDC_IMPORT_OBJ_FROM_3DSMAX_CHECK, m_from3DSMax);
	SetCheckBox(IDC_IMPORT_OBJ_FROM_MAYA_CHECK, m_fromMaya);
	SetCheckBox(IDC_IMPORT_OBJ_FROM_SOFTIMAGE_CHECK, m_fromSoftImage);
}

void CImportObjFileFormat::OnImportObjFromBlenderCheck()	{ SelectSource(IDC_IMPORT_OBJ_FROM_BLENDER_CHECK); }
void CImportObjFileFormat::OnImportObjFrom3dsmaxCheck()		{ SelectSource(IDC_IMPORT_OBJ_FROM_3DSMAX_CHECK); }
void CImportObjFileFormat::OnImportObjFromMayaCheck()		{ SelectSource(IDC_IMPORT_OBJ_FROM_MAYA_CHECK); }
void CImportObjFileFormat::OnImportObjFromSoftimageCheck()	{ SelectSource(IDC_IMPORT_OBJ_FROM_SOFTIMAGE_CHECK); }

void CImportObjFileFormat::SetCheckBox(UINT id, BOOL bCheck)
{
	CButton *pButton;

	pButton = (CButton*)GetDlgItem(id);
	if(pButton)
		pButton->SetCheck(bCheck);
}
