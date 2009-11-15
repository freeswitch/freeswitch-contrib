#include <QtGui>
#include <QtPlugin>
#include "consoleplugin.h"
#include "eslconnection.h"
#include "settingsdialog.h"
#include "servermanager.h"
#include "consoletabwidget.h"
#include "ui_console_settings.h"


ConsolePlugin::ConsolePlugin(QWidget *parent)
        : QMainWindow(parent),
        consoleWindow(new Ui::consoleWindow),
        serverManager(NULL)
{
    consoleWindow->setupUi(this);
    consoleWindow->tabConsole->clear();

    QObject::connect(consoleWindow->tabConsole, SIGNAL(tabCloseRequested(int)),
                     this, SLOT(tabClose(int)));
    QObject::connect(consoleWindow->tabConsole, SIGNAL(currentChanged(int)),
                     this, SLOT(tabChanged(int)));

    QObject::connect(consoleWindow->action_Connect, SIGNAL(triggered()),
                     this, SLOT(connect()));
    QObject::connect(consoleWindow->action_Disconnect, SIGNAL(triggered()),
                     this, SLOT(disconnect()));
    QObject::connect(consoleWindow->action_Reconnect, SIGNAL(triggered()),
                     this, SLOT(reconnect()));
    QObject::connect(consoleWindow->action_Clear, SIGNAL(triggered()),
                     this, SLOT(clearLogContents()));
    QObject::connect(consoleWindow->action_Save_log, SIGNAL(triggered()),
                     this, SLOT(saveLogToFile()));
    QObject::connect(consoleWindow->actionRealtime_Statistics, SIGNAL(triggered()),
                     this, SLOT(showRealtimeStats()));
    QObject::connect(consoleWindow->action_Pastebin_Log, SIGNAL(triggered()),
                     this, SLOT(pastebinLog()));
    QObject::connect(consoleWindow->action_Find, SIGNAL(triggered()),
                     this, SLOT(findText()));
}

ConsolePlugin::~ConsolePlugin(){}

void ConsolePlugin::closeEvent(QCloseEvent *event)
 {
    QSettings settings;
    settings.beginGroup("Console");
    settings.setValue("WindowGeometry", this->saveGeometry());
    settings.endGroup();
    QWidget::closeEvent(event);
 }

void ConsolePlugin::showEvent ( QShowEvent * event )
{
    QSettings settings;
    settings.beginGroup("Console");
    this->restoreGeometry(settings.value("WindowGeometry").toByteArray());
    settings.endGroup();
    QWidget::showEvent(event);
}

QObject * ConsolePlugin::getMonitorInterfaceObject()
{
    return this;
}

QString ConsolePlugin::appName()
{
    return "Console";
}

QString ConsolePlugin::appDescription()
{
    return tr("This application will do the same as fs_cli does.");
}

void ConsolePlugin::addConfigItems(SettingsDialog *settings, QMap<QListWidgetItem *, QWidget *>&configItems)
{
    settingsDialog = settings;

    consoleConfigPage = new Ui::configPage();
    QWidget *base = new QWidget(this);

    consoleConfigPage->setupUi(base);

    QObject::connect(consoleConfigPage->btnConsoleFont, SIGNAL(clicked()),
                     this, SLOT(changeConsoleFont()));
    QObject::connect(consoleConfigPage->btnAlertFont, SIGNAL(clicked()),
                     this, SLOT(changeAlertFont()));
    QObject::connect(consoleConfigPage->btnCritFont, SIGNAL(clicked()),
                     this, SLOT(changeCriticalFont()));
    QObject::connect(consoleConfigPage->btnErrorFont, SIGNAL(clicked()),
                     this, SLOT(changeErrorFont()));
    QObject::connect(consoleConfigPage->btnWarningFont, SIGNAL(clicked()),
                     this, SLOT(changeWarningFont()));
    QObject::connect(consoleConfigPage->btnNoticeFont, SIGNAL(clicked()),
                     this, SLOT(changeNoticeFont()));
    QObject::connect(consoleConfigPage->btnInfoFont, SIGNAL(clicked()),
                     this, SLOT(changeInfoFont()));
    QObject::connect(consoleConfigPage->btnDebugFont, SIGNAL(clicked()),
                     this, SLOT(changeDebugFont()));
    QObject::connect(consoleConfigPage->btnBackgroundConsole, SIGNAL(clicked()),
                     this, SLOT(changeConsoleBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundAlert, SIGNAL(clicked()),
                     this, SLOT(changeAlertBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundCritical, SIGNAL(clicked()),
                     this, SLOT(changeCriticalBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundError, SIGNAL(clicked()),
                     this, SLOT(changeErrorBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundWarning, SIGNAL(clicked()),
                     this, SLOT(changeWarningBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundNotice, SIGNAL(clicked()),
                     this, SLOT(changeNoticeBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundInfo, SIGNAL(clicked()),
                     this, SLOT(changeInfoBackgroundColor()));
    QObject::connect(consoleConfigPage->btnBackgroundDebug, SIGNAL(clicked()),
                     this, SLOT(changeDebugBackgroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundConsole, SIGNAL(clicked()),
                     this, SLOT(changeConsoleForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundAlert, SIGNAL(clicked()),
                     this, SLOT(changeAlertForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundCritical, SIGNAL(clicked()),
                     this, SLOT(changeCriticalForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundError, SIGNAL(clicked()),
                     this, SLOT(changeErrorForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundWarning, SIGNAL(clicked()),
                     this, SLOT(changeWarningForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundNotice, SIGNAL(clicked()),
                     this, SLOT(changeNoticeForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundInfo, SIGNAL(clicked()),
                     this, SLOT(changeInfoForegroundColor()));
    QObject::connect(consoleConfigPage->btnForegroundDebug, SIGNAL(clicked()),
                     this, SLOT(changeDebugForegroundColor()));
    readSettings();
    QObject::connect(settings, SIGNAL(accepted()),
                     this, SLOT(writeSettings()));
    configItems.insert(new QListWidgetItem("Console"), base);
}

void ConsolePlugin::newInstance()
{
    this->show();
    newConnection();
}

void ConsolePlugin::setServerManager(ServerManager *manager)
{
    serverManager = manager;
    QObject::connect(consoleWindow->action_Server_Manager, SIGNAL(triggered()),
                     this, SLOT(newConnection()));
}

void ConsolePlugin::newConnection()
{
    if (serverManager->exec())
    {
        ESLconnection *esl = serverManager->getESLconnection(PLUGIN_NAME);

        if (esl)
        {
            for (int i = 0; i < consoleWindow->tabConsole->count(); i++)
            {
                if (consoleWindow->tabConsole->tabText(i) == esl->getName())
                {
                    consoleWindow->tabConsole->setCurrentIndex(i);
                    return;
                }
            }
        }

        ConsoleTabWidget *newTab = new ConsoleTabWidget(this, esl);
        consoleWindow->tabConsole->addTab(newTab, esl->getName());
        consoleWindow->tabConsole->setCurrentWidget(newTab);
        QObject::connect(esl, SIGNAL(connected()),
                         this, SLOT(connectionStateChanged()));
        QObject::connect(esl, SIGNAL(disconnected()),
                         this, SLOT(connectionStateChanged()));
        QObject::connect(esl, SIGNAL(connectionFailed(QString)),
                         this, SLOT(connectionStateChanged()));
    }
}

void ConsolePlugin::clearLogContents()
{
    ConsoleTabWidget *tab = qobject_cast<ConsoleTabWidget*>(consoleWindow->tabConsole->currentWidget());
    if (tab)
        tab->clearConsoleContents();
}

void ConsolePlugin::saveLogToFile()
{
    ConsoleTabWidget *tab = qobject_cast<ConsoleTabWidget*>(consoleWindow->tabConsole->currentWidget());
    if (tab)
        tab->saveLogToFile();
}

void ConsolePlugin::pastebinLog()
{
    ConsoleTabWidget *tab = qobject_cast<ConsoleTabWidget*>(consoleWindow->tabConsole->currentWidget());
    if (tab)
        tab->pastebinLog();
}

void ConsolePlugin::findText()
{
    ConsoleTabWidget *tab = qobject_cast<ConsoleTabWidget*>(consoleWindow->tabConsole->currentWidget());
    if (tab)
        tab->findText();
}

void ConsolePlugin::showRealtimeStats()
{
    ConsoleTabWidget *tab = qobject_cast<ConsoleTabWidget*>(consoleWindow->tabConsole->currentWidget());
    if (tab)
        tab->showRealtimeStats();
}

void ConsolePlugin::connectionStateChanged()
{
    tabChanged(consoleWindow->tabConsole->currentIndex());
}

void ConsolePlugin::setPalette(QLineEdit *control, QString key)
{
    QSettings settings;
    if (settings.contains(key)) {
        control->setPalette(settings.value(key).value<QPalette>());
    }
}

void ConsolePlugin::readSettings()
{
    QSettings settings;
    settings.beginGroup("Console");

    consoleConfigPage->lineConsole->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_EMERG)).value<QPalette>());
    consoleConfigPage->lineConsole->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_EMERG)).value<QFont>());
    consoleConfigPage->lineAlert->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ALERT)).value<QPalette>());
    consoleConfigPage->lineAlert->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_ALERT)).value<QFont>());
    consoleConfigPage->lineCritical->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_CRIT)).value<QPalette>());
    consoleConfigPage->lineCritical->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_CRIT)).value<QFont>());
    consoleConfigPage->lineError->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ERROR)).value<QPalette>());
    consoleConfigPage->lineError->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_ERROR)).value<QFont>());
    consoleConfigPage->lineWarning->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_WARNING)).value<QPalette>());
    consoleConfigPage->lineWarning->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_WARNING)).value<QFont>());
    consoleConfigPage->lineNotice->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_NOTICE)).value<QPalette>());
    consoleConfigPage->lineNotice->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_NOTICE)).value<QFont>());
    consoleConfigPage->lineInfo->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_INFO)).value<QPalette>());
    consoleConfigPage->lineInfo->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_INFO)).value<QFont>());
    consoleConfigPage->lineDebug->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_DEBUG)).value<QPalette>());;
    consoleConfigPage->lineDebug->setFont(settings.value(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_DEBUG)).value<QFont>());

    settings.endGroup();
}

void ConsolePlugin::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Console");
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_EMERG), consoleConfigPage->lineConsole->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_EMERG), consoleConfigPage->lineConsole->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ALERT), consoleConfigPage->lineAlert->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_ALERT), consoleConfigPage->lineAlert->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_CRIT), consoleConfigPage->lineCritical->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_CRIT), consoleConfigPage->lineCritical->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ERROR), consoleConfigPage->lineError->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_ERROR), consoleConfigPage->lineError->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_WARNING), consoleConfigPage->lineWarning->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_WARNING), consoleConfigPage->lineWarning->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_NOTICE), consoleConfigPage->lineNotice->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_NOTICE), consoleConfigPage->lineNotice->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_INFO), consoleConfigPage->lineInfo->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_INFO), consoleConfigPage->lineInfo->font());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_DEBUG), consoleConfigPage->lineDebug->palette());
    settings.setValue(QString("log-level-%1-font").arg(ESL_LOG_LEVEL_DEBUG), consoleConfigPage->lineDebug->font());
    settings.endGroup();
}

void ConsolePlugin::changeConsoleFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineConsole->font(), this);
    if (ok)
    {
        consoleConfigPage->lineConsole->setFont(font);
    }
}

void ConsolePlugin::changeAlertFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineAlert->font(), this);
    if (ok)
    {
        consoleConfigPage->lineAlert->setFont(font);
    }
}

void ConsolePlugin::changeWarningFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineWarning->font(), this);
    if (ok)
    {
        consoleConfigPage->lineWarning->setFont(font);
    }
}

void ConsolePlugin::changeCriticalFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineCritical->font(), this);
    if (ok)
    {
        consoleConfigPage->lineCritical->setFont(font);
    }
}

void ConsolePlugin::changeErrorFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineError->font(), this);
    if (ok)
    {
        consoleConfigPage->lineError->setFont(font);
    }
}

void ConsolePlugin::changeNoticeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineNotice->font(), this);
    if (ok)
    {
        consoleConfigPage->lineNotice->setFont(font);
    }
}

void ConsolePlugin::changeInfoFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineInfo->font(), this);
    if (ok)
    {
        consoleConfigPage->lineInfo->setFont(font);
    }
}

void ConsolePlugin::changeDebugFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, consoleConfigPage->lineDebug->font(), this);
    if (ok)
    {
        consoleConfigPage->lineDebug->setFont(font);
    }
}

void ConsolePlugin::changeConsoleBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineConsole->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineConsole->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineConsole->setPalette(palette);
        consoleConfigPage->lineConsole->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeAlertBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineAlert->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineAlert->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineAlert->setPalette(palette);
        consoleConfigPage->lineAlert->setAutoFillBackground(true);
    }
}


void ConsolePlugin::changeCriticalBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineCritical->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineCritical->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineCritical->setPalette(palette);
        consoleConfigPage->lineCritical->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeErrorBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineError->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineError->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineError->setPalette(palette);
        consoleConfigPage->lineError->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeWarningBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineWarning->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineWarning->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineWarning->setPalette(palette);
        consoleConfigPage->lineWarning->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeNoticeBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineNotice->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineNotice->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineNotice->setPalette(palette);
        consoleConfigPage->lineNotice->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeInfoBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineInfo->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineInfo->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineInfo->setPalette(palette);
        consoleConfigPage->lineInfo->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeDebugBackgroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineDebug->palette().color(QPalette::Base), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineDebug->palette();
        palette.setColor(QPalette::Base, color);
        consoleConfigPage->lineDebug->setPalette(palette);
        consoleConfigPage->lineDebug->setAutoFillBackground(true);
    }
}

void ConsolePlugin::changeConsoleForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineConsole->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineConsole->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineConsole->setPalette(palette);
    }
}

void ConsolePlugin::changeAlertForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineAlert->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineAlert->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineAlert->setPalette(palette);
    }
}

void ConsolePlugin::changeCriticalForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineCritical->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineCritical->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineCritical->setPalette(palette);
    }
}

void ConsolePlugin::changeErrorForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineError->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineError->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineError->setPalette(palette);
    }
}

void ConsolePlugin::changeWarningForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineWarning->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineWarning->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineWarning->setPalette(palette);
    }
}

void ConsolePlugin::changeNoticeForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineNotice->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineNotice->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineNotice->setPalette(palette);
    }
}

void ConsolePlugin::changeInfoForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineInfo->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineInfo->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineInfo->setPalette(palette);
    }
}

void ConsolePlugin::changeDebugForegroundColor()
{
    QColor color = QColorDialog::getColor(consoleConfigPage->lineDebug->palette().color(QPalette::Text), this);
    if (color.isValid())
    {
        QPalette palette = consoleConfigPage->lineDebug->palette();
        palette.setColor(QPalette::Text, color);
        consoleConfigPage->lineDebug->setPalette(palette);
    }
}

void ConsolePlugin::tabClose(int index)
{
    QWidget *tab = consoleWindow->tabConsole->widget(index);
    serverManager->endESLconnection(PLUGIN_NAME, consoleWindow->tabConsole->tabText(index));
    consoleWindow->tabConsole->removeTab(index);
    delete tab;
    if (consoleWindow->tabConsole->count() == 0)
    {
        consoleWindow->action_Connect->setEnabled(false);
        consoleWindow->action_Disconnect->setEnabled(false);
    }
}

void ConsolePlugin::tabChanged(int index)
{
    if (index == -1)
        return;
    ESLconnection *esl = serverManager->getESLconnection(PLUGIN_NAME, consoleWindow->tabConsole->tabText(index));
    if (esl)
    {
        consoleWindow->action_Connect->setDisabled(esl->isConnected());
        consoleWindow->action_Disconnect->setEnabled(esl->isConnected());
        consoleWindow->action_Reconnect->setEnabled(esl->isConnected());
    }
}

void ConsolePlugin::connect()
{
    ESLconnection *esl = serverManager->getESLconnection(PLUGIN_NAME, consoleWindow->tabConsole->tabText(consoleWindow->tabConsole->currentIndex()));
    if (esl)
    {
        esl->connect();
    }
}

void ConsolePlugin::reconnect()
{
    disconnect(); 
    QTimer::singleShot(500, this, SLOT(connect()));
}

void ConsolePlugin::disconnect()
{
    ESLconnection *esl = serverManager->getESLconnection(PLUGIN_NAME, consoleWindow->tabConsole->tabText(consoleWindow->tabConsole->currentIndex()));
    if (esl)
    {
        esl->disconnect();
    }
}

Q_EXPORT_PLUGIN2(consoleplugin, ConsolePlugin)
