/*********************** Information *************************\
| $HeadURL$
|
| Author: Jo2003
|
| Begin: 20.06.2010 / 19:50:35
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#include "cvideoframe.h"

/* -----------------------------------------------------------------\
|  Method: CVideoFrame / constructor
|  Begin: 20.06.2010 / 19:05:00
|  Author: Jo2003
|  Description: create object, init values
|
|  Parameters: pointer to parent window, windows flags
|
|  Returns: --
\----------------------------------------------------------------- */
CVideoFrame::CVideoFrame(QWidget * parent, Qt::WindowFlags f)
   : QFrame(parent, f)
{

}

CVideoFrame::~CVideoFrame()
{

}

void CVideoFrame::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
   emit sigToggleFullscreen();
   pEvent->accept();
}

/************************* History ***************************\
| $Log$
\*************************************************************/
