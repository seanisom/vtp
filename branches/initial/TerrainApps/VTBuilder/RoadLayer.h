//
// RoadLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ROADLAYER_H
#define ROADLAYER_H

#include "Layer.h"
#include "RoadMapEdit.h"

//////////////////////////////////////////////////////////

class vtRoadLayer : public vtLayer, public RoadMapEdit
{
public:
	vtRoadLayer();
	~vtRoadLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void AppendDataFrom(vtLayer *pL);
	void Offset(DPoint2 p);

	static bool GetDrawNodes() { return m_bDrawNodes; }
	static void SetDrawNodes(bool d) { m_bDrawNodes = d; }
	static bool GetShowDirection() { return m_bShowDirection; }
	static void SetShowDirection(bool d) { m_bShowDirection = d; }

	int GetSelectedNodes();
	int GetSelectedRoads();
	void ToggleRoadDirection(RoadEdit *pRoad);
	bool SelectArea(DRECT box, bool nodemode, bool crossSelect);
	void MoveSelectedNodes(DPoint2 offset);

	//edit a single node
	bool EditNodeProperties(DPoint2 point, float error, DRECT &bound);
	//edit a single road
	bool EditRoadProperties(DPoint2 point, float error, DRECT &bound);
	//edit all selected nodes
	bool EditNodesProperties();
	//edit all selected roads
	bool EditRoadsProperties();

protected:
	static bool	m_bDrawNodes;
	static bool	m_bShowDirection;
};

#endif