//
// SceneOSG.cpp
//
// Implementation of vtScene for the OSG library
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "StructureShadowsOSG.h"

#include <osg/PolygonMode>	// SetGlobalWireframe
#include <osgDB/Registry>	// for clearObjectCache

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <iostream>			// For redirecting OSG's stdout messages
#include "vtdata/vtLog.h"	// to the VTP log.

using namespace osg;

///////////////////////////////////////////////////////////////
// Trap for OSG messages
//

class OsgMsgTrap : public std::streambuf
{
public:
	inline virtual int_type overflow(int_type c = std::streambuf::traits_type::eof())
	{
		if (c == std::streambuf::traits_type::eof()) return std::streambuf::traits_type::not_eof(c);
		VTLOG1((char) c);
		return c;
	}
} g_Trap;

// preserve and restore
static std::streambuf *previous_cout;
static std::streambuf *previous_cerr;

///////////////////////////////////////////////////////////////

// There is always and only one global vtScene object
vtScene g_Scene;

vtScene::vtScene() : vtSceneBase()
{
	m_bInitialized = false;
	m_bWireframe = false;
	m_bWinInfo = false;
	m_pHUD = NULL;
}

vtScene::~vtScene()
{
	// Do not release camera or window, that is left for the application.
}

vtScene *vtGetScene()
{
	return &g_Scene;
}

float vtGetTime()
{
	return g_Scene.GetTime();
}

float vtGetFrameTime()
{
	return g_Scene.GetFrameTime();
}

int vtGetMaxTextureSize()
{
	GLint tmax = 0;	// TODO: cannot make direct GL calls in threaded environment
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmax);
	return tmax;
}

/**
 * Initialize the vtlib library, including the display and scene graph.
 * You should call this function only once, before any other vtlib calls.
 *
 * \param bStereo True for a stereo display output.
 * \param iStereoMode Currently for vtosg, supported values are 0 for
 *		Anaglyphic (red-blue) and 1 for Quad-buffer (shutter glasses).
 */
bool vtScene::Init(bool bStereo, int iStereoMode)
{
	// Redirect cout messages (where OSG sends its messages) to our own log
	previous_cout =  std::cout.rdbuf(&g_Trap);
	previous_cerr = std::cerr.rdbuf(&g_Trap);

#if 0
	// If you encounter trouble in OSG that you want to debug, enable this
	//  to get a LOT of diagnostic messages from OSG.
	osg::setNotifyLevel(osg::INFO);
#endif

	m_pDefaultCamera = new vtCamera;
	m_pDefaultWindow = new vtWindow;
	SetCamera(m_pDefaultCamera);
	AddWindow(m_pDefaultWindow);

	// OSG's display setting include stereo
	osg::DisplaySettings* displaySettings = new osg::DisplaySettings;
	if (bStereo)
	{
		displaySettings->setStereo(true);
		//? displaySettings->setScreenDistance(2);
		osg::DisplaySettings::StereoMode mode;
		if (iStereoMode == 0) mode = osg::DisplaySettings::ANAGLYPHIC;
		if (iStereoMode == 1) mode = osg::DisplaySettings::QUAD_BUFFER;
		displaySettings->setStereoMode(mode);
	}
	// Allow the user to also use OSG's environment variables to
	//  override/extend the settings.
	displaySettings->readEnvironmentalVariables();

	m_pOsgSceneView = new osgUtil::SceneView(displaySettings);

	// From the OSG mailing list: You must specify the lighting mode in
	// setDefaults() and override the default options. If you call
	// setDefaults() with the default options, a headlight is added to the
	// global state set of the SceneView.  With the default options applied,
	// I have tried subsequently calling setLightingMode(NO_SCENE_LIGHT)
	// and setLight(NULL), but I still get a headlight.
	m_pOsgSceneView->setDefaults(osgUtil::SceneView::NO_SCENEVIEW_LIGHT);

	// OSG 0.9.0 and newer
	m_pOsgSceneView->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

	// we are emphatic about this: no sceneview light!
	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::NO_SCENEVIEW_LIGHT);

	// OSG 0.9.2 and newer: turn off "small feature culling"
	m_pOsgSceneView->setCullingMode( m_pOsgSceneView->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

	// enable lighting by default.
	osg::StateSet *ss = m_pOsgSceneView->getGlobalStateSet();
	ss->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	m_bInitialized = true;

	_initialTick = _timer.tick();
	_frameTick = _initialTick;

	return true;
}

void vtScene::Shutdown()
{
	VTLOG("vtScene::Shutdown\n");
	m_pDefaultCamera->Release();
	delete m_pDefaultWindow;
	vtNode::ClearOsgModelCache();
	vtImageCacheClear();
	osgDB::Registry::instance()->clearObjectCache();

	// restore
	std::cout.rdbuf(previous_cout);
	std::cerr.rdbuf(previous_cerr);
}

void vtScene::TimerRunning(bool bRun)
{
	if (!bRun)
	{
		// stop timer, count how much running time has already elapsed
		m_fAccumulatedFrameTime = _timer.delta_s(_lastRunningTick,_timer.tick());
		//VTLOG("partial frame: %lf seconds\n", m_fAccumulatedFrameTime);
	}
	else
		// start again
		_lastRunningTick = _timer.tick();
}

void vtScene::UpdateBegin()
{
	_lastFrameTick = _frameTick;
	_frameTick = _timer.tick();

	// finish counting the split frame's elapsed time
	if (_lastRunningTick != _lastFrameTick)
	{
		m_fAccumulatedFrameTime += _timer.delta_s(_lastRunningTick,_frameTick);
		//VTLOG("   full frame: %lf seconds\n", m_fAccumulatedFrameTime);
		m_fLastFrameTime = m_fAccumulatedFrameTime;
	}
	else
		m_fLastFrameTime = _timer.delta_s(_lastFrameTick,_frameTick);

	_lastRunningTick = _frameTick;
}

void vtScene::UpdateEngines()
{
	if (!m_bInitialized) return;
	DoEngines();
}

void vtScene::UpdateWindow(vtWindow *pWindow)
{
	if (!m_bInitialized) return;

	// window background color
	Vec4 color2;
	v2s(pWindow->GetBgColor(), color2);
	m_pOsgSceneView->setClearColor(color2);

	// window size
	IPoint2 winsize = pWindow->GetSize();
	if (winsize.x == 0 || winsize.y == 0)
		VTLOG("Warning: winsize %d %d\n", winsize.x, winsize.y);
	m_pOsgSceneView->setViewport(0, 0, winsize.x, winsize.y);

	// As of OSG 0.9.5, we need to store our own camera params and recreate
	//  the projection matrix each frame.
	float aspect;
	if (winsize.x == 0 || winsize.y == 0)		// safety
		aspect = 1.0;
	else
		aspect = (float) winsize.x / winsize.y;

	if (m_pCamera->IsOrtho())
	{
		// Arguments are left, right, bottom, top, zNear, zFar
		float w2 = m_pCamera->GetWidth() /2;
		float h2 = w2 / aspect;
		m_pOsgSceneView->setProjectionMatrixAsOrtho(-w2, w2, -h2, h2,
			m_pCamera->GetHither(), m_pCamera->GetYon());
	}
	else
	{
		float fov_x = m_pCamera->GetFOV();
		float a = tan (fov_x/2);
		float b = a / aspect;
		float fov_y_div2 = atan(b);
		float fov_y_deg = RadiansToDegrees(fov_y_div2 * 2);

		m_pOsgSceneView->setProjectionMatrixAsPerspective(fov_y_deg,
			aspect, m_pCamera->GetHither(), m_pCamera->GetYon());
	}

	// And apply the rotation and translation of the camera itself
	const osg::Matrix &mat2 = m_pCamera->GetOsgTransform()->getMatrix();
	osg::Matrix imat;
	imat.invert(mat2);
	m_pOsgSceneView->setViewMatrix(imat);

	CalcCullPlanes();

	// We don't actually need OSG's "update" functionality, but enabling it
	//  avoids some potential crashing if they try to load something like a
	//  OSG file with particle effects in it.
#define USE_OSG_UPDATE 1

#if USE_OSG_UPDATE
	// record the timer tick at the start of rendering.
	static osg::Timer_t start_tick = osg::Timer::instance()->tick();
	static unsigned int frameNum = 0;

	// set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
	frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
	frameStamp->setFrameNumber(frameNum++);

	// pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
	m_pOsgSceneView->setFrameStamp(frameStamp.get());
#endif

#if USE_OSG_UPDATE
	m_pOsgSceneView->update();
#endif

	m_pOsgSceneView->cull();
	m_pOsgSceneView->draw();
}

/**
 * Compute the full current view transform as a matrix, which includes
 * the projection of the camera and the transform to window coordinates.
 *
 * This transform is the one used to convert XYZ points in world coodinates
 * into XY window coordinates.
 *
 * By inverting this matrix, you can "un-project" window coordinates back
 * into the world.
 *
 * \param mat This matrix will receive the current view transform.
 */
void vtScene::ComputeViewMatrix(FMatrix4 &mat)
{
	osg::Matrix _viewMatrix = m_pOsgSceneView->getViewMatrix();
	osg::Matrix _projectionMatrix = m_pOsgSceneView->getProjectionMatrix();
	osg::Matrix matrix( _viewMatrix * _projectionMatrix);
        
	osg::Viewport *_viewport = m_pOsgSceneView->getViewport();
    if (_viewport != NULL)
        matrix.postMult(_viewport->computeWindowMatrix());

	ConvertMatrix4(&matrix, &mat);
}

void vtScene::DoUpdate()
{
	UpdateBegin();
	UpdateEngines();
	UpdateWindow(GetWindow(0));
}

void vtScene::SetRoot(vtGroup *pRoot)
{
	if (pRoot)
		m_pOsgSceneRoot = pRoot->GetOsgGroup();
	else
		m_pOsgSceneRoot = NULL;

	// Clear out any shadow stuff
	m_pStructureShadowsOSG = NULL;

	if (m_pOsgSceneView != NULL)
		m_pOsgSceneView->setSceneData(m_pOsgSceneRoot.get());
	m_pRoot = pRoot;
}

bool vtScene::IsStereo() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgSceneView->getDisplaySettings();
	return displaySettings->getStereo();
}

void vtScene::SetStereoSeparation(float fSep)
{
	osg::DisplaySettings* displaySettings = m_pOsgSceneView->getDisplaySettings();
	displaySettings->setEyeSeparation(fSep);
}

float vtScene::GetStereoSeparation() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgSceneView->getDisplaySettings();
	return displaySettings->getEyeSeparation();
}

/**
 * Convert window coordinates (in pixels) to a ray from the camera
 * in world coordinates.  Pixel coordinates are measured from the
 * top left corner of the window: X right, Y down.
 */
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);

	Vec3 near_point, far_point, diff;

	// call the handy OSG function
	IPoint2 winsize = pWindow->GetSize();
	m_pOsgSceneView->projectWindowXYIntoObject(win.x, winsize.y-1-win.y, near_point, far_point);

	diff = far_point - near_point;
	diff.normalize();

	s2v(near_point, pos);
	s2v(diff, dir);

	return true;
}

void vtScene::WorldToScreen(const FPoint3 &point, IPoint2 &result)
{
	Vec3 object;
	v2s(point, object);
	Vec3 window;
	m_pOsgSceneView->projectObjectIntoWindow(object, window);
	result.x = (int) window.x();
	result.y = (int) window.y();
}


// Debugging helper
void LogCullPlanes(FPlane *planes)
{
	for (int i = 0; i < 4; i++)
		VTLOG(" plane %d: %.3f %.3f %.3f %.3f\n", i, planes[i].x, planes[i].y, planes[i].z, planes[i].w);
	VTLOG("\n");
}

void vtScene::CalcCullPlanes()
{
#if 0
	// Non-API-Specific code - will work correctly as long as the Camera
	// methods are fully functional.
	FMatrix4 mat;
	m_pCamera->GetTransform1(mat);

	assert(( m_WindowSize.x > 0 ) && ( m_WindowSize.y > 0 ));

	double fov = m_pCamera->GetFOV();

	double aspect = (float)m_WindowSize.y / m_WindowSize.x;
	double hither = m_pCamera->GetHither();

	double a = hither * tan(fov / 2);
	double b = a * aspect;

	FPoint3 vec_l(-a, 0, -hither);
	FPoint3 vec_r(a, 0, -hither);
	FPoint3 vec_t(0, b, -hither);
	FPoint3 vec_b(0, -b, -hither);

	vec_l.Normalize();
	vec_r.Normalize();
	vec_t.Normalize();
	vec_b.Normalize();

	FPoint3 up(0.0f, 1.0f, 0.0f);
	FPoint3 right(1.0f, 0.0f, 0.0f);

	FPoint3 temp;

	FPoint3 center;
	FPoint3 norm_l, norm_r, norm_t, norm_b;

	temp = up.Cross(vec_l);
	mat.TransformVector(temp, norm_l);

	temp = vec_r.Cross(up);
	mat.TransformVector(temp, norm_r);

	temp = right.Cross(vec_t);
	mat.TransformVector(temp, norm_t);

	temp = vec_b.Cross(right);
	mat.TransformVector(temp, norm_b);

	mat.Transform(FPoint3(0.0f, 0.0f, 0.0f), center);

	// set up m_cullPlanes in world coordinates!
	m_cullPlanes[0].Set(center, norm_l);
	m_cullPlanes[1].Set(center, norm_r);
	m_cullPlanes[2].Set(center, norm_t);
	m_cullPlanes[3].Set(center, norm_b);
#else
	// Get the view frustum clipping planes directly from OSG.
	// We can't get the planes from the state, because the state
	//  includes the funny modelview matrix used to scale the
	//  heightfield.  We must get it from the 'scene' instead.

	const osg::Matrixd &_projection = m_pOsgSceneView->getProjectionMatrix();
	const osg::Matrixd &_modelView = m_pOsgSceneView->getViewMatrix();

	Polytope tope;
	tope.setToUnitFrustum();
	tope.transformProvidingInverse((_modelView)*(_projection));

	const Polytope::PlaneList &planes = tope.getPlaneList();

	int i = 0;
	for (Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr)
	{
		// make a copy of the clipping plane
		Plane plane = *itr;

		// extract the OSG plane to our own structure
		Vec4 pvec = plane.asVec4();
		m_cullPlanes[i++].Set(-pvec.x(), -pvec.y(), -pvec.z(), -pvec.w());
	}
#endif

	// For debugging
//	LogCullPlanes(m_cullPlanes);
}


void vtScene::DrawFrameRateChart()
{
	static float fps[100];
	static int s = 0;
	fps[s] = GetFrameRate();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.0f);

	glBegin(GL_LINE_STRIP);
	for (int i = 1; i <= 100; i++)
	{
		glVertex3f(-1.0 + i/200.0f, -1.0f + fps[(s+i)%100]/200.0f, 0.0f);
	}
	glEnd();

	s++;
	if (s == 100) s = 0;
}

void vtScene::SetGlobalWireframe(bool bWire)
{
	m_bWireframe = bWire;

	// Set the scene's global PolygonMode attribute, which will affect all
	// other materials in the scene, except those which explicitly override
	// the attribute themselves.
	StateSet *global_state = m_pOsgSceneView->getGlobalStateSet();
	PolygonMode *npm = new PolygonMode();
	if (m_bWireframe)
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
	else
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::FILL);
	global_state->setAttributeAndModes(npm, StateAttribute::ON);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}

void vtScene::SetWindowSize(int w, int h, vtWindow *pWindow)
{
	if (!m_bInitialized) return;
	if (m_pHUD)
		m_pHUD->SetWindowSize(w, h);
	vtSceneBase::SetWindowSize(w, h, pWindow);
}


void vtScene::ShadowVisibleNode(vtNode *node, bool bVis)
{
	if (m_pStructureShadowsOSG.valid())
		if (bVis)
			m_pStructureShadowsOSG->ExcludeFromShadower(node->GetOsgNode(), false);
		else
			m_pStructureShadowsOSG->ExcludeFromShadower(node->GetOsgNode(), true);
}

void vtScene::SetShadowedNode(vtTransform *pLight, vtNode *pShadowNode,
							  vtTransform *pTransform, int iRez, float fDarkness,
							  int iTextureUnit)
{
	m_pStructureShadowsOSG = new CStructureShadowsOSG;
	m_pStructureShadowsOSG->Initialise(m_pOsgSceneView.get(),
		pShadowNode->GetOsgNode(), pTransform->GetOsgNode(), iRez, fDarkness,
		iTextureUnit);
	m_pStructureShadowsOSG->SetSunPosition(v2s(-pLight->GetDirection()), true);
}

void vtScene::UnsetShadowedNode(vtTransform *pTransform)
{
	m_pStructureShadowsOSG = NULL;
}

void vtScene::UpdateShadowLightDirection(vtTransform *pLight)
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->SetSunPosition(v2s(-pLight->GetDirection()));
}

void vtScene::SetShadowDarkness(float fDarkness)
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->SetShadowDarkness(fDarkness);
}

////////////////////////////////////////

// Helper fn for dumping an OSG scenegraph
void printnode(osg::Node *node, int tab)
{
	for (int i = 0; i < tab*2; i++) {
	   osg::notify(osg::WARN) << " ";
	}
	osg::notify(osg::WARN) << node->className() << " - " << node->getName() << " @ " << node << std::endl;
	osg::Group *group = node->asGroup();
	if (group) {
		for (unsigned int i = 0; i < group->getNumChildren(); i++) {
			printnode(group->getChild(i), tab+1);
		}
	}
}

///////////////////////////////////////////////////////////////////////

const vtStringArray &vtGetDataPath()
{
	return g_Scene.m_DataPaths;
}
