#ifndef CPARENTALCONTROLDLG_H
#define CPARENTALCONTROLDLG_H

#include <QDialog>
#include <QTimer>

#include "cvlcrecdb.h"
#include "clogfile.h"
#include "cdirstuff.h"
#include "ckartinaxmlparser.h"
#include "cwaittrigger.h"

//===================================================================
// namespace
//===================================================================
namespace Ui
{
    class CParentalControlDlg;
}

class CParentalControlDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CParentalControlDlg(QWidget *parent = 0);
    ~CParentalControlDlg();

    void setWaitTrigger (CWaitTrigger *pTrigger);
    void setXmlParser (CKartinaXMLParser *parser);
    void setAccountInfo(const cparser::SAccountInfo *pInfo);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::CParentalControlDlg     *m_ui;
    QString                      sTempPasswd;
    CWaitTrigger                *pCmdQueue;
    CKartinaXMLParser           *pParser;
    QVector<cparser::SChan>      channelVector;
    QVector<cparser::SVodRate>   vodRatesVector;
    const cparser::SAccountInfo *pAccountInfo;

private slots:
    void on_linePasswd_returnPressed();
    void on_btnChgPCode_clicked();
    void on_btnSaveExitManager_clicked();
    void on_btnEnterManager_clicked();

public slots:
   void slotBuildChanManager (const QString &str);
   void slotBuildVodManager (const QString &str);
   void slotLockParentalManager ();
   void slotNewPCodeSet ();
   void slotEnablePCodeForm ();
};

#endif // CPARENTALCONTROLDLG_H
