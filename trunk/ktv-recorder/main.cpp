#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include "cvlcrecdb.h"
#include "cshowinfo.h"

#ifdef DINCLUDEPLUGS
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsqlite)
// Q_IMPORT_PLUGIN(qgif)
// Q_IMPORT_PLUGIN(qico)
#endif // DINCLUDEPLUGS

#ifdef Q_WS_X11
   #include <X11/Xlib.h>
#endif

// make logging class available everywhere ...
CLogFile VlcLog;

// make directory names available globally ...
CDirStuff *pFolders;

// db storage class must be global ...
CVlcRecDB *pDb;

// make show info global available ...
CShowInfo showInfo;

int main(int argc, char *argv[])
{
    // bugfix for crash on exit on *nix ...
 #ifdef Q_WS_X11
     XInitThreads();
 #endif

    int          iRV = -1;
    QApplication app(argc, argv);
    app.setOrganizationName("Joerg");
    app.setApplicationName("KTV-Recorder");
    QTranslator  trans;
    QApplication::installTranslator (&trans);

    pFolders = new CDirStuff();

    if (pFolders)
    {
       if (pFolders->isInitialized ())
       {
          pDb = new CVlcRecDB();

          if (pDb)
          {
              MainWindow w(&trans);
              w.show();

             iRV = app.exec ();

             delete pDb;
          }
       }

       delete pFolders;
    }

    return iRV;
}
