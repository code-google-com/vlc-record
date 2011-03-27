#ifndef CINSTRUCTION_H
#define CINSTRUCTION_H

#include <QDialog>
#include <QString>
#include <QTextStream>

namespace Ui {
    class CInstruction;
}

class CInstruction : public QDialog
{
    Q_OBJECT

public:
    explicit CInstruction(QWidget *parent = 0);
    ~CInstruction();

private:
    Ui::CInstruction *ui;
    QString strInstr;

private slots:
    void on_pushOK_clicked();
};

#endif // CINSTRUCTION_H
