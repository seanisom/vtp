/////////////////////////////////////////////////////////////////////////////
// Name:        LayerPropDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __LayerPropDlg_H__
#define __LayerPropDlg_H__

#ifdef __GNUG__
    #pragma interface "LayerPropDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LayerPropDlg
//----------------------------------------------------------------------------

class LayerPropDlg: public AutoDialog
{
public:
    // constructors and destructors
    LayerPropDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for LayerPropDlg
	double m_fLeft, m_fTop, m_fRight, m_fBottom;
	wxString m_strText;

private:
    // WDR: member variable declarations for LayerPropDlg
    
private:
    // WDR: handler declarations for LayerPropDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};




#endif