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
}

ConsolePlugin::~ConsolePlugin(){}

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

void ConsolePlugin::addConfigItems(SettingsDialog *settings)
{
    consoleConfigPage = new Ui::configPage;
    QWidget *base = new QWidget;
    consoleConfigPage->setupUi(base);
    settings->addConfigItem("Console", base);

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

void ConsolePlugin::connected()
{
    consoleWindow->action_Connect->setEnabled(false);
    consoleWindow->action_Disconnect->setEnabled(true);
}

void ConsolePlugin::disconnected()
{
    consoleWindow->action_Connect->setEnabled(true);
    consoleWindow->action_Disconnect->setEnabled(false);
}

void ConsolePlugin::connectionFailed(QString /*reason*/)
{
}

void ConsolePlugin::newConnection()
{
    if (serverManager->exec())
    {
        ESLconnection *esl = serverManager->getESLconnection();
        consoleWindow->tabConsole->addTab(new ConsoleTabWidget(this, esl), esl->getName());
        if (hashESL.contains(esl->getName()))
        {
            esl->disconnect();
            esl->wait();
            hashESL.remove(esl->getName());
            delete esl;
        }
        hashESL.insert(esl->getName(), esl);
        QObject::connect(esl, SIGNAL(connected()),
                         this, SLOT(connected()));
        QObject::connect(esl, SIGNAL(connectionFailed(QString)),
                         this, SLOT(connectionFailed(QString)));
        QObject::connect(esl, SIGNAL(disconnected()),
                         this, SLOT(disconnected()));
    }
}

void ConsolePlugin::readSettings()
{
    QSettings settings;
    settings.beginGroup("Console");
    consoleConfigPage->lineConsole->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_EMERG)).value<QPalette>());
    consoleConfigPage->lineAlert->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ALERT)).value<QPalette>());
    consoleConfigPage->lineCritical->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_CRIT)).value<QPalette>());
    consoleConfigPage->lineError->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ERROR)).value<QPalette>());
    consoleConfigPage->lineWarning->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_WARNING)).value<QPalette>());
    consoleConfigPage->lineNotice->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_NOTICE)).value<QPalette>());
    consoleConfigPage->lineInfo->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_INFO)).value<QPalette>());
    consoleConfigPage->lineDebug->setPalette(settings.value(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_DEBUG)).value<QPalette>());
    settings.endGroup();
}

void ConsolePlugin::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Console");
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_EMERG), consoleConfigPage->lineConsole->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ALERT), consoleConfigPage->lineAlert->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_CRIT), consoleConfigPage->lineCritical->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_ERROR), consoleConfigPage->lineError->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_WARNING), consoleConfigPage->lineWarning->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_NOTICE), consoleConfigPage->lineNotice->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_INFO), consoleConfigPage->lineInfo->palette());
    settings.setValue(QString("log-level-%1-palette").arg(ESL_LOG_LEVEL_DEBUG), consoleConfigPage->lineDebug->palette());
    settings.endGroup();
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

Q_EXPORT_PLUGIN2(consoleplugin, ConsolePlugin)
