#include "consoletabwidget.h"
#include "ui_consoletabwidget.h"
#include "eslconnection.h"

ConsoleTabWidget::ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection) :
    QWidget(parent),
    m_ui(new Ui::ConsoleTabWidget),
    esl(eslconnection)
{
    m_ui->setupUi(this);
    sourceModel = new QStandardItemModel(this);
    model = new SortFilterProxyModel(this);
    model->setSourceModel(sourceModel);
    model->setFilterKeyColumn(0);
    m_ui->consoleListView->setModel(model);

    QObject::connect(m_ui->btnSend, SIGNAL(clicked()),
                     this, SLOT(cmdSendClicked()));
    QObject::connect(m_ui->lineCmd, SIGNAL(textChanged(QString)),
                     this, SLOT(lineCmdChanged(QString)));
    QObject::connect(m_ui->checkEmerg, SIGNAL(clicked(bool)),
                     this, SLOT(checkEmerg(bool)));
    QObject::connect(m_ui->checkAlert, SIGNAL(clicked(bool)),
                     this, SLOT(checkAlert(bool)));
    QObject::connect(m_ui->checkCrit, SIGNAL(clicked(bool)),
                     this, SLOT(checkCrit(bool)));
    QObject::connect(m_ui->checkError, SIGNAL(clicked(bool)),
                     this, SLOT(checkError(bool)));
    QObject::connect(m_ui->checkNotice, SIGNAL(clicked(bool)),
                     this, SLOT(checkNotice(bool)));
    QObject::connect(m_ui->checkInfo, SIGNAL(clicked(bool)),
                     this, SLOT(checkInfo(bool)));
    QObject::connect(m_ui->checkDebug, SIGNAL(clicked(bool)),
                     this, SLOT(checkDebug(bool)));

    QObject::connect(esl, SIGNAL(connected()),
                     this, SLOT(connected()));
    QObject::connect(esl, SIGNAL(connectionFailed(QString)),
                     this, SLOT(connectionFailed(QString)));
    QObject::connect(esl, SIGNAL(disconnected()),
                     this, SLOT(disconnected()));
    QObject::connect(esl, SIGNAL(gotEvent(ESLevent*)),
                     this, SLOT(gotEvent(ESLevent*)));

    esl->connect();
}

ConsoleTabWidget::~ConsoleTabWidget()
{
    delete m_ui;
}

void ConsoleTabWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConsoleTabWidget::connected()
{
    m_ui->btnSend->setEnabled(true);
    m_ui->lineCmd->setEnabled(true);
    QStandardItem *item = new QStandardItem(tr("Connected!"));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
}

void ConsoleTabWidget::disconnected()
{
    m_ui->btnSend->setEnabled(false);
    m_ui->lineCmd->setEnabled(false);
    QStandardItem *item = new QStandardItem(tr("Disconnected!"));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
}

void ConsoleTabWidget::connectionFailed(QString reason)
{
    m_ui->btnSend->setEnabled(false);
    m_ui->lineCmd->setEnabled(false);
    QStandardItem *item = new QStandardItem(tr("Connection Failed! Reason: %1").arg(reason));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
}

void ConsoleTabWidget::gotEvent(ESLevent * event)
{
    if (event->getBody())
    {
        QString text(event->getBody());

        /* Remove \r\n */
        QStringList textList = text.split(QRegExp("(\r+)"), QString::SkipEmptyParts);
        QString final_str;
        for (int line = 0; line<textList.size(); line++)
        {
            final_str += textList[line];
            if (line == textList.size()-1)
            {
                final_str.chop(1);
            }
        }
        QStringList lines = final_str.split('\n', QString::SkipEmptyParts);
        for (int line = 0; line < lines.size(); ++line)
        {
            QStandardItem *item = new QStandardItem(lines[line]);
            item->setData(QString(event->getHeader("Log-Level")).toInt(), Qt::UserRole);
            addNewConsoleItem(item);
        }
    }
    delete event;
}

void ConsoleTabWidget::addNewConsoleItem(QStandardItem *item)
{
    QSettings settings;
    settings.beginGroup("Console");
    QPalette palette = settings.value(QString("log-level-%1-palette").arg(item->data(Qt::UserRole).toInt())).value<QPalette>();
    item->setBackground(palette.base());
    item->setForeground(palette.text());
    if (m_ui->consoleListView->verticalScrollBar()->value() == m_ui->consoleListView->verticalScrollBar()->maximum())
    {
        sourceModel->appendRow(item);
        m_ui->consoleListView->scrollToBottom();
    }
    else
    {
        sourceModel->appendRow(item);
    }
}

void ConsoleTabWidget::cmdSendClicked()
{
    esl->bgapi(m_ui->lineCmd->text());
    m_ui->lineCmd->clear();
}

void ConsoleTabWidget::lineCmdChanged(QString text)
{
    m_ui->btnSend->setDisabled(text.isEmpty());
}

void ConsoleTabWidget::checkEmerg(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_EMERG, state);
}

void ConsoleTabWidget::checkAlert(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_ALERT, state);
}

void ConsoleTabWidget::checkCrit(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_CRIT, state);
}

void ConsoleTabWidget::checkError(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_ERROR, state);
}

void ConsoleTabWidget::checkNotice(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_NOTICE, state);
}

void ConsoleTabWidget::checkInfo(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_INFO, state);
}

void ConsoleTabWidget::checkDebug(bool state)
{
    model->setLogLevelFilter(ESL_LOG_LEVEL_DEBUG, state);
}
