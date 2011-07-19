//
// GeometryUtils.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_GEOMETRYUTILSH
#define VTOSG_GEOMETRYUTILSH

#include "vtlib/core/Structure3d.h"

class vtBuilding3d;
class vtLevel;

namespace OSGGeometryUtils
{
	class GenerateBuildingGeometry : public osg::Referenced, public vtStructure3d // subclass vtStructure3d to get hold of shared material functions 
	{
	public:
		GenerateBuildingGeometry(const vtBuilding3d& Building) : m_Building(Building) {}
		vtGeode* Generate();
	protected: 
		void AddFlatRoof(const FPolygon3 &pp, const vtLevel *pLev);
		void CreateUniformLevel(int iLevel, float fHeight, int iHighlightEdge);
		bool MakeFacade(vtEdge *pEdge, FLine3 &quad, int stories);
		osg::Vec3 Normal(const vtVec3 &p0, const vtVec3 &p1, const vtVec3 &p2);
		void AddWallSection(vtEdge *pEdge, bool bUniform,
			const FLine3 &quad, float vf1, float vf2, float hf1 = -1.0f);
		void AddHighlightSection(vtEdge *pEdge, const FLine3 &quad);
		float MakeFelkelRoof(const FPolygon3 &EavePolygons, const vtLevel *pLev);
		bool Collinear2d(const FPoint3& Previous, const FPoint3& Current, const FPoint3& Next);
		void CreateUpperPolygon(const vtLevel *lev, FPolygon3 &polygon, FPolygon3 &polygon2);
		void CreateEdgeGeometry(const vtLevel *pLev, const FPolygon3 &polygon1,
									  const FPolygon3 &polygon2, int iEdge, bool bShowEdge);
		void AddWallNormal(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		void AddWindowSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		void AddDoorSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		int FindVertex(FPoint3 Point, FLine3 &RoofSection3D, vtArray<int> &iaVertices);
		~GenerateBuildingGeometry() {}
		const vtBuilding3d& m_Building;
		osg::ref_ptr<vtGeode> m_pGeode;

		// abstract memebers
		osg::Node *vtStructure3d::GetContained(void) { return NULL; }
		bool vtStructure3d::CreateNode(vtTerrain *) { return false; }
		bool vtStructure3d::IsCreated(void) { return false; }
		void vtStructure3d::DeleteNode(void) {}
	};
	class osg::Geometry* FindOrCreateGeometryObject(osg::Geode *pGeode, vtMaterial& Material, const int ArraysRequired);
	class osg::PrimitiveSet* FindOrCreatePrimitiveSet(osg::Geometry* pGeometry, const osg::PrimitiveSet::Mode Mode, const osg::PrimitiveSet::Type Type);
};
#endif	// VTOSG_GEOMETRYUTILSH

