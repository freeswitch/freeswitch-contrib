#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "esl.h"

preferencesDialog::preferencesDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::preferencesDialog),
    colorChooser(NULL)
{
    m_ui->setupUi(this);

    readSettings();

    connect(m_ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    connect(m_ui->btnChangeBackgroundColor, SIGNAL(clicked()),
            this, SLOT(changeBackgroundColor()));

    connect(m_ui->btnChangeConsoleLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeConsoleLogLevelColor()));
    connect(m_ui->btnChangeCriticalLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeCriticalLogLevelColor()));
    connect(m_ui->btnChangeDebugLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeDebugLogLevelColor()));
    connect(m_ui->btnChangeErrorLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeErrorLogLevelColor()));
    connect(m_ui->btnChangeInfoLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeInfoLogLevelColor()));
    connect(m_ui->btnChangeNoticeLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeNoticeLogLevelColor()));
    connect(m_ui->btnChangeWarningLogLevelColor, SIGNAL(clicked()),
            this, SLOT(changeWarningLogLevelColor()));

    connect(this, SIGNAL(accepted()),
            this, SLOT(saveSettings()));
    connect(this, SIGNAL(rejected()),
            this, SLOT(readSettings()));
}

preferencesDialog::~preferencesDialog()
{
    delete m_ui;
}

void preferencesDialog::changeEvent(QEvent *e)
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
 void preferencesDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
 {
     if (!current)
         current = previous;

     m_ui->stackedWidget->setCurrentIndex(m_ui->listWidget->row(current));
 }

 void preferencesDialog::changeBackgroundColor()
 {
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmBackgroundColor->setPalette(colorChooser->getColor(
             m_ui->frmBackgroundColor->palette().color(QPalette::Background)));
 }
void preferencesDialog::changeConsoleLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmConsoleLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmConsoleLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeCriticalLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmCriticalLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmCriticalLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeDebugLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmDebugLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmDebugLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeErrorLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmErrorLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmErrorLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeInfoLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmInfoLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmInfoLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeNoticeLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmNoticeLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmNoticeLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::changeWarningLogLevelColor()
{
     if(!colorChooser)
         colorChooser = new QColorDialog();

     m_ui->frmWarningLogLevelColor->setPalette(colorChooser->getColor(
             m_ui->frmWarningLogLevelColor->palette().color(QPalette::Background)));
}
void preferencesDialog::saveSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

    /* Console Background */
    settings.setValue("consoleBackgroundColor", m_ui->frmBackgroundColor->palette().color(QPalette::Background));

    /* Log level colors */
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_EMERG), m_ui->frmConsoleLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_CRIT), m_ui->frmCriticalLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_DEBUG), m_ui->frmDebugLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_ERROR), m_ui->frmErrorLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_INFO), m_ui->frmInfoLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_NOTICE), m_ui->frmNoticeLogLevelColor->palette().color(QPalette::Background));
    settings.setValue(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_WARNING), m_ui->frmWarningLogLevelColor->palette().color(QPalette::Background));

    /* Make all tabs change their settings */
    emit backgroundColorChanged();
}
void preferencesDialog::readSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

    /* Console Background */
    m_ui->frmBackgroundColor->setPalette(settings.value("consoleBackgroundColor",QColor(Qt::white)).value<QColor>());

    /* Log level colors */
    m_ui->frmConsoleLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_EMERG)
                                                             ,QColor(Qt::black)).value<QColor>());
    m_ui->frmCriticalLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_CRIT)
                                                              ,QColor(Qt::red)).value<QColor>());
    m_ui->frmDebugLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_DEBUG)
                                                           ,QColor(Qt::darkYellow)).value<QColor>());
    m_ui->frmErrorLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_ERROR)
                                                           ,QColor(Qt::red)).value<QColor>());
    m_ui->frmInfoLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_INFO)
                                                          ,QColor(Qt::cyan)).value<QColor>());
    m_ui->frmNoticeLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_NOTICE)
                                                            ,QColor(Qt::darkGreen)).value<QColor>());
    m_ui->frmWarningLogLevelColor->setPalette(settings.value(QString("Log-Level-%1-Color").arg(ESL_LOG_LEVEL_WARNING)
                                                             ,QColor(Qt::yellow)).value<QColor>());
}
