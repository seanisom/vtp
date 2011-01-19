#include "vtlib/vtlib.h"
#include <osgDB/ReadFile>
#include <osg/PagedLOD>
#include <osgTerrain/TerrainTile>
#include <osgSim/HeightAboveTerrain>
#include "ExternalHeightField3d.h"

vtExternalHeightField3d::vtExternalHeightField3d(void)
{
	m_pHat = NULL;
}

vtExternalHeightField3d::~vtExternalHeightField3d(void)
{
	if (NULL != m_pHat)
		delete m_pHat;
}

bool vtExternalHeightField3d::Initialize(const char *external_data)
{
	// OSG doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(external_data);
	osg::Node *pNode = osgDB::readNodeFile((const char *)fname_local);
	if (NULL == pNode)
		return false;
	// Top level node may be a coordinate system node
	osg::CoordinateSystemNode *pCoordSystem = dynamic_cast<osg::CoordinateSystemNode*>(pNode);
	// Look for the PagedLOD node
	osg::PagedLOD *pLod;
	if (NULL != pCoordSystem)
		pLod = dynamic_cast<osg::PagedLOD*>(pCoordSystem->getChild(0));
	else
		pLod = dynamic_cast<osg::PagedLOD*>(pNode);
	if (NULL == pLod)
		return false;
	m_pLOD = pLod;
	osgTerrain::TerrainTile *pTopTile = dynamic_cast<osgTerrain::TerrainTile*>(pLod->getChild(0));
	if (NULL == pTopTile)
		return false;
	m_pLayer = pTopTile->getElevationLayer();
	if (osgTerrain::Locator::GEOCENTRIC == m_pLayer->getLocator()->getCoordinateSystemType())
		// I have not worked out whether I need to do something about the ellisoidal model yet
		return false;
	// Use coordinate system of top tile for conversions
	m_Projection.SetTextDescription((const char *)"wkt", m_pLayer->getLocator()->getCoordinateSystem().c_str());
	osg::BoundingBox bb;
	unsigned int numColumns = m_pLayer->getNumColumns();
	unsigned int numRows = m_pLayer->getNumRows();
	osg::Vec3d Local;
	for(unsigned int r = 0; r<numRows; ++r)
	{
		for(unsigned int c = 0;c<numColumns;++c)
		{
			float value = 0.0f;
			bool validValue = m_pLayer->getValidValue(c,r, value);
			if (validValue) 
			{
				Local.x() = 0; 
				Local.y() = 0;
				Local.z() = value;

				bb.expandBy(Local);
			}
		}
	}
	// OSG local cordinates are in range 0 to 1
	osg::Vec3d ModelBottomLeft, ModelTopRight;
	m_pLayer->getLocator()->convertLocalToModel(Local, ModelBottomLeft);
	Local.x() = 1;
	Local.y() = 1;
	m_pLayer->getLocator()->convertLocalToModel(Local, ModelTopRight);
	// This OSG Model coordinate system can be either projected, geographic (lat/long), or geocentric (geo xyz).
	// I do not handle geocentric at the moment(see above)
	// So I treat projected and geographic as basically the same as VTP Earth coordinates but Y up
	vtHeightField3d::Initialize(m_Projection.GetUnits(), DRECT(ModelBottomLeft.x(), ModelTopRight.y(), ModelTopRight.x(), ModelBottomLeft.y()), bb.zMin(), bb.zMax());
	// Computer a matrix to take OSG model coordinates - Left Handed Z up origin the origin of the coordinate system (obvious!(
	// into VTP world - Left Handed Y up origin always at the bottom left of the terrain extents.
	// Translate to origin
	m_TransfromOSGModel2VTPWorld.set(osg::Matrix::translate(osg::Vec3(-ModelBottomLeft.x(), -ModelBottomLeft.y(), 0)));
	// Spin Y up
	m_TransfromOSGModel2VTPWorld.postMult(osg::Matrix::rotate(-PID2d, osg::Vec3(1,0,0)));
	m_TransformVTPWorld2OSGModel.invert(m_TransfromOSGModel2VTPWorld);

	osg::MatrixTransform *transform = new osg::MatrixTransform;
	m_pNode = transform;
	transform->setName("Translate then spin Y up");
	transform->setMatrix(m_TransfromOSGModel2VTPWorld);
	transform->addChild(pNode);
	transform->setDataVariance(osg::Object::STATIC);

	m_pHat = new osgSim::HeightAboveTerrain;
	if (NULL == m_pHat)
		return false;
	return true;
}

vtNode* vtExternalHeightField3d::CreateGeometry()
{
	vtNode *pNode = new vtNativeNode(m_pNode.get());
	return pNode;
}

vtProjection &vtExternalHeightField3d::GetProjection()
{
	return m_Projection;
}

const vtProjection &vtExternalHeightField3d::GetProjection() const
{
	return m_Projection;
}


bool vtExternalHeightField3d::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	FPoint3 VTPWorld;
	m_Conversion.ConvertFromEarth(DPoint3(p.x, p.y, 0.0), VTPWorld);
	return FindAltitudeAtPoint(VTPWorld, fAltitude, bTrue);
}

bool vtExternalHeightField3d::FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	if (NULL != vNormal)
		return false; // Cannot do normals at the moment
	// Transform to OSG Model and then to OSG Local
	osg::Vec3d Model = osg::Vec3d(p3.x, p3.y, p3.z) * m_TransformVTPWorld2OSGModel;
	osg::Vec3d Local;
	m_pLayer->getLocator()->convertModelToLocal(Model, Local);
	// Check if over heightfield
	if (Local.x() < 0.0 || Local.x() > 1.0 || Local.y() < 0.0 || Local.y() > 1.0)
		return false;
	// Use HAT
	Model.z() = m_fMaxHeight + 1;
	if (m_pHat->getNumPoints() == 0)
		m_pHat->addPoint(Model);
	else
		m_pHat->setPoint(0, Model);
	if (bTrue)
		m_pHat->computeIntersections(m_pLOD);
	else
	{
		// Kill database reading
		osg::ref_ptr<osgSim::DatabaseCacheReadCallback> pCallback = m_pHat->getDatabaseCacheReadCallback();
		m_pHat->setDatabaseCacheReadCallback(NULL);
		m_pHat->computeIntersections(m_pLOD);
		m_pHat->setDatabaseCacheReadCallback(pCallback.get());
	}
	fAltitude = (float)(Model.z() - m_pHat->getHeightAboveTerrain(0));
	return true;
//		Just use the top level
//		return m_pLayer->getInterpolatedValue(Local.x(), Local.y(), fAltitude);
}

bool vtExternalHeightField3d::CastRayToSurface(const FPoint3 &point, const FPoint3 &dir, FPoint3 &result) const
{
	float alt;
	bool bOn = FindAltitudeAtPoint(point, alt);

	// special case: straight up or down
	float mag2 = sqrt(dir.x*dir.x+dir.z*dir.z);
	if (fabs(mag2) < .000001)
	{
		result = point;
		result.y = alt;
		if (!bOn)
			return false;
		if (dir.y > 0)	// points up
			return (point.y < alt);
		else
			return (point.y > alt);
	}

	if (bOn && point.y < alt)
		return false;	// already firmly underground

	unsigned int NumColumns = m_pLayer->getNumColumns();
	unsigned int NumRows = m_pLayer->getNumRows();
	osg::Vec3d Local((double)1.0/(double)(NumColumns - 1), double(1.0)/(double)(NumRows - 1), 0.0);
	osg::Vec3d Model;
	m_pLayer->getLocator()->convertLocalToModel(Local, Model);
	osg::Vec3d World = Model * m_TransfromOSGModel2VTPWorld;

	// adjust magnitude of dir until 2D component has a good magnitude
	float smallest = std::min(World.x(), -World.z());
	float adjust = smallest / mag2;
	FPoint3 dir2 = dir * adjust;

	bool found_above = false;
	FPoint3 p = point, lastp = point;
	while (true)
	{
		// are we out of bounds and moving away?
		if (p.x < m_WorldExtents.left && dir2.x < 0)
			return false;
		if (p.x > m_WorldExtents.right && dir2.x > 0)
			return false;
		if (p.z < m_WorldExtents.top && dir2.z < 0)
			return false;
		if (p.z > m_WorldExtents.bottom && dir2.z > 0)
			return false;

		bOn = FindAltitudeAtPoint(p, alt);
		if (bOn)
		{
			if (p.y > alt)
				found_above = true;
			else
				break;
		}
		lastp = p;
		p += dir2;
	}
	if (!found_above)
		return false;

	// now, do a binary search to refine the result
	FPoint3 p0 = lastp, p1 = p, p2;
	for (int i = 0; i < 10; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		int above = PointIsAboveTerrain(p2);
		if (above == 1)	// above
			p0 = p2;
		else if (above == 0)	// below
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}
