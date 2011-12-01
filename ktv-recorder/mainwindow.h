#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QScrollBar>
#include "caboutdialog.h"
#include "csettingsdlg.h"
#include "cchannelsepgdlg.h"
#include "ctimerrec.h"
#include "cepgbrowser.h"
#include "cvodbrowser.h"
#include "ckartinaclnt.h"
#include "ckartinaxmlparser.h"
#include "cwaittrigger.h"
#include "cplayer.h"
#include "cvlcctrl.h"
#include "ctranslit.h"
#include "cshowinfo.h"
#include "cinstruction.h"

//------------------------------------------------------------------
/// \name definition of start flags
//------------------------------------------------------------------
// @{
#define FLAG_INITDIALOG     (ulong)(1<<0) ///< should we run initDialog()
#define FLAG_CONN_CHAIN     (ulong)(1<<1) ///< should we start connection chain
#define FLAG_CHAN_LIST      (ulong)(1<<2) ///< should we set channel from former session
#define FLAG_EPG_DAY        (ulong)(1<<3) ///< should we set epg day from former session
#define FLAG_CLOGOS_READY   (ulong)(1<<4) ///< are the channel logos ready
#define FLAG_VLOGOS_READY   (ulong)(1<<5) ///< are the VOD logos ready
// @}

namespace Ui
{
    class MainWindow;

    struct SShortCuts
    {
       QString     sDescr;
       QObject    *pObj;
//       QString     sObj;
       const char *pSlot;
       QString     sShortCut;
    };

    struct SVodSite
    {
       QString sContent;
       int     iScrollBarVal;
    };
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QTranslator *trans, QWidget *parent = 0);
    ~MainWindow();

public slots:
    virtual void exec();

private:
    Ui::MainWindow 		  *ui;
    CChannelsEPGdlg               *pChannelDlg;
    CSettingsDlg                   dlgSettings;
    CKartinaClnt                   KartinaTv;
    CKartinaXMLParser              XMLParser;
    CWaitTrigger                   Trigger;
    CStreamLoader                  streamLoader;
    QTranslator                   *pTranslator;
    QTimer                         Refresh;
    CPixLoader                     dwnLogos;
    CPixLoader                     dwnVodPics;
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
    QNetworkAccessManager          *pUpdateChecker;
    Ui::SVodSite                    lastVodSite;
    QMap<uint, epg::SShow>          archProgMap;

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
    QString createTooltip (const QString & name, const QString & prog, uint start, uint end);
    void retranslateShortcutTable();
    void fillShortCutTab();

    virtual void changeEvent(QEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void showEvent (QShowEvent * event);
    virtual void hideEvent (QHideEvent * event);
    virtual void closeEvent(QCloseEvent *event);

private slots:
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
#endif /* INCLUDE_LIBVLC */
    void on_pushStop_clicked();
    void on_pushPlay_clicked();
    void on_pushRecord_clicked();
    void slotErr (QString str);
    void slotChanList (QString str);
    void slotEPG(QString str);
    void slotStreamURL (QString str);
    void slotArchivURL (QString str);
    void slotServerForm (QString str);
    void slotCookie (QString str);
    void slotTimeShift (QString str);
    void slotEpgAnchor (const QUrl & link);
    void slotLogosReady ();
    void slotReloadLogos ();
    void slotSetSServer (QString sIp);
    void slotTimerRecActive (int iState);
    void slotTimerRecordDone ();
    void slotVlcStarts (int iState);
    void slotVlcEnds (int iState);
    void slotShutdown ();
    void slotSplashScreen ();
    void slotIncPlayState (int);
    void slotGotTimeShift (QString str);
    void slotLogout (QString str);
    void slotDownloadStarted (int id, QString sFileName);
    void slotGotVodGenres (QString str);
    void slotGotVideos (QString str);
    void slotVodAnchor (const QUrl &link);
    void slotGotVideoInfo (QString str);
    void slotVodURL(QString str);
    void slotGotBitrate (QString str);
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
//    void slotStartConnectionChain();
    void slotUpdateProgress (int iMin, int iMax, int iAct);
    void slotUpdateAnswer (QNetworkReply* pRes);
    void slotCheckArchProg(ulong ulArcGmt);

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
};

#endif // MAINWINDOW_H

