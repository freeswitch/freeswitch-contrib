#include "consoletabwidget.h"
#include "ui_consoletabwidget.h"
#include "eslconnection.h"
#include "monitorstatemachine.h"
#include "realtimestatisticsdialog.h"

ConsoleTabWidget::ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection) :
    QWidget(parent),
    m_ui(new Ui::ConsoleTabWidget),
    esl(eslconnection),
    _rtStatsDlg(NULL),
    findNext(false)
{
    m_ui->setupUi(this);
    sourceModel = new ConsoleModel(this);
    model = new SortFilterProxyModel(this);
    model->setSourceModel(sourceModel);
    model->setFilterKeyColumn(0);
    m_ui->consoleListView->setModel(model);

    QObject::connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                     this, SLOT(setConditionalScroll()));
    QObject::connect(sourceModel, SIGNAL(layoutChanged()),
                     this, SLOT(conditionalScroll()));

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
    QObject::connect(m_ui->comboLogLevel, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(changeLogLevel(int)));

    QObject::connect(m_ui->btnFilterClear, SIGNAL(clicked()),
                     this, SLOT(filterClear()));
    QObject::connect(m_ui->lineFilter, SIGNAL(textChanged(QString)),
                     this, SLOT(filterStringChanged()));
    QObject::connect(m_ui->filterCaseSensitivityCheckBox, SIGNAL(toggled(bool)),
                     this, SLOT(filterStringChanged()));
    QObject::connect(m_ui->filterSyntaxComboBox, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(filterStringChanged()));

    QObject::connect(esl, SIGNAL(connected()),
                     this, SLOT(connected()));
    QObject::connect(esl, SIGNAL(connectionFailed(QString)),
                     this, SLOT(connectionFailed(QString)));
    QObject::connect(esl, SIGNAL(disconnected()),
                     this, SLOT(disconnected()));
    QObject::connect(esl, SIGNAL(receivedLogMessage(ESLevent)),
                     this, SLOT(gotEvent(ESLevent)));


    esl->connect();

    msm = new MonitorStateMachine();
    msm->addESLconnection(esl);

}

ConsoleTabWidget::~ConsoleTabWidget()
{
    writeSettings();
    delete m_ui;
}

void ConsoleTabWidget::setConditionalScroll()
{
    autoScroll = (m_ui->consoleListView->verticalScrollBar()->maximum() == m_ui->consoleListView->verticalScrollBar()->value());
}

void ConsoleTabWidget::conditionalScroll()
{
    if (autoScroll)
        m_ui->consoleListView->scrollToBottom();
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

void ConsoleTabWidget::filterClear()
{
    m_ui->lineFilter->clear();
}

void ConsoleTabWidget::filterStringChanged()
{
    Qt::CaseSensitivity caseSensitivity =
            m_ui->filterCaseSensitivityCheckBox->isChecked() ? Qt::CaseSensitive
            : Qt::CaseInsensitive;
    switch (m_ui->filterSyntaxComboBox->currentIndex())
    {
        case 0:
        {
            QRegExp regExp(m_ui->lineFilter->text(), caseSensitivity, QRegExp::RegExp2);
            model->setUUIDFilterLog(QString());
            model->setFilterRegExp(regExp);
            break;
        }
        case 1:
        {
            QRegExp regExp(m_ui->lineFilter->text(), caseSensitivity, QRegExp::Wildcard);
            model->setUUIDFilterLog(QString());
            model->setFilterRegExp(regExp);
            break;
        }
        case 2:
        {
            QRegExp regExp(m_ui->lineFilter->text(), caseSensitivity, QRegExp::FixedString);
            model->setUUIDFilterLog(QString());
            model->setFilterRegExp(regExp);
            break;
        }
        default:
        {
            model->setFilterRegExp(QString());
            model->setUUIDFilterLog(m_ui->lineFilter->text());
            break;
        }
    }
    m_ui->btnFilterClear->setDisabled(m_ui->lineFilter->text().isEmpty());
}

void ConsoleTabWidget::connected()
{
    m_ui->btnSend->setEnabled(true);
    m_ui->lineCmd->setEnabled(true);
    m_ui->comboLogLevel->setEnabled(true);
    QStandardItem *item = new QStandardItem(tr("Connected!"));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
    readSettings();
}

void ConsoleTabWidget::disconnected()
{
    m_ui->btnSend->setEnabled(false);
    m_ui->lineCmd->setEnabled(false);
    m_ui->comboLogLevel->setEnabled(false);
    QStandardItem *item = new QStandardItem(tr("Disconnected!"));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
}

void ConsoleTabWidget::connectionFailed(QString reason)
{
    m_ui->btnSend->setEnabled(false);
    m_ui->lineCmd->setEnabled(false);
    m_ui->comboLogLevel->setEnabled(false);
    QStandardItem *item = new QStandardItem(tr("Connection Failed! Reason: %1").arg(reason));
    item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
    addNewConsoleItem(item);
}

void ConsoleTabWidget::gotEvent(ESLevent event)
{
    if (!event.getBody().isEmpty())
    {
        QString text(event.getBody());

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
            item->setData(event._headers.value("Log-Level").toInt(), ConsoleModel::LogLevelRole);
            item->setData(event._headers.value("User-Data"), ConsoleModel::UUIDRole);
            addNewConsoleItem(item);
        }
    }
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
    CommandTransaction *transaction = new CommandTransaction(m_ui->lineCmd->text());
    QObject::connect(transaction, SIGNAL(gotResponse(ESLevent)),
                     this, SLOT(gotEvent(ESLevent)));
    esl->addCommand(transaction);
    m_ui->lineCmd->clear();
}

void ConsoleTabWidget::showRealtimeStats()
{
    if (!_rtStatsDlg)
    {
        _rtStatsDlg = new RealtimeStatisticsDialog(this, msm);
    }

    _rtStatsDlg->show();
    _rtStatsDlg->raise();
    _rtStatsDlg->activateWindow();
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

void ConsoleTabWidget::changeLogLevel(int level)
{
    if (esl->setConsoleLogLevel(level))
    {
        currentLogLevel = level;
        QStandardItem *item = new QStandardItem(QString("Changed loglevel to %1").arg(level));
        item->setData(ESL_LOG_LEVEL_EMERG, Qt::UserRole);
        addNewConsoleItem(item);
        if (level > 4)
        {
            m_ui->lblWarningMsg->show();
            m_ui->lblWarningMsg->setText("High loglevels connected to high load servers can cause network consgestion!");
            m_ui->lblWarningMsg->setAutoFillBackground(true);
            m_ui->lblWarningMsg->setPalette(QPalette(Qt::yellow));
            QTimer::singleShot(5000, m_ui->lblWarningMsg, SLOT(hide()));
        }
    }
    else
    {
        currentLogLevel = level;
        QStandardItem *item = new QStandardItem(QString("Could not change loglevel to %1").arg(level));
        item->setData(ESL_LOG_LEVEL_ALERT, Qt::UserRole);
        m_ui->comboLogLevel->blockSignals(true);
        m_ui->comboLogLevel->setCurrentIndex(currentLogLevel);
        m_ui->comboLogLevel->blockSignals(false);
    }
}

void ConsoleTabWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("Console");
    settings.beginGroup(esl->getName());
    /* Set the saved loglevel */
    m_ui->comboLogLevel->setCurrentIndex(settings.value("Loglevel", 0).toInt());
    settings.endGroup();
    settings.endGroup();
}

void ConsoleTabWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Console");
    settings.beginGroup(esl->getName());
    /* Save the current loglevel */
    settings.setValue("Loglevel", currentLogLevel);
    settings.endGroup();
    settings.endGroup();
}

void ConsoleTabWidget::filterLogUUID(QString uuid)
{
    m_ui->filterSyntaxComboBox->setCurrentIndex(3);
    m_ui->lineFilter->setText(uuid);
}
