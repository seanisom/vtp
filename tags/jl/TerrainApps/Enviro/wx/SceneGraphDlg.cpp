//
// Name:		SceneGraphDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "SceneGraphDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/treectrl.h"
#include <typeinfo>

#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"
#include "SceneGraphDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__)
#  include "icon1.xpm"
#  include "icon2.xpm"
#  include "icon3.xpm"
#  include "icon4.xpm"
#  include "icon5.xpm"
#  include "icon6.xpm"
#  include "icon7.xpm"
#  include "icon8.xpm"
#  include "icon9.xpm"
#  include "icon10.xpm"
#endif

/////////////////////////////

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(vtNode *pNode, vtEngine *pEngine)
	{
		m_pNode = pNode;
		m_pEngine = pEngine;
	}
	vtNode *m_pNode;
	vtEngine *m_pEngine;
};


// WDR: class implementations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

// WDR: event table for SceneGraphDlg

BEGIN_EVENT_TABLE(SceneGraphDlg,wxDialog)
	EVT_INIT_DIALOG (SceneGraphDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_SCENETREE, SceneGraphDlg::OnTreeSelChanged )
	EVT_CHECKBOX( ID_ENABLED, SceneGraphDlg::OnEnabled )
	EVT_BUTTON( ID_ZOOMTO, SceneGraphDlg::OnZoomTo )
	EVT_BUTTON( ID_REFRESH, SceneGraphDlg::OnRefresh )
END_EVENT_TABLE()

SceneGraphDlg::SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	SceneGraphFunc( this, TRUE ); 

	m_pZoomTo = GetZoomto();
	m_pEnabled = GetEnabled();
	m_pTree = GetScenetree();

	m_pZoomTo->Enable(false);

	m_imageListNormal = NULL;
	CreateImageList(16);
}

SceneGraphDlg::~SceneGraphDlg()
{
	delete m_imageListNormal;
}

///////////

void SceneGraphDlg::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}
	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	// should correspond to TreeCtrlIcon_xxx enum
	wxIcon icons[10];
	icons[0] = wxICON(icon1);
	icons[1] = wxICON(icon2);
	icons[2] = wxICON(icon3);
	icons[3] = wxICON(icon4);
	icons[4] = wxICON(icon5);
	icons[5] = wxICON(icon6);
	icons[6] = wxICON(icon7);
	icons[7] = wxICON(icon8);
	icons[8] = wxICON(icon9);
	icons[9] = wxICON(icon10);

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxImage(icons[i]).Rescale(size, size).
									ConvertToBitmap());
	}
	m_pTree->SetImageList(m_imageListNormal);
}


void SceneGraphDlg::RefreshTreeContents()
{
	vtScene* scene = vtGetScene();
	if (!scene)
		return;

	// start with a blank slate
	m_pTree->DeleteAllItems();

	// Fill in the tree with nodes
	vtNode *pRoot = scene->GetRoot();
	if (pRoot) AddNodeItemsRecursively(wxTreeItemId(), pRoot, 0);

	wxTreeItemId hRoot = m_pTree->GetRootItem();
	wxTreeItemId hEngRoot = m_pTree->AppendItem(hRoot, "Engines", 7, 7);

	// Fill in the tree with engines
	int num = scene->GetNumEngines();
	for (int i = 0; i < num; i++)
	{
		vtEngine *pEng = scene->GetEngine(i);
		wxString str;
		str += pEng->GetName2();
		vtNode *target = (vtNode *) pEng->GetTarget();
		if (target)
		{
			str += " -> ";
			vtNode *node = dynamic_cast<vtNode*>(target);
			if (node)
			{
				str += "\"";
				str += node->GetName2();
				str += "\"";
			}
			else
				str += "(non-node)";
		}
		wxTreeItemId hEng = m_pTree->AppendItem(hEngRoot, str, 1, 1);
		m_pTree->SetItemData(hEng, new MyTreeItemData(NULL, pEng));
	}
	m_pTree->Expand(hEngRoot);

	m_pSelectedEngine = NULL;
	m_pSelectedNode = NULL;
}


void SceneGraphDlg::AddNodeItemsRecursively(wxTreeItemId hParentItem,
										vtNode *pNode, int depth)
{
	wxString str;
	int nImage;
	wxTreeItemId hNewItem;

	if (!pNode) return;

	if (dynamic_cast<vtRoot*>(pNode))
	{
		str = "Root";
		nImage = 7;
	}
	else if (dynamic_cast<vtLight*>(pNode))
	{
		str = "Light";
		nImage = 4;
	}
	else if (dynamic_cast<vtGeom*>(pNode))
	{
		str = "Geom";
		nImage = 2;
	}
	else if (dynamic_cast<vtLOD*>(pNode))
	{
		str = "LOD";
		nImage = 5;
	}
	else if (dynamic_cast<vtTransform*>(pNode))
	{
		str = "XForm";
		nImage = 9;
	}
	else if (dynamic_cast<vtGroup*>(pNode))
	{
		// must be just a group for grouping's sake
		str = "Group";
		nImage = 3;
	}
	else
	{
		// must be something else
		str = "Other";
		nImage = 8;
	}
	if (pNode->GetName2())
	{
		str += " \"";
		str += pNode->GetName2();
		str += "\"";
	}

	if (dynamic_cast<vtRoot*>(pNode))
		hNewItem = m_pTree->AddRoot(str);
	else
		hNewItem = m_pTree->AppendItem(hParentItem, str, nImage, nImage);

	const type_info &t1 = typeid(*pNode);
	if (t1 == typeid(vtGeom))
	{
		vtGeom *pGeom = dynamic_cast<vtGeom*>(pNode);
		int num_mesh = pGeom->GetNumMeshes();
		wxTreeItemId	hGeomItem;

		for (int i = 0; i < num_mesh; i++)
		{
			vtMesh *pMesh = pGeom->GetMesh(i);
			int iNumPrim = pMesh->GetNumPrims();

			GLenum pt = pMesh->GetPrimType();
			const char *mtype;
			switch (pt)
			{
			case GL_POINTS: mtype = "Points"; break;
			case GL_LINES: mtype = "Lines"; break;
			case GL_LINE_LOOP: mtype = "LineLoop"; break;
			case GL_LINE_STRIP: mtype = "LineStrip"; break;
			case GL_TRIANGLES: mtype = "Triangles"; break;
			case GL_TRIANGLE_STRIP: mtype = "TriStrip"; break;
			case GL_TRIANGLE_FAN: mtype = "TriFan"; break;
			case GL_QUADS: mtype = "Quads"; break;
			case GL_QUAD_STRIP: mtype = "QuadStrip"; break;
			case GL_POLYGON: mtype = "Polygon"; break;
			}
			str.Printf("Mesh %d, %s, %d prims", i, mtype, iNumPrim);
			hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
		}
	}

	m_pTree->SetItemData(hNewItem, new MyTreeItemData(pNode, NULL));

	vtGroup *pGroup = dynamic_cast<vtGroup*>(pNode);
	if (pGroup)
	{
		int num_children = pGroup->GetNumChildren();
		if (num_children > 200)
		{
			wxTreeItemId	hSubItem;
			str.Format("(%d children)", num_children);
			hSubItem = m_pTree->AppendItem(hNewItem, str, 8, 8);
		}
		else
		{
			for (int i = 0; i < num_children; i++)
			{
				vtNode *pChild = pGroup->GetChild(i);
				if (!pChild) continue;

				AddNodeItemsRecursively(hNewItem, pChild, depth+1);
			}
		}
	}
	// expand a bit so that the tree is initially partially exposed
	if (depth < 2)
		m_pTree->Expand(hNewItem);
}


// WDR: handler implementations for SceneGraphDlg

void SceneGraphDlg::OnRefresh( wxCommandEvent &event )
{
	RefreshTreeContents();
}

void SceneGraphDlg::OnZoomTo( wxCommandEvent &event )
{
	if (m_pSelectedNode)
	{
		FSphere sph;
		m_pSelectedNode->GetBoundSphere(sph);
		vtGetScene()->GetCamera()->ZoomToSphere(sph);
	}
}

void SceneGraphDlg::OnEnabled( wxCommandEvent &event )
{
	if (m_pSelectedEngine)
		m_pSelectedEngine->SetEnabled(m_pEnabled->GetValue());
	if (m_pSelectedNode)
		m_pSelectedNode->SetEnabled(m_pEnabled->GetValue());
}

void SceneGraphDlg::OnTreeSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

	m_pEnabled->Enable(data != NULL);

	m_pSelectedEngine = NULL;
	m_pSelectedNode = NULL;

	if (data && data->m_pEngine)
	{
		m_pSelectedEngine = data->m_pEngine;
		m_pEnabled->SetValue(m_pSelectedEngine->GetEnabled());
	}
	if (data && data->m_pNode)
	{
		m_pSelectedNode = data->m_pNode;
		m_pEnabled->SetValue(m_pSelectedNode->GetEnabled());
		m_pZoomTo->Enable(true);
	}
	else
		m_pZoomTo->Enable(false);
}

void SceneGraphDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	RefreshTreeContents();

	wxWindow::OnInitDialog(event);
}
