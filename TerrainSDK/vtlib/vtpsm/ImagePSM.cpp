//
// ImagePSM.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

vtImage::vtImage(const char *fname, int internalformat) : PSImage(fname)
{
//	m_internalformat = internalformat;
}

vtImage::vtImage(vtDIB *pDIB, int internalformat)
{
//	m_internalformat = internalformat;
	SetWidth(pDIB->GetWidth());
	SetHeight(pDIB->GetHeight());
	SetDepth(pDIB->GetDepth());
	SetBitmap(IMAGE_DIB, pDIB->GetDIBHeader());
}

#ifdef VTLIB_PSMDIB
/*
 * Called from the image load thread to read a .BMP from local disk.
 * A load image event (EVENT_LoadImage) will be generated by the load thread.
 * Any object that wants to know when the DIB has finished loading can observe
 * EVENT_LoadImage events; the OnEvent routine will be called for every image loaded.
 * The Object of the event will be the vtDIB object loaded, the event FileName will be
 * the name of the .BMP file.
 *
 * To cause a DIB to load asynchronously:
 *
 *	vtDIB* pdib = new vtDIB;
 *	PSWorld3D::Get()->LoadAsync(filename, vtDIB);
 *
 * To enable this function, override to PSWorld3D::OnInit() as follows:
 * myWorld::OnInit()
 * {
 *		if (PSWorld3D::OnInit())
 *		{
 *			m_LoadQueue->SetFileFunc("bmp", &psm_ReadDIB);
 *			return true;
 *		}
 *		return false;
 * }
 */
bool	_psm_ReadDIB(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	vtDIB* dib = (vtDIB*) (PSObj*) event->Object;	// this cast won't work unless vtDIB is a PSObj
	event->Code = EVENT_LoadImage;
	event->FileName = filename;
	if (dib == NULL)
	   {
		dib = new vtDIB();
		event->Object = dib;
	   }
	dib->ReadBMP(filename);
	return true;
}
#endif