#include "consoletabwidget.h"
#include "ui_consoletabwidget.h"
#include "eslconnection.h"

ConsoleTabWidget::ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection) :
    QWidget(parent),
    m_ui(new Ui::ConsoleTabWidget),
    esl(eslconnection),
    scrollTimer(new QTimer),
    findNext(false),
    autoScroll(true)
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

    QObject::connect(m_ui->btnFind, SIGNAL(clicked()),
                     this, SLOT(find()));
    QObject::connect(m_ui->lineFind, SIGNAL(textChanged(QString)),
                     this, SLOT(findStringChanged(QString)));

    QObject::connect(esl, SIGNAL(connected()),
                     this, SLOT(connected()));
    QObject::connect(esl, SIGNAL(connectionFailed(QString)),
                     this, SLOT(connectionFailed(QString)));
    QObject::connect(esl, SIGNAL(disconnected()),
                     this, SLOT(disconnected()));
    QObject::connect(esl, SIGNAL(gotEvent(ESLevent*)),
                     this, SLOT(gotEvent(ESLevent*)));

    QObject::connect(scrollTimer, SIGNAL(timeout()),
                     this, SLOT(conditionalScroll()));

    esl->connect();
}

ConsoleTabWidget::~ConsoleTabWidget()
{
    delete m_ui;
}

void ConsoleTabWidget::clearConsoleContents()
{
    sourceModel->clear();
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

void ConsoleTabWidget::flipScrollTimer()
{
    if (!esl->isConnected())
    {
        scrollTimer->stop();
        return;
    }

    if (scrollTimer->isActive())
        scrollTimer->stop();
    else if (autoScroll)
        scrollTimer->start(500);
}

void ConsoleTabWidget::setAutomaticScroll(bool enabled)
{
    qDebug() << "setAutomaticScroll to:" << enabled;
    autoScroll = enabled;
}

bool ConsoleTabWidget::getAutomaticScroll()
{
    return autoScroll;
}

void ConsoleTabWidget::conditionalScroll()
{
    //if (m_ui->consoleListView->verticalScrollBar()->value() == m_ui->consoleListView->verticalScrollBar()->maximum())
    qDebug() << "Asked to scroll!";
    m_ui->consoleListView->scrollToBottom();
}

void ConsoleTabWidget::find()
{
    if (m_ui->lineFind->text().isEmpty())
        return;

    QModelIndexList index;
    if (m_ui->consoleListView->currentIndex().isValid())
    {
        index = model->match(m_ui->consoleListView->currentIndex(), Qt::DisplayRole, m_ui->lineFind->text(), -1, Qt::MatchContains|Qt::MatchWrap);
    }
    else
    {
        index =  model->match(model->index(0,0), Qt::DisplayRole, m_ui->lineFind->text(), -1, Qt::MatchContains|Qt::MatchWrap);
    }

    if (index.isEmpty())
    {
        QMessageBox::warning(this, tr("No match"), tr("No match found!"));
    }
    else
    {
        if (findNext == true && index.count() > 1)
        {
            m_ui->consoleListView->scrollTo(index.at(1));
            m_ui->consoleListView->setCurrentIndex(index.at(1));
        }
        else
        {
            m_ui->consoleListView->scrollTo(index.at(0));
            m_ui->consoleListView->setCurrentIndex(index.at(0));
        }
        findNext = true;
        m_ui->btnFind->setText(tr("Find Next"));
    }
}

void ConsoleTabWidget::findStringChanged(QString findString)
{
    m_ui->btnFind->setDisabled(findString.isEmpty());
    m_ui->btnFind->setText(tr("Find"));
    findNext = false;
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
        }
        QStringList lines = final_str.split(QRegExp("(\n+)"), QString::SkipEmptyParts);
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
    sourceModel->appendRow(item);
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
