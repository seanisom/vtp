//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: cmanager.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_cmanager_H__
#define __WDR_cmanager_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "cmanager_wdr.h"
#endif

// Include wxWidgets' headers

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/image.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/toolbar.h>

// Declare window functions

const int ID_SCENETREE = 10000;
const int ID_ENABLED = 10001;
const int ID_ZOOMTO = 10002;
const int ID_REFRESH = 10003;
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TEXT = 10004;
const int ID_ITEM = 10005;
const int ID_TYPECHOICE = 10006;
const int ID_SUBTYPECHOICE = 10007;
const int ID_ADDTAG = 10008;
const int ID_REMOVETAG = 10009;
const int ID_EDITTAG = 10010;
const int ID_TAGLIST = 10011;
wxSizer *PropDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_FILENAME = 10012;
const int ID_DISTANCE = 10013;
const int ID_SCALE = 10014;
const int ID_STATUS = 10015;
wxSizer *ModelDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TAGNAME = 10016;
const int ID_TAGTEXT = 10017;
wxSizer *TagDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_LIGHT = 10018;
const int ID_AMBIENT = 10019;
const int ID_DIFFUSE = 10020;
const int ID_DIRX = 10021;
const int ID_DIRY = 10022;
const int ID_DIRZ = 10023;
wxSizer *LightDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

const int ID_BM_AXES = 10024;
const int ID_BM_CONTENTS_OPEN = 10025;
const int ID_BM_MODEL_ADD = 10026;
const int ID_BM_ITEM_NEW = 10027;
const int ID_BM_ITEM_REMOVE = 10028;
const int ID_BM_MODEL_REMOVE = 10029;
const int ID_BM_RULERS = 10030;
const int ID_BM_PROPERTIES = 10031;
const int ID_BM_WIRE = 10032;
wxBitmap MyBitmapsFunc( size_t index );

#endif

// End of generated file