//
// Name:        StartupDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
    #pragma implementation "StartupDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#if defined(UNIX)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <GL/glx.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"

#include "StartupDlg.h"
#include "../TerrainSceneWP.h"
#include "../Options.h"
#include "vtdata/boost/directory.h"

#include "app.h"
#include "TParamsDlg.h"

//
// This function is used to find all files in a given directory,
// and if they match a wildcard, add them to a combo box.
//
void AddFilenamesToComboBox(wxComboBox *box, const char *directory,
                            const char *wildcard, int omit_chars)
{
    using namespace boost::filesystem;

    for (dir_it it((const char *)directory); it != dir_it(); ++it)
    {
        if (get<is_hidden>(it) || get<is_directory>(it))
            continue;

        std::string name1 = *it;
        wxString name = name1.c_str();
        if (name.Matches(wildcard))
        {
            if (omit_chars)
                box->Append(name.Left(name.Length()-omit_chars));
            else
                box->Append(name);
        }
    }
}

//
// Helper: find the largest texture size supported by OpenGL
//
#ifdef WIN32
static void ShowOGLInfo(HDC hdc)
#else
static void ShowOGLInfo()
#endif
{
#if defined(WIN32)
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
        1,                      // version number 
        PFD_DRAW_TO_WINDOW |    // support window 
        PFD_SUPPORT_OPENGL |    // support OpenGL 
        PFD_DOUBLEBUFFER,       // double buffered 
        PFD_TYPE_RGBA,          // RGBA type 
        24,                     // 24-bit color depth 
        0, 0, 0, 0, 0, 0,       // color bits ignored 
        0, 0, 0,                // no alpha buffer 
        0, 0, 0, 0,             // accum bits ignored 
        32, 0, 0,               // 32-bit z-buffer 
        PFD_MAIN_PLANE,         // main layer
        0,                      // reserved 
        0, 0, 0                 // layer masks ignored
    };
    int  iPixelFormat;

    // get the best available match of pixel format for the device context  
    iPixelFormat = ChoosePixelFormat(hdc, &pfd);
    // make that the pixel format of the device context 
    SetPixelFormat(hdc, iPixelFormat, &pfd);

    HGLRC device = wglCreateContext(hdc);
    if (device == NULL)
    {
        DWORD lasterror = GetLastError();
        // 2000 The pixel format is invalid.  ERROR_INVALID_PIXEL_FORMAT 
    }
    wglMakeCurrent(hdc, device);
#elif defined(UNIX)
    static int dblBuf[] =  {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
                            GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
                            GLX_DOUBLEBUFFER, None};
    Display *dpy;
    Window win;
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    GLXContext cx;
    XEvent event;
    Bool needRedraw = False, recalcModelView = True;
    int dummy;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) 
        wxFatalError( "could not open display" );

    if (!glXQueryExtension(dpy, &dummy, &dummy))
        wxFatalError( "X server has no OpenGL GLX extension" );

    vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
    if (vi == NULL)
        wxFatalError( "no RGB visual with double and depth buffer" );
    if (vi->c_class != TrueColor)
        wxFatalError( "TrueColor visual required for this program" );

    cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
                           vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
    win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
                        0, 0, 300, 300, 0, vi->depth,
                        InputOutput, vi->visual,
                        CWBorderPixel | CWColormap | CWEventMask, &swa);

    XSetStandardProperties(dpy, win, "test", "test",
                           None, NULL, 0, NULL);

    cx = glXCreateContext(dpy, vi, None, True);
    if (cx == NULL)
        wxFatalError( "could not create rendering context" );

    glXMakeCurrent(dpy, win, cx);
#else
#  error "I don't know this platform."
#endif

    GLint value;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    GLenum error = glGetError();
    if (error == GL_INVALID_OPERATION)
        value = 0;

    wxString str;
    str.Printf("OpenGL Version: %s\nVendor: %s\nRenderer: %s\n"
        "Maximum Texture Dimension: %d\nExtensions: %s",
        glGetString(GL_VERSION), glGetString(GL_VENDOR),
        glGetString(GL_RENDERER), value, glGetString(GL_EXTENSIONS));

    wxDialog dlg(NULL, -1, "OpenGL Info", wxDefaultPosition);
    TextDialogFunc(&dlg, TRUE);
    wxTextCtrl* pText = (wxTextCtrl*) dlg.FindWindow( ID_TEXT );
    pText->SetValue(str);

    dlg.ShowModal();

#ifdef WIN32
    wglDeleteContext(device);
#elif defined(UNIX)
    glXDestroyContext(dpy, cx);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
#endif
}

// WDR: class implementations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

void StartupDlg::GetOptionsFrom(EnviroOptions &opt)
{
    m_strDataPath = opt.m_strDataPath;
    m_bStartEarth = (opt.m_bEarthView == TRUE);
    m_bStartTerrain = !opt.m_bEarthView;
    m_strImage = opt.m_strImage;
    m_strTName = opt.m_strInitTerrain;
    m_bFullscreen = (opt.m_bFullscreen == TRUE);
    m_bGravity = (opt.m_bGravity == TRUE);
    m_bHtmlpane = (opt.m_bHtmlpane == TRUE);
    m_bFloatingToolbar = (opt.m_bFloatingToolbar == TRUE);
    m_bSound = (opt.m_bSound == TRUE);
    m_bVCursor = (opt.m_bVCursor == TRUE);
    m_bSpeedTest = (opt.m_bSpeedTest == TRUE);
    m_bQuakeNavigation = (opt.m_bQuakeNavigation == TRUE);
    m_fPlantScale = opt.m_fPlantScale;
    m_bShadows = (opt.m_bShadows == TRUE);
}

void StartupDlg::PutOptionsTo(EnviroOptions &opt)
{
    opt.m_strDataPath = m_strDataPath;
    opt.m_bEarthView = (m_bStartEarth == 1);
    opt.m_strImage = m_strImage;
    opt.m_strInitTerrain = m_strTName;
    opt.m_bFullscreen = m_bFullscreen;
    opt.m_bGravity = m_bGravity;
    opt.m_bHtmlpane = m_bHtmlpane;
    opt.m_bFloatingToolbar = m_bFloatingToolbar;
    opt.m_bSound = m_bSound;
    opt.m_bVCursor = m_bVCursor;
    opt.m_bSpeedTest = m_bSpeedTest;
    opt.m_bQuakeNavigation = m_bQuakeNavigation;
    opt.m_fPlantScale = m_fPlantScale;
    opt.m_bShadows = m_bShadows;
}

void StartupDlg::UpdateState()
{
    m_psImage->Enable(m_bStartEarth);
    m_pImage->Enable(m_bStartEarth);
    m_pTName->Enable(!m_bStartEarth);
    m_pTSelect->Enable(!m_bStartEarth);
}

// WDR: event table for StartupDlg

BEGIN_EVENT_TABLE(StartupDlg,AutoDialog)
    EVT_BUTTON( ID_TSELECT, StartupDlg::OnSelectTerrain )
    EVT_BUTTON( wxID_OK, StartupDlg::OnOK )
    EVT_BUTTON( ID_OPENGL, StartupDlg::OnOpenGLInfo )
    EVT_RADIOBUTTON( ID_EARTHVIEW, StartupDlg::OnEarthView )
    EVT_RADIOBUTTON( ID_TERRAIN, StartupDlg::OnTerrain )
    EVT_BUTTON( ID_EDITPROP, StartupDlg::OnEditProp )
END_EVENT_TABLE()

StartupDlg::StartupDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    StartupDialogFunc( this, TRUE ); 
}

void StartupDlg::EditParameters(const char *filename) 
{
    TParamsDlg dlg(this, -1, "Terrain Creation Parameters", wxDefaultPosition);

    TParams Params;
    dlg.SetPath(m_strDataPath);
    if (Params.LoadFromFile(filename))
        dlg.SetParams(Params);

    dlg.CenterOnParent();
    int result = dlg.ShowModal();
    if (result == wxID_OK)
    {
        dlg.GetParams(Params);
        if (!Params.SaveToFile(filename))
        {
            wxString str;
            str.Printf("Couldn't save to file %s.\n"
                       "Please make sure the file is not read-only.", filename);
            wxMessageBox(str);
        }
    }
}

// WDR: handler implementations for StartupDlg

#if 0
void StartupDlg::OnSelectDataPath( wxCommandEvent &event )
{
    wxDirDialog dlg(this, "Please indicate your data directory", m_strDataPath);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_strDataPath = dlg.GetPath();
#if WIN32
		wxString path_separator = "\\";
#else
		wxString path_separator = "/";
#endif
		m_strDataPath += path_separator;
		TransferDataToWindow();
	}
}
#endif

void StartupDlg::OnEditProp( wxCommandEvent &event )
{
    vtTerrain *pTerr = GetTerrainScene().FindTerrainByName(m_strTName);
    if (pTerr)
        EditParameters(pTerr->GetParamFile());
}

void StartupDlg::OnTerrain( wxCommandEvent &event )
{
    TransferDataFromWindow();
    UpdateState();
}

void StartupDlg::OnEarthView( wxCommandEvent &event )
{
    TransferDataFromWindow();
    UpdateState();
}

void StartupDlg::OnOpenGLInfo( wxCommandEvent &event )
{
    // check the OpenGL max texture size
#ifdef WIN32
    wxClientDC wdc(this);
    HDC hdc = (HDC) wdc.GetHDC();
    ShowOGLInfo(hdc);
#else
    ShowOGLInfo();
#endif
}

void StartupDlg::OnOK( wxCommandEvent &event )
{
    wxDialog::OnOK(event);
}

void StartupDlg::OnInitDialog(wxInitDialogEvent& event) 
{
    vtTerrain *pTerr = GetTerrainScene().FindTerrainByName(m_strTName);
    if (pTerr)
        m_strTName = pTerr->GetName();
    else
        m_strTName = "none";

    m_pTName = GetTname();
    m_pTSelect = GetTselect();
    m_psImage = GetImagetext();
    m_pImage = GetImage();

    AddFilenamesToComboBox(m_pImage, m_strDataPath + "WholeEarth/",
        "*_0106.png", 9);
    int sel = m_pImage->FindString(m_strImage);
    if (sel != -1)
        m_pImage->SetSelection(sel);

    UpdateState();

    AddValidator(ID_EARTHVIEW, &m_bStartEarth);
    AddValidator(ID_TERRAIN, &m_bStartTerrain);

    AddValidator(ID_FULLSCREEN, &m_bFullscreen);
    AddValidator(ID_GRAVITY, &m_bGravity);
    AddValidator(ID_HTML_PANE, &m_bHtmlpane);
    AddValidator(ID_FLOATING, &m_bFloatingToolbar);
    AddValidator(ID_SOUND, &m_bSound);
    AddValidator(ID_VCURSOR, &m_bVCursor);
    AddValidator(ID_SHADOWS, &m_bShadows);

    AddValidator(ID_TNAME, &m_strTName);
    AddValidator(ID_IMAGE, &m_strImage);
    AddNumValidator(ID_PLANTSIZE, &m_fPlantScale);

    wxWindow::OnInitDialog(event);
}

void StartupDlg::OnSelectTerrain( wxCommandEvent &event )
{
    TransferDataFromWindow();

    AskForTerrainName(this, m_strTName);

    TransferDataToWindow();
}

