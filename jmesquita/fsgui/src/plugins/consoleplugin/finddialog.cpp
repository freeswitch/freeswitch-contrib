#include <QtGui>
#include "finddialog.h"
#include "ui_finddialog.h"
#include "sortfilterproxymodel.h"

FindDialog::FindDialog(QWidget *parent, SortFilterProxyModel* model, QTableView *console) :
    QDialog(parent),
    m_ui(new Ui::FindDialog),
    _model(model),
    _console(console)
{
    m_ui->setupUi(this);
    QObject::connect(m_ui->lineFind, SIGNAL(textChanged(QString)),
                     this, SLOT(findStringChanged(QString)));
    QObject::connect(m_ui->btnFindNext, SIGNAL(clicked()),
                     this, SLOT(findNext()));
    QObject::connect(m_ui->btnFindPrev, SIGNAL(clicked()),
                     this, SLOT(findPrevious()));

    _currentHit = -1;
}

FindDialog::~FindDialog()
{
    delete m_ui;
}

void FindDialog::findNext()
{
    if (processFind())
    {
        _currentHit++;
        _console->selectRow(_results.at(_currentHit).row());
        if (_currentHit == _results.size()-1)
            _currentHit--;
    }
}

void FindDialog::findPrevious()
{
    if (processFind())
    {
        if (_currentHit == -1 || _currentHit == 0)
            _currentHit = _results.size()-1;

        _console->selectRow(_results.at(_currentHit).row());
        _currentHit--;

    }
}

void FindDialog::changeEvent(QEvent *e)
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

bool FindDialog::processFind()
{

    QModelIndexList selectionList = _console->selectionModel()->selection().indexes();
    QModelIndex start;
    if (selectionList.isEmpty())
        start = _model->index(0,0,QModelIndex());
    else
        start = selectionList.at(0);

    /* String has changed. Restart the search!*/
    if (QString::compare(m_ui->lineFind->text(), _findStr))
    {
        _findStr = m_ui->lineFind->text();
        _currentHit = -1;

        Qt::MatchFlags caseSensitivity =
                m_ui->checkCaseSensitive->isChecked() ? Qt::MatchCaseSensitive
                : Qt::MatchExactly;
        switch (m_ui->comboStringType->currentIndex())
        {
        case 0:
            {
                _results = _model->match(start, Qt::DisplayRole, _findStr, -1, Qt::MatchContains | caseSensitivity);
                break;
            }
        case 1:
            {
                _results = _model->match(start, Qt::DisplayRole, _findStr, -1, Qt::MatchRegExp | caseSensitivity);
                break;
            }
        case 2:
            {
                _results = _model->match(start, Qt::DisplayRole, _findStr, -1, Qt::MatchWildcard | caseSensitivity);
                break;
            }
        }


        if (_results.isEmpty())
        {
            QMessageBox::warning(this, "FsGui", "No matches found!");
            m_ui->lineFind->setText("");
            m_ui->lineFind->setFocus();
            return false;
        }
    }
    return true;
}

void FindDialog::findStringChanged(QString ntext)
{
    m_ui->btnFindNext->setDisabled(ntext.isEmpty());
    m_ui->btnFindPrev->setDisabled(ntext.isEmpty());
}
