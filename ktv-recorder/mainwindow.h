#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QScrollBar>
#include <QStatusBar>
#include <QWindowStateChangeEvent>
#include <QMenu>
#include "caboutdialog.h"
#include "csettingsdlg.h"
#include "cchannelsepgdlg.h"
#include "ctimerrec.h"
#include "ckartinaclnt.h"
#include "ckartinaxmlparser.h"
#include "ctranslit.h"
#include "cinstruction.h"
#include "cparentalcontroldlg.h"
#include "qseccodedlg.h"
#ifdef INCLUDE_LIBVLC
    #include <QStackedLayout>
    #include "qvlcvideowidget.h"
#endif

//------------------------------------------------------------------
/// \name definition of start flags
//------------------------------------------------------------------
// @{
#define FLAG_INITDIALOG     (ulong)(1<<0) ///< should we run initDialog()
#define FLAG_CONN_CHAIN     (ulong)(1<<1) ///< should we start connection chain
#define FLAG_CHAN_LIST      (ulong)(1<<2) ///< should we set channel from former session
#define FLAG_EPG_DAY        (ulong)(1<<3) ///< should we set epg day from former session
#define FLAG_CLOGO_COMPL    (ulong)(1<<4) ///< channel logos completely loaded
// @}

namespace Ui
{
    class MainWindow;

    struct SShortCuts
    {
       QString     sDescr;
       QObject    *pObj;
       const char *pSlot;
       QString     sShortCut;
    };

    struct SVodSite
    {
       QString sContent;
       int     iScrollBarVal;
    };

    struct STabWidget
    {
        QString  sText;
        QIcon    icon;
        int      iPos;
        QWidget *pWidget;
    };
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QTranslator *trans = 0, QWidget *parent = 0);
    ~MainWindow();

public slots:
    virtual void exec();

private:
    Ui::MainWindow 		  *ui;
    CChannelsEPGdlg               *pChannelDlg;
    CSettingsDlg                   dlgSettings;
    CParentalControlDlg            dlgParentalControl;
    QSecCodeDlg                    secCodeDlg;
    CKartinaClnt                   KartinaTv;
    CKartinaXMLParser              XMLParser;
    CWaitTrigger                   Trigger;
    CStreamLoader                  streamLoader;
    QTranslator                   *pTranslator;
    QTimer                         Refresh;
    QTimer                         tEpgRefresh;
    CPixLoader                      pixCache;
    CTimerRec                      dlgTimeRec;
    CVlcCtrl                       vlcCtrl;
    CTranslit                      translit;
    IncPlay::ePlayStates           ePlayState;
    QVector<CShortcutEx *>         vShortcutPool;
    int                            iDwnReqId;
    QSystemTrayIcon                trayIcon;
    QRect                          sizePos;
    bool                           bLogosReady;
    bool                           bDoInitDlg;
    bool                           bFirstConnect;
    bool                           bVODLogosReady;
    bool                           bOnTop;
    bool                           bFirstInit;
    bool                           bSetRecentChan;
    bool                           bShortCuts;
    bool                           bGotVOD; // update vod stuff only at startup ...
    Qt::WindowFlags                flags;
    QMenu                         *ChanGroup[MAX_CHANNEL_GROUPS]; // define in defdef.h
    QAction                       *ChannelActs[MAX_CHANNEL_ACTS]; // define in defdef.h
    QAction                       *RecentChansActs[MAX_RECENT_CHANNELS]; // define in defdef.h
    QAction                       *Aspect[MAX_ASPECTS]; // define in defdef.h
    QAction                       *Crop[MAX_CROPS]; // define in defdef.h
    QActionGroup                  *pAspectGroup;
    QActionGroup                  *pCropGroup;
    QVector<Ui::SShortCuts>        vShortCutTab;
    cparser::SAccountInfo          accountInfo;
    cparser::SGenreInfo            genreInfo;
    ulong                          ulStartFlags;
    QNetworkAccessManager         *pUpdateChecker;
    Ui::SVodSite                   lastVodSite;
    Ui::STabWidget                 vodTabWidget;
    Kartina                        metaKartina;
    QVector<cparser::SEpg>         vEpgList;
    bool                           bEpgRefresh;

    #ifdef INCLUDE_LIBVLC
        QStackedLayout                 *stackedLayout;
        QVlcVideoWidget                *pVideoWidget;
    #endif //INCLUDE_LIBVLC

protected:
    int StartVlcRec (const QString &sURL, const QString &sChannel);
    int StartVlcPlay (const QString &sURL);
    void StartStreamDownload (const QString &sURL, const QString &sName, const QString &sFileExt = "ts");
    void TouchPlayCtrlBtns (bool bEnable = true);
    QString CleanShowName (const QString &str);
    bool WantToStopRec ();
    void FillChanMap (const QVector<cparser::SChan> &chanlist);
    int FillChannelList (const QVector<cparser::SChan> &chanlist);
    int  CheckCookie (const QString &cookie);
    int  AllowAction (IncPlay::ePlayStates newState);
    bool TimeJumpAllowed ();
    void InitShortCuts ();
    void ClearShortCuts ();
    void ConnectionInit();
    void CreateSystray ();
    void initDialog();
    void setRecentChannel(const QString &ChanName);
    void updateRecentChanActions();
    int getChanId(const QString &chanName);
    void contextMenuEvent(QContextMenuEvent *event);
    void retranslateShortcutTable();
    void fillShortCutTab();
    int  grantAdultAccess (bool bProtected);

    virtual void changeEvent(QEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void showEvent (QShowEvent * event);
    virtual void hideEvent (QHideEvent * event);
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void on_actionParental_Control_triggered();
    void on_actionOne_Click_Play_triggered();
    void on_actionShow_Live_triggered();
    void on_pushLive_clicked();
    void on_actionGuid_triggered();
    void on_actionShow_Status_Bar_triggered();
    void on_actionClear_Recent_Channel_List_triggered();
    void on_actionJumpForward_triggered();
    void on_actionJumpBackward_triggered();
    void on_actionAlways_On_Top_triggered();
    void on_actionShow_Lower_Tools_Panel_triggered();
    void on_actionShow_Upper_Tools_Panel_triggered();
    void on_actionShow_Channel_Information_triggered();
    void on_actionStop_triggered();
    void on_actionRecord_triggered();
    void on_actionPlay_triggered();
    void on_actionChannelsEPG_triggered();
    void on_actionSettings_triggered();
    void on_actionTime_Record_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();

#ifdef INCLUDE_LIBVLC
    void on_pushBwd_clicked();
    void on_pushFwd_clicked();
    void slotToogleFullscreen();
#endif /* INCLUDE_LIBVLC */
    void on_pushStop_clicked();
    void on_pushPlay_clicked();
    void on_pushRecord_clicked();
    void slotChanList (const QString &str);
    void slotEPG(const QString &str);
    void slotStreamURL (const QString &str);
    void slotArchivURL (const QString &str);
    void slotCookie (const QString &str);
    void slotTimeShift (const QString &str);
    void slotEpgAnchor (const QUrl & link);
    void slotReloadLogos ();
    void slotSetSServer (QString sIp);
    void slotTimerRecActive (int iState);
    void slotTimerRecordDone ();
    void slotVlcStarts (int iState);
    void slotVlcEnds (int iState);
    void slotShutdown ();
    void slotSplashScreen ();
    void slotIncPlayState (int);
    void slotLogout (const QString &str);
    void slotDownloadStarted (int id, QString sFileName);
    void slotGotVodGenres (const QString &str);
    void slotGotVideos (const QString &str, bool bVodFavs = false);
    void slotVodAnchor (const QUrl &link);
    void slotGotVideoInfo (const QString &str);
    void slotVodURL(const QString &str);
    void slotSetBitrate (int iRate);
    void slotSetTimeShift (int iShift);
    void slotDoubleClick();
    void slotChannelDlgClosed();
    void slotSystrayActivated (QSystemTrayIcon::ActivationReason reason);
    void slotSelectChannel();
    void slotChannelDown();
    void slotChannelUp();
    void slotToggleEpgVod();
    void slotAspect();
    void slotCrop();
    void slotAspectToggle(int idx);
    void slotCropToggle(int idx);
    void slotUpdateAnswer (QNetworkReply* pRes);
    void slotCheckArchProg(ulong ulArcGmt);
    void slotEpgRefresh();

    void slotKartinaErr (const QString &str, int req, int err);
    void slotKartinaResponse(const QString& resp, int req);
    void slotUnused(const QString &str);
    void slotRefreshChanLogos ();
    void slotPCodeChangeResp (const QString &str);

signals:
    void sigToggleFullscreen ();
    void sigToggleAspectRatio ();
    void sigToggleCropGeometry ();
    void sigLCDStateChange (int iState);
    void sigJmpFwd ();
    void sigJmpBwd ();
    void sigShowInfoUpdated();
    void sigShow ();
    void sigHide ();
    void sigFullScreenToggled (int on);
    void sigEpgRefresh();
    void sigLockParentalManager();
};

#endif // MAINWINDOW_H

