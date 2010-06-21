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
#ifndef __062010__CVIDEOFRAME_H
   #define __062010__CVIDEOFRAME_H

#include <QFrame>
#include <QMouseEvent>

/********************************************************************\
|  Class: CVideoFrame
|  Date:  21.06.2010 / 11:00:28
|  Author: Jo2003
|  Description: extend QFrame for video functions
|
\********************************************************************/
class CVideoFrame : public QFrame
{
   Q_OBJECT

public:
   CVideoFrame(QWidget * parent = NULL, Qt::WindowFlags f = 0);
   virtual ~CVideoFrame();

protected:
   virtual void mouseDoubleClickEvent(QMouseEvent *pEvent);

signals:
   void sigToggleFullscreen ();
};

#endif // __062010__CVIDEOFRAME_H
/************************* History ***************************\
| $Log$
\*************************************************************/
