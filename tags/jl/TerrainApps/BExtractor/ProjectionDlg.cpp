//
// ProjectionDlg.cpp : implementation file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "stdafx.h"
#include "BExtractor.h"
#include "ProjectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectionDlg dialog


CProjectionDlg::CProjectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectionDlg)
	m_iZone = 0;
	//}}AFX_DATA_INIT
}


void CProjectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectionDlg)
	DDX_Control(pDX, IDC_UTMZONE, m_pcZone);
	DDX_Text(pDX, IDC_UTMZONE, m_iZone);
	DDV_MinMaxInt(pDX, m_iZone, -1, 30);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectionDlg, CDialog)
	//{{AFX_MSG_MAP(CProjectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectionDlg message handlers

BOOL CProjectionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_pcZone.SetFocus();
	m_pcZone.SetSel(0, -1);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}