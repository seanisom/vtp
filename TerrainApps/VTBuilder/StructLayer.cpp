//
// StructLayer.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/DLG.h"
#include "vtdata/Fence.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "ogrsf_frmts.h"

#include "Frame.h"
#include "StructLayer.h"
#include "ElevLayer.h"
#include "BuilderView.h"
#include "vtui/BuildingDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/Helper.h"
#include "Helper.h"
#include "ImportStructDlg.h"

wxPen orangePen;
wxPen yellowPen;
wxPen thickPen;
static bool g_bInitializedPens = false;

//////////////////////////////////////////////////////////////////////////

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	wxString name = _("Untitled");
	name += _T(".vtst");
	SetLayerFilename(name);

	m_bPreferGZip = false;

	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;

		orangePen.SetColour(255,128,0);
		yellowPen.SetColour(255,255,0);

		thickPen.SetColour(255,255,255);
		thickPen.SetWidth(3);
	}
}

bool vtStructureLayer::GetExtent(DRECT &rect)
{
	if (IsEmpty())
		return false;

	GetExtents(rect);

	// expand by 10 meters (converted to appropriate units)
	DPoint2 offset(10, 10);
	if (m_proj.IsGeographic())
	{
		DPoint2 center;
		rect.GetCenter(center);
		double fMetersPerLongitude = EstimateDegreesToMeters(center.y);
		offset.x /= fMetersPerLongitude;
		offset.y /= METERS_PER_LATITUDE;
	}
	else
	{
		double fMetersPerUnit = GetMetersPerUnit(m_proj.GetUnits());
		offset.x /= fMetersPerUnit;
		offset.y /= fMetersPerUnit;
	}
	rect.left -= offset.x;
	rect.right += offset.x;
	rect.bottom -= offset.y;
	rect.top += offset.y;

	return true;
}

void vtStructureLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	int structs = GetSize();
	if (!structs)
		return;

	pDC->SetPen(orangePen);
	pDC->SetBrush(*wxTRANSPARENT_BRUSH);
	bool bSel = false;

	m_size = pView->sdx(20);
	if (m_size > 5) m_size = 5;
	if (m_size < 1) m_size = 1;

	int i;
	for (i = 0; i < structs; i++)
	{
		// draw each building
		vtStructure *str = GetAt(i);
		if (str->IsSelected())
		{
			if (!bSel)
			{
				pDC->SetPen(yellowPen);
				bSel = true;
			}
		}
		else
		{
			if (bSel)
			{
				pDC->SetPen(orangePen);
				bSel = false;
			}
		}
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			DrawBuilding(pDC, pView, bld);

		vtFence *fen = str->GetFence();
		if (fen)
			DrawLinear(pDC, pView, fen);

		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			wxPoint origin;
			pView->screen(inst->GetPoint(), origin);

			pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
			pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);

			if (inst->GetItem())
			{
				FRECT ext = inst->GetItem()->m_extents;
				if (!ext.IsEmpty())
				{
					pView->screen(inst->GetPoint() + ext.Center(), origin);
					int radius = pView->sdx(ext.Width() / 2);
					pDC->DrawCircle(origin, radius);
				}
			}
		}
	}
	DrawBuildingHighlight(pDC, pView);
}

void vtStructureLayer::DrawBuildingHighlight(wxDC* pDC, vtScaledView *pView)
{
	if (m_pEditBuilding)
	{
		pDC->SetLogicalFunction(wxINVERT);
		pDC->SetPen(thickPen);

		const DLine2 &dl = m_pEditBuilding->GetFootprint(m_iEditLevel);
		int sides = dl.GetSize();
		int j = m_iEditEdge;
		wxPoint p[2];
		pView->screen(dl.GetAt(j), p[0]);
		pView->screen(dl.GetAt((j+1)%sides), p[1]);
		pDC->DrawLines(2, p);
	}
}

void vtStructureLayer::DrawBuilding(wxDC* pDC, vtScaledView *pView,
									vtBuilding *bld)
{
	DPoint2 center;
	int i, j;

	wxPoint origin;
	bld->GetBaseLevelCenter(center);
	pView->screen(center, origin);

	// crosshair at building center
	pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
	pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);

	// draw building footprint for all levels
	int levs = bld->GetNumLevels();

	// unless we're XORing, in which case multiple overlapping level would
	// cancel each other out
	if (pDC->GetLogicalFunction() == wxINVERT)
		levs = 1;

	for (i = 0; i < levs; i++)
	{
		const DLine2 &dl = bld->GetFootprint(i);
		pView->DrawLine(pDC, dl, true);

		int sides = dl.GetSize();
		for (j = 0; j < sides; j++)
			pDC->DrawCircle(g_screenbuf[j], 3);
	}
}

void vtStructureLayer::DrawLinear(wxDC* pDC, vtScaledView *pView, vtFence *fen)
{
	unsigned int j;

	// draw the line itself
	DLine2 &pts = fen->GetFencePoints();
	pView->DrawLine(pDC, pts, false);

	// draw a small crosshair on each vertex
	for (j = 0; j < pts.GetSize(); j++)
	{
		pDC->DrawLine(g_screenbuf[j].x-2, g_screenbuf[j].y,
			g_screenbuf[j].x+2, g_screenbuf[j].y);
		pDC->DrawLine(g_screenbuf[j].x, g_screenbuf[j].y-2,
			g_screenbuf[j].x, g_screenbuf[j].y+2);
	}
}

bool vtStructureLayer::OnSave()
{
	return WriteXML(GetFilename(), m_bPreferGZip);
}

bool vtStructureLayer::OnLoad()
{
	vtString fname = GetFilename();

	// remember whether this layer was read from a compressed file
	if (GetExtension(fname, true).CompareNoCase(".vtst.gz") == 0)
		m_bPreferGZip = true;

	// check file size, show progressm dialog for big files
	int size = GetFileSize(fname);
	bool bShowProgress = (size > 1024*1024);	// 1 MB
	if (m_bPreferGZip)
		bShowProgress = (size > 1024*50);	// 50 KB

	if (bShowProgress)
		OpenProgressDialog(_T("Loading Structures"), false);

	bool success = ReadXML(fname, progress_callback);

	if (bShowProgress)
		CloseProgressDialog();

	if (!success)
		return false;

	ResolveInstancesOfItems();
	return true;
}

void vtStructureLayer::ResolveInstancesOfItems()
{
	MainFrame *frame = GetMainFrame();

	unsigned int structs = GetSize();
	for (unsigned int i = 0; i < structs; i++)
	{
		vtStructure *str = GetAt(i);
		vtStructInstance *inst = str->GetInstance();
		if (!inst)
			continue;
		frame->ResolveInstanceItem(inst);
	}
}

void vtStructureLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtStructureLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

bool vtStructureLayer::TransformCoords(vtProjection &proj)
{
	if (proj == m_proj)
		return true;

	// Create conversion object
	vtProjection Source;
	GetProjection(Source);
	OCT *trans = CreateCoordTransform(&Source, &proj);
	if (!trans)
		return false;		// inconvertible projections

	DPoint2 loc;
	unsigned int i, j;
	unsigned int count = GetSize();
	for (i = 0; i < count; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->TransformCoords(trans);

		vtFence *fen = str->GetFence();
		if (fen)
		{
			DLine2 line = fen->GetFencePoints();
			for (j = 0; j < line.GetSize(); j++)
			{
				loc = line[j];
				trans->Transform(1, &loc.x, &loc.y);
				line[j] = loc;
			}
		}

		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			loc = inst->GetPoint();
			trans->Transform(1, &loc.x, &loc.y);
			inst->SetPoint(loc);
		}
	}

	// set the projection
	m_proj = proj;

	delete trans;
	return true;
}

bool vtStructureLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_STRUCTURE)
		return false;

	vtStructureLayer *pFrom = (vtStructureLayer *)pL;

	int count = pFrom->GetSize();
	for (int i = 0; i < count; i++)
	{
		vtStructure *str = pFrom->GetAt(i);
		Append(str);
	}
	// tell the source layer that it has no structures (we have taken them)
	pFrom->SetSize(0);

	return true;
}

void vtStructureLayer::Offset(const DPoint2 &delta)
{
	vtStructureArray::Offset(delta);
}

void vtStructureLayer::GetPropertyText(wxString &strIn)
{
	int i, size = GetSize();

	strIn += _("Filename: ");
	strIn += GetLayerFilename();
	strIn += _T("\n");

	wxString str;
	str.Printf(_("Number of structures: %d\n"), size);
	strIn += str;

	int bld = 0, lin = 0, ins = 0;
	for (i = 0; i < size; i++)
	{
		vtStructure *sp = GetAt(i);
		if (sp->GetBuilding()) bld++;
		else if (sp->GetFence()) lin++;
		else if (sp->GetInstance()) ins++;
	}
	str.Printf(_("\t %d Buildings (procedural)\n"), bld);
	strIn += str;
	str.Printf(_("\t %d Linear (fences/walls)\n"), lin);
	strIn += str;
	str.Printf(_("\t %d Instances (imported models)\n"), ins);
	strIn += str;

	str.Printf(_("Number of selected structures: %d\n"), NumSelected());
	strIn += str;
}

void vtStructureLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	switch (ui.mode)
	{
	case LB_AddInstance:
		OnLeftDownAddInstance(pView, ui);
		break;
	case LB_AddLinear:
		if (ui.m_pCurLinear == NULL)
		{
			ui.m_pCurLinear = NewFence();
			ui.m_pCurLinear->SetParams(GetMainFrame()->m_LSOptions);
			Append(ui.m_pCurLinear);
			ui.m_bRubber = true;
		}
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(true);
		break;
	case LB_BldEdit:
		OnLeftDownEditBuilding(pView, ui);
		break;
	case LB_BldAddPoints:
		OnLeftDownBldAddPoints(pView, ui);
		break;
	case LB_BldDeletePoints:
		OnLeftDownBldDeletePoints(pView, ui);
		break;
	case LB_EditLinear:
		OnLeftDownEditLinear(pView, ui);
		// TODO - find nearest point on nearest linear
		break;
	default:
		break;
	}
}

void vtStructureLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		DRECT extent_old, extent_new;
		ui.m_pCurBuilding->GetExtents(extent_old);
		ui.m_EditBuilding.GetExtents(extent_new);
		wxRect screen_old = pView->WorldToWindow(extent_old);
		wxRect screen_new = pView->WorldToWindow(extent_new);
		screen_old.Inflate(1);
		screen_new.Inflate(1);

		pView->Refresh(true, &screen_old);
		pView->Refresh(true, &screen_new);

		// copy back from temp building to real building
		*ui.m_pCurBuilding = ui.m_EditBuilding;
		ui.m_bRubber = false;
		GetMainFrame()->GetActiveLayer()->SetModified(true);
		ui.m_pCurBuilding = NULL;
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		DRECT extent_old, extent_new;
		ui.m_pCurLinear->GetExtents(extent_old);
		ui.m_EditLinear.GetExtents(extent_new);
		wxRect screen_old = pView->WorldToWindow(extent_old);
		wxRect screen_new = pView->WorldToWindow(extent_new);

		pView->Refresh(true, &screen_old);
		pView->Refresh(true, &screen_new);

		// copy back from temp building to real building
		*ui.m_pCurLinear = ui.m_EditLinear;
		ui.m_bRubber = false;
		GetMainFrame()->GetActiveLayer()->SetModified(true);
		ui.m_pCurLinear = NULL;
	}
}

/*void ResolveClosest(bool &valid1, bool &valid2, bool &valid3,
					double dist1, double dist2, double dist3)
{
	if (valid1 && valid2)
	{
		if (dist1 < dist2)
			valid2 = false;
		else
			valid1 = false;
	}
	if (valid1 && valid3)
	{
		if (dist1 < dist3)
			valid3 = false;
		else
			valid1 = false;
	}
	if (valid2 && valid3)
	{
		if (dist2 < dist3)
			valid3 = false;
		else
			valid2 = false;
	}
}*/

void vtStructureLayer::OnLeftDownEditBuilding(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->odx(6);  // 6 pixels as world coord

	int building1, building2,  corner;
	double dist1, dist2;

	bool found1 = FindClosestBuildingCenter(ui.m_DownLocation, epsilon, building1, dist1);
	bool found2 = FindClosestBuildingCorner(ui.m_DownLocation, epsilon, building2, corner, dist2);

	if (found1 && found2)
	{
		// which was closer?
		if (dist1 < dist2)
			found2 = false;
		else
			found1 = false;
	}
	if (found1)
	{
		// closest point is a building center
		ui.m_pCurBuilding = GetAt(building1)->GetBuilding();
		ui.m_bDragCenter = true;
	}
	if (found2)
	{
		// closest point is a building corner
		ui.m_pCurBuilding = GetAt(building2)->GetBuilding();
		ui.m_bDragCenter = false;
		ui.m_iCurCorner = corner;
		ui.m_bRotate = ui.m_bControl;
	}
	if (found1 || found2)
	{
		ui.m_bRubber = true;

		// make a copy of the building, to edit and display while dragging
		ui.m_EditBuilding = *ui.m_pCurBuilding;
	}
}

void vtStructureLayer::OnLeftDownBldAddPoints(BuilderView *pView, UIContext &ui)
{
	double dEpsilon = pView->odx(6);  // 6 pixels as world coord
	double dClosest;
	int iStructure;

	if (!FindClosestBuilding(ui.m_DownLocation, dEpsilon, iStructure, dClosest))
		return;

	vtBuilding *pBuilding = GetAt(iStructure)->GetBuilding();

	// Find extent of building and refresh that area of the window
	DRECT Extent;
	wxRect Redraw;
	pBuilding->GetExtents(Extent);
	Redraw = pView->WorldToWindow(Extent);
	Redraw.Inflate(3);
	pView->Refresh(true, &Redraw);

	// Variables for levels and edges
	vtLevel *pLevel;
	int i, iIndex, iLevel, iNumLevels;
	DPoint2 Intersection;

#if ROGER
	// See if I can get some UI feedback;
	m_pEditBuilding = pBuilding;
	m_iEditLevel = 0;
	pLevel = pBuilding->GetLevel(0);
	pLevel->GetFootprint().NearestSegment(ui.m_DownLocation, iIndex,
		dClosest, Intersection);
	m_iEditEdge = iIndex;

	// Find out the level to work on
#if 0
	CLevelSelectionDialog LevelSelectionDialog(pView, -1, _T("Select level to edit"));

	LevelSelectionDialog.SetBuilding(pBuilding);

	if (LevelSelectionDialog.ShowModal()!= wxID_OK)
	{
		m_pEditBuilding = NULL;
		pView->Refresh(true, &Redraw);
		return;
	}
	iLevel = LevelSelectionDialog.GetLevel();
#else
	int iLevels = pBuilding->GetNumLevels();
	wxString msg;
	msg.Printf(_("Select level to edit (0 .. %d)"), iLevels);
	iLevel = wxGetNumberFromUser(msg, _("Level"), _("Enter Value"), 0, 0, iLevels);
	if (iLevel == -1)
	{
		m_pEditBuilding = NULL;
		pView->Refresh(true, &Redraw);
		return;
	}
#endif

	m_pEditBuilding = NULL;
	pView->Refresh(true, &Redraw);
#else
	// Since we don't yet do editing of level footprints other than 0,
	//  and editing it affects all levels, it's only useful to do all levels
	iLevel = -1;
#endif

	if (-1 == iLevel)
	{
		// Add in all levels
		iNumLevels = pBuilding->GetNumLevels();
		for (i = 0; i < iNumLevels; i++)
		{
			pLevel = pBuilding->GetLevel(i);
			if (pLevel->GetAtFootprint().NearestSegment(ui.m_DownLocation,
				iIndex, dClosest, Intersection))
			{
				pLevel->AddEdge(iIndex, Intersection);
			}
		}
	}
	else
	{
		// Add in specified level
		pLevel = pBuilding->GetLevel(iLevel);
		if (pLevel->GetAtFootprint().NearestSegment(ui.m_DownLocation,
			iIndex, dClosest, Intersection))
		{
			pLevel->AddEdge(iIndex, Intersection);
		}
	}

	// Find new extent of building and refresh that area of the window
	pBuilding->GetExtents(Extent);
	Redraw = pView->WorldToWindow(Extent);
	Redraw.Inflate(3);
	pView->Refresh(true, &Redraw);
}

void vtStructureLayer::OnLeftDownBldDeletePoints(BuilderView *pView, UIContext &ui)
{
	double dEpsilon = pView->odx(6);  // 6 pixels as world coord
	double dClosest;
	int iStructure;

	if (!FindClosestBuilding(ui.m_DownLocation, dEpsilon, iStructure, dClosest))
		return;

	vtBuilding *pBuilding = GetAt(iStructure)->GetBuilding();

	// Find extent of building before any point removal
	DRECT Extent;
	pBuilding->GetExtents(Extent);

	vtLevel *pLevel;
	int i, iIndex, iLevel, iNumLevels;

#if ROGER
	// See if I can get some UI feedback;
	m_pEditBuilding = pBuilding;
	m_iEditLevel = 0;
	pLevel = pBuilding->GetLevel(0);
	pLevel->GetFootprint().NearestPoint(ui.m_DownLocation, iIndex);
	m_iEditEdge = iIndex;

	// Find out the level to work on
#if 0
	CLevelSelectionDialog LevelSelectionDialog(pView, -1, _T("Select level to edit"));

	LevelSelectionDialog.SetBuilding(pBuilding);

	if (LevelSelectionDialog.ShowModal()!= wxID_OK)
	{
		m_pEditBuilding = NULL;
		pView->Refresh(true, &Redraw);
		return;
	}
	iLevel = LevelSelectionDialog.GetLevel();
#else
	int iLevels = pBuilding->GetNumLevels();
	wxString msg;
	msg.Printf(_T("Select level to edit (0 .. %d)"), iLevels);
	iLevel = wxGetNumberFromUser(msg, _("Level"), _("Enter Value"), 0, 0, iLevels);
	if (iLevel == -1)
	{
		m_pEditBuilding = NULL;
		pView->Refresh(true, &Redraw);
		return;
	}
#endif
	m_pEditBuilding = NULL;
	pView->Refresh(true, &Redraw);
#else
	// Since we don't yet do editing of level footprints other than 0,
	//  and editing it affects all levels, it's only useful to do all levels
	iLevel = -1;
#endif

	if (-1 == iLevel)
	{
		// Remove in all levels
		iNumLevels = pBuilding->GetNumLevels();
		for (i = 0; i <iNumLevels; i++)
		{
			pLevel = pBuilding->GetLevel(i);
			pLevel->GetFootprint().NearestPoint(ui.m_DownLocation, iIndex, dClosest);
			pLevel->DeleteEdge(iIndex);
		}
	}
	else
	{
		// Remove in specified level
		pLevel = pBuilding->GetLevel(iLevel);
		pLevel->GetFootprint().NearestPoint(ui.m_DownLocation, iIndex, dClosest);
		pLevel->DeleteEdge(iIndex);
	}

	// Refresh area of the window with original building extent
	wxRect Redraw;
	Redraw = pView->WorldToWindow(Extent);
	Redraw.Inflate(3);
	pView->Refresh(true, &Redraw);
}

void vtStructureLayer::OnLeftDownEditLinear(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->odx(6);  // 6 pixels as world coord

	int structure, corner;
	double dist1;

	bool found1 = FindClosestLinearCorner(ui.m_DownLocation, epsilon,
		structure, corner, dist1);

	if (found1)
	{
		// closest point is a building center
		ui.m_pCurLinear = GetAt(structure)->GetFence();
		ui.m_iCurCorner = corner;
		ui.m_bRubber = true;

		// make a copy of the linear, to edit and display while dragging
		ui.m_EditLinear = *ui.m_pCurLinear;
	}
}

void vtStructureLayer::OnLeftDownAddInstance(BuilderView *pView, UIContext &ui)
{
	MainFrame *frame = GetMainFrame();
	InstanceDlg *dlg = frame->m_pInstanceDlg;

	vtStructInstance *inst = AddNewInstance();

	inst->SetPoint(ui.m_DownLocation);

	vtTagArray *tags = dlg->GetTagArray();
	inst->CopyTagsFrom(*tags);

	frame->ResolveInstanceItem(inst);

	SetModified(true);
	pView->Refresh();
}

void vtStructureLayer::OnRightDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_AddLinear && ui.m_pCurLinear != NULL)
	{
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(true);
		ui.m_pCurLinear = NULL;
		ui.m_bRubber = false;
		SetModified(true);
	}
}

void vtStructureLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	// create rubber (xor) pen
	wxClientDC dc(pView);
	pView->PrepareDC(dc);
	wxPen pen(*wxBLACK_PEN);
	dc.SetPen(pen);
	dc.SetLogicalFunction(wxINVERT);

	if (ui.m_bLMouseButton && ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		// rubber-band a building
		DrawBuilding(&dc, pView, &ui.m_EditBuilding);

		if (ui.m_bDragCenter)
			UpdateMove(ui);
		else if (ui.m_bRotate)
			UpdateRotate(ui);
		else
			UpdateResizeScale(pView, ui);

		DrawBuilding(&dc, pView, &ui.m_EditBuilding);
	}
	if (ui.mode == LB_AddLinear && ui.m_bRubber)
	{
		wxPoint p1, p2;
		DLine2 &pts = ui.m_pCurLinear->GetFencePoints();
		pView->screen(pts.GetAt(pts.GetSize()-1), p1);
		dc.DrawLine(p1, ui.m_LastPoint);
		dc.DrawLine(p1, ui.m_CurPoint);
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		// rubber-band a linear
		DrawLinear(&dc, pView, &ui.m_EditLinear);

		ui.m_EditLinear.GetFencePoints().SetAt(ui.m_iCurCorner, ui.m_CurLocation);

		DrawLinear(&dc, pView, &ui.m_EditLinear);
	}
}

void vtStructureLayer::UpdateMove(UIContext &ui)
{
	DPoint2 p;
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	int i, levs = ui.m_pCurBuilding->GetNumLevels();
	for (i = 0; i < levs; i++)
	{
		DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
		dl.Add(moved_by);
		ui.m_EditBuilding.SetFootprint(i, dl);
	}
}

void vtStructureLayer::UpdateRotate(UIContext &ui)
{
	DPoint2 origin;
	ui.m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 original_vector = ui.m_DownLocation - origin;
	double angle1 = atan2(original_vector.y, original_vector.x);

	DPoint2 cur_vector = ui.m_CurLocation - origin;
	double angle2 = atan2(cur_vector.y, cur_vector.x);

	double angle_diff = angle2 - angle1;

	DPoint2 p;
	unsigned int i, j, levs = ui.m_pCurBuilding->GetNumLevels();
	for (i = 0; i < levs; i++)
	{
		DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
		for (j = 0; j < dl.GetSize(); j++)
		{
			p = dl.GetAt(j);
			p -= origin;
			p.Rotate(angle_diff);
			p += origin;
			dl.SetAt(j, p);
		}
		ui.m_EditBuilding.SetFootprint(i, dl);
	}
}

void vtStructureLayer::UpdateResizeScale(BuilderView *pView, UIContext &ui)
{
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	DPoint2 origin;
	ui.m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 diff1 = ui.m_DownLocation - origin;
	DPoint2 diff2 = ui.m_CurLocation - origin;
	float fScale = diff2.Length() / diff1.Length();

	unsigned int i, j;
	DPoint2 p;
	if (ui.m_bShift)
	{
		// Scale evenly
		unsigned int levs = ui.m_pCurBuilding->GetNumLevels();
		for (i = 0; i < levs; i++)
		{
			DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
			for (j = 0; j < dl.GetSize(); j++)
			{
				p = dl.GetAt(j);
				p -= origin;
				p *= fScale;
				p += origin;
				dl.SetAt(j, p);
			}
			ui.m_EditBuilding.SetFootprint(i, dl);
		}
	}
	else
	{
		// drag individual corner points
		DLine2 foot = ui.m_pCurBuilding->GetFootprint(0);
		p = foot.GetAt(ui.m_iCurCorner);

		int points = foot.GetSize();
		if (pView->m_bConstrain && points > 3)
		{
			// Maintain angles
			DPoint2 p0 = foot.GetSafePoint(ui.m_iCurCorner - 1);
			DPoint2 p1 = foot.GetSafePoint(ui.m_iCurCorner + 1);
			DPoint2 vec0 = (p - p0).Normalize();
			DPoint2 vec1 = (p - p1).Normalize();
			DPoint2 vec2 = vec0;
			vec2.Rotate(PID2d);
			DPoint2 vec3 = vec1;
			vec3.Rotate(PID2d);

			p += moved_by;
			double a;

			a = (p - p0).Dot(vec2);
			foot.SetSafePoint(ui.m_iCurCorner - 1, p0 + (vec2 * a));
			a = (p - p1).Dot(vec3);
			foot.SetSafePoint(ui.m_iCurCorner + 1, p1 + (vec3 * a));
			foot.SetAt(ui.m_iCurCorner, p);
		}
		else
		{
			p += moved_by;
			foot.SetAt(ui.m_iCurCorner, p);
		}
		// Changing only the lowest level is near useless.  For the great
		//  majority of cases, the user will want the footprints for all
		//  levels to remain in sync.
		for (i = 0; i < ui.m_EditBuilding.GetNumLevels(); i++)
			ui.m_EditBuilding.SetFootprint(i, foot);

		// A better guess might be to offset the footprint points of each
		// level by the delta change; e.g. that would preserve roof overhangs.
	}
}


/////////////////////////////////////////////////////////////////////////////

bool vtStructureLayer::EditBuildingProperties()
{
	// Look for the first selected building, and count how many are selected
	int count = 0;
	vtBuilding *bld_selected=NULL;
	int size = GetSize();
	for (int i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld && str->IsSelected())
		{
			count++;
			bld_selected = bld;
		}
	}
	// We don't support editing more than 1 building at a time
	if (count != 1)
		return false;

	// Get a heightfield.  This is for RJ's Baseline Editor" Height Dialog
	MainFrame *pFrame = GetMainFrame();
	vtHeightField *pHeightField = NULL;
	vtElevLayer *pElevLayer = (vtElevLayer *) pFrame->FindLayerOfType(LT_ELEVATION);
	if (pElevLayer)
		pHeightField = pElevLayer->GetHeightField();

	// for now, assume they will use the dialog to change something about
	// the building (pessimistic)
	SetModified(true);

	BuildingDlg dlg(NULL, -1, _("Building Properties"));
	dlg.Setup(this, bld_selected, pHeightField, GetMainFrame()->m_datapaths);

	dlg.ShowModal();

	return true;
}

void vtStructureLayer::AddFoundations(vtElevLayer *pEL)
{
	// TODO: ask user if they want concrete, the same material as the
	//	building wall, or what.
	// TODO: ask user exactly how much slope (or depth) to tolerate
	//	without building a foundation.

#if 1
	int built = vtStructureArray::AddFoundations(pEL->GetHeightField());
#else
	vtLevel *pLev, *pNewLev;
	int i, j, pts, built = 0;
	float fElev;

	int selected = NumSelected();
	int size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		if (selected > 0 && !str->IsSelected())
			continue;

		// Get the footprint of the lowest level
		pLev = bld->GetLevel(0);
		const DLine2 &foot = pLev->GetFootprint();
		pts = foot.GetSize();

		float fMin = 1E9, fMax = -1E9;
		for (j = 0; j < pts; j++)
		{
			fElev = pEL->GetElevation(foot.GetAt(j));
			if (fElev == INVALID_ELEVATION)
				continue;

			if (fElev < fMin) fMin = fElev;
			if (fElev > fMax) fMax = fElev;
		}
		float fDiff = fMax - fMin;

		// if there's less than 50cm of depth, don't bother building
		// a foundation
		if (fDiff < 0.5f)
			continue;

		// Create and add a foundation level
		pNewLev = new vtLevel;
		pNewLev->m_iStories = 1;
		pNewLev->m_fStoryHeight = fDiff;
		bld->InsertLevel(0, pNewLev);
		bld->SetFootprint(0, foot);
		pNewLev->SetEdgeMaterial(BMAT_NAME_CEMENT);
		pNewLev->SetEdgeColor(RGBi(255, 255, 255));
		built++;
	}
#endif
	DisplayAndLog("Added a foundation level to %d buildings.", built);
}

void vtStructureLayer::InvertSelection()
{
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		str->Select(!str->IsSelected());
	}
}

void vtStructureLayer::DeselectAll()
{
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		str->Select(false);
	}
}

int vtStructureLayer::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	bool bWas;

	for (unsigned int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);

		bWas = str->IsSelected();
		if (st == ST_NORMAL)
			str->Select(false);

		if (!str->IsContainedBy(rect))
			continue;

		switch (st)
		{
		case ST_NORMAL:
			str->Select(true);
			affected++;
			break;
		case ST_ADD:
			str->Select(true);
			if (!bWas) affected++;
			break;
		case ST_SUBTRACT:
			str->Select(false);
			if (bWas) affected++;
			break;
		case ST_TOGGLE:
			str->Select(!bWas);
			affected++;
			break;
		}
	}
	return affected;
}


void vtStructureLayer::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	vtScaledView *pView = GetMainFrame()->GetView();
	wxClientDC dc(pView);
	pView->PrepareDC(dc);

	DrawBuildingHighlight(&dc, pView);

	vtStructureArray::SetEditedEdge(bld, lev, edge);

	DrawBuildingHighlight(&dc, pView);

	// Trigger re-draw of the building
//	bound = WorldToWindow(world_bounds[n]);
//	IncreaseRect(bound, BOUNDADJUST);
//	Refresh(TRUE, &bound);
}


/////////////////////////////////////////////////////////////////////////////
// Import methods

bool vtStructureLayer::AddElementsFromSHP(const wxString &filename,
										  const vtProjection &proj, DRECT rect)
{
	ImportStructDlg dlg(NULL, -1, _("Import Structures"));

	dlg.SetFileName(filename);
	if (dlg.ShowModal() != wxID_OK)
		return false;

	dlg.m_opt.rect = rect;

	bool success = ReadSHP(filename.mb_str(wxConvUTF8), dlg.m_opt, progress_callback);
	if (!success)
		return false;

	m_proj = proj;	// Set projection
	return true;
}

void vtStructureLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	// set projection
	m_proj = pDlg->GetProjection();

/*	TODO: similar code to import from SDTS-DLG
	NEEDED: an actual sample file containing building data in this format.
*/
}

bool vtStructureLayer::AskForSaveFilename()
{
	wxString filter;
	filter += _T("VTST File (.vtst)|*.vtst");
	filter += _T("|");
	filter += _T("GZipped VTST File (.vtst.gz)|*.vtst.gz");

	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), GetLayerFilename(),
		filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	saveFile.SetFilterIndex( m_bPreferGZip ? 1 : 0);

	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	wxString fname = saveFile.GetPath();
	m_bPreferGZip = (saveFile.GetFilterIndex() == 1);

	// work around incorrect extension(s) that wxFileDialog added
	RemoveFileExtensions(fname);
	if (m_bPreferGZip)
		fname += _T(".vtst.gz");
	else
		fname += _T(".vtst");

	SetLayerFilename(fname);
	m_bNative = true;
	return true;
}