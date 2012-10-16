#ifndef CCHANNELSEPGDLG_H
#define CCHANNELSEPGDLG_H

#include <QDialog>
#include <QToolButton>
#include <QPushButton>
#include <QTabBar>
#include <QListWidget>
#include <QtGui/QComboBox>
#include <QDateTime>
#include <QProgressBar>
#include <QStandardItemModel>
#include "cepgbrowser.h"
#include "cvodbrowser.h"
#include "cvlcctrl.h"
#include "cfavaction.h"
#include "cpixloader.h"
#include "cwaittrigger.h"
#include "cshowinfo.h"
#include "qchanlistdelegate.h"

namespace Ui {
    class CChannelsEPGdlg;

    struct SChanListElement
    {
       int iIndex;
       QString sChannel;
       QString sURL;
    };
}

class CChannelsEPGdlg : public QDialog
{
    Q_OBJECT

public:
    CChannelsEPGdlg(QWidget *parent = 0);
    ~CChannelsEPGdlg();
    void initDialog(bool bInit);
    CEpgBrowser* getEpgBrowser();
    CVodBrowser* getVodBrowser();
    QComboBox* getCbxChannelGroup();
    QComboBox* getCbxLastOrBest();
    QTabBar* getNavBar();
    QLabel* getLabChanIcon();
    QLabel* getLabCurrDay();
    QLabel* getLabChanName();
    QComboBox* getCbxGenre();
    QList<int>* getListFav();
    CEpgBrowser* getTextEpg();
    int getEpgOffset();
    void setEpgOffset(int iEpgOffs);
    void setTrigger (CWaitTrigger *pTrig);
    void setProgressBar(QProgressBar* pProgBar);
    void setTextEpgShort(CEpgBrowser* pEpgShort);
    void CreateFav();
    void HandleFavourites();
    void CleanContextMenu();
    void setStatusBar(QStatusBar *pStBar);
    QTabWidget* getTabEpgVOD();
    void setSettings(CSettingsDlg *pSett);
    QStandardItemModel* getModel();
    int getCurrentCid();
    QListView* getChannelList();
    QChanMap* getChanMap();
    void updateFavourites();
    void touchLastOrBestCbx();
    void touchGenreCbx();
    void touchVodNavBar(const cparser::SGenreInfo &gInfo);
    void setGenreInfo(cparser::SGenreInfo *pGenrInf);
    void activateVOD();

private:
    Ui::CChannelsEPGdlg *ui;
    QProgressBar* pProgressBar;
    CEpgBrowser* pTextEpgShort;
    CWaitTrigger*                  pTrigger;
    CSettingsDlg*                  pSettings;
    CPixLoader                     dwnLogos;
    CPixLoader                     dwnVodPics;
    QTabBar                       *pEpgNavbar;
    CVlcCtrl                       vlcCtrl;
    QToolButton                   *pFavBtn[MAX_NO_FAVOURITES];
    CFavAction                    *pFavAct[MAX_NO_FAVOURITES];
    QMenu                          favContext;
    CFavAction                    *pContextAct[MAX_NO_FAVOURITES];
    int                            iCurrentRow;
    QList<int>                     lFavourites;
    QChanMap                       chanMap;
    int                            iFontSzChg;
    int                            iEpgOffset;
    IncPlay::ePlayStates           ePlayState;
    bool                           bSaveCng;
    QStatusBar                    *pStatusBar;
    QStandardItemModel            *pModel;
    QChanListDelegate             *pDelegate;
    cparser::SGenreInfo           *pGenreInfo;

protected:
    void TouchEpgNavi (bool bCreate);
    void savePositions();
    void correctEpgOffset();

    virtual void changeEvent(QEvent* e);
    virtual void closeEvent (QCloseEvent *event);

private slots:
    void on_cbxGenre_activated(int index);
    void on_cbxLastOrBest_activated(int index);
    void on_btnNextSite_clicked();
    void on_btnPrevSite_clicked();
    void on_cbxSites_activated(int index);
    void on_lineSearch_returnPressed();
    void on_btnVodSearch_clicked();
    void on_btnFontLarger_clicked();
    void on_btnFontSmaller_clicked();
    void on_btnSearch_clicked();
    void on_cbxChannelGroup_activated(int index);
    void slotDayTabChanged(int iIdx);
    void slotbtnBack_clicked();
    void slotbtnNext_clicked();
    void on_channelList_doubleClicked(const QModelIndex & index);
    void slotChanListContext(const QPoint &pt);
    void slotChgFavourites (QAction *pAct);
    void slotHandleFavAction(QAction *pAct);
    void slotFavBtnContext(const QPoint &pt);
    void slotCurrentChannelChanged(const QModelIndex & current);
    void slotAddFav(int cid);

signals:
    void sigDoubliClickOnListWidget();
    void sigChannelDlgClosed();
};

#endif // CCHANNELSEPGDLG_H
