#include "cinstruction.h"
#include "ui_cinstruction.h"

CInstruction::CInstruction(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CInstruction)
{
    ui->setupUi(this);

    strInstr = "";
    QTextStream str(&strInstr);

    str << tr("Instruction.") << endl;

    ui->textBrowser->setHtml(strInstr);
}

CInstruction::~CInstruction()
{
    delete ui;
}

void CInstruction::on_pushOK_clicked()
{
    this->close();
}
