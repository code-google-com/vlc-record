#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include "cvlcrecdb.h"
#include "qftsettings.h"

#ifdef DINCLUDEPLUGS
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsqlite)
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
    QTranslator  trans[Translators::TRANS_MAX];
    QApplication::installTranslator (&trans[Translators::TRANS_QT]);
    QApplication::installTranslator (&trans[Translators::TRANS_OWN]);

    pFolders = new CDirStuff();

    if (pFolders)
    {
       if (pFolders->isInitialized ())
       {
          pDb = new CVlcRecDB();

          if (pDb)
          {
              // check if needed settings are there ...
              if ((pDb->stringValue("User") == "")
                 && (pDb->stringValue("Passwd") == ""))
              {
                 QFTSettings ftSet(NULL, trans);
                 ftSet.exec();
              }

              MainWindow w(trans);
              w.show();

             iRV = app.exec ();

             delete pDb;
          }
       }

       delete pFolders;
    }

    return iRV;
}
