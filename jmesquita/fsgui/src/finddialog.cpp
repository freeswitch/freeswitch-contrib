#include <QtGui>
#include "finddialog.h"
#include "ui_finddialog.h"

findDialog::findDialog(QWidget *parent, QTextEdit *console) :
    QDialog(parent),
    m_ui(new Ui::findDialog),
    editConsole(console),
    pos(0)
{
    m_ui->setupUi(this);

    connect(m_ui->btnFind, SIGNAL(clicked()),
            this, SLOT(findClicked()));
    connect(this, SIGNAL(rejected()),
            this, SLOT(cancelClicked()));
}

findDialog::~findDialog()
{
    delete m_ui;
}

void findDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void findDialog::findClicked()
{
    QTextCursor findCursor;
    QFlags<QTextDocument::FindFlag> options = 0;
    if (m_ui->checkCase->isChecked())
        options = options|QTextDocument::FindCaseSensitively;
    if (m_ui->checkEntire->isChecked())
        options = options|QTextDocument::FindWholeWords;

    if (m_ui->checkRegex->isChecked())
    {
        findCursor = editConsole->document()->find(QRegExp(m_ui->lineFind->text()), pos, options);
    }
    else
    {
        findCursor = editConsole->document()->find(m_ui->lineFind->text(), pos, options);
    }
    if (findCursor.isNull())
    {
        QMessageBox::warning(this, tr("Find ..."),
                    tr("Nothing found"),
                    QMessageBox::Ok);
        pos = 0;
        m_ui->btnFind->setText(tr("Find again"));
        return;
    }
    pos = findCursor.position();
    editConsole->setTextCursor(findCursor);
    m_ui->btnFind->setText(tr("Find next"));
}
void findDialog::cancelClicked()
{
    pos = 0;
    m_ui->btnFind->setText(tr("Find"));
}
