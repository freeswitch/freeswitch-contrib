/********************************************************************************
** Form generated from reading ui file 'settings.ui'
**
** Created: Thu Aug 6 02:01:13 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QListWidget *listConfig;
    QStackedWidget *configPages;
    QWidget *page;
    QWidget *page_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnApply;
    QPushButton *btnOk;
    QPushButton *btnCancel;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
        SettingsDialog->resize(400, 300);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/fsgui_logo.png")), QIcon::Normal, QIcon::Off);
        SettingsDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(SettingsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        listConfig = new QListWidget(SettingsDialog);
        listConfig->setObjectName(QString::fromUtf8("listConfig"));
        listConfig->setMaximumSize(QSize(128, 16777215));
        listConfig->setIconSize(QSize(96, 84));
        listConfig->setMovement(QListView::Static);
        listConfig->setSpacing(12);
        listConfig->setViewMode(QListView::IconMode);

        horizontalLayout->addWidget(listConfig);

        configPages = new QStackedWidget(SettingsDialog);
        configPages->setObjectName(QString::fromUtf8("configPages"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        configPages->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        configPages->addWidget(page_2);

        horizontalLayout->addWidget(configPages);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        btnApply = new QPushButton(SettingsDialog);
        btnApply->setObjectName(QString::fromUtf8("btnApply"));

        horizontalLayout_2->addWidget(btnApply);

        btnOk = new QPushButton(SettingsDialog);
        btnOk->setObjectName(QString::fromUtf8("btnOk"));

        horizontalLayout_2->addWidget(btnOk);

        btnCancel = new QPushButton(SettingsDialog);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));

        horizontalLayout_2->addWidget(btnCancel);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(SettingsDialog);
        QObject::connect(btnOk, SIGNAL(clicked()), SettingsDialog, SLOT(accept()));
        QObject::connect(btnCancel, SIGNAL(clicked()), SettingsDialog, SLOT(reject()));

        configPages->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QApplication::translate("SettingsDialog", "FsGui", 0, QApplication::UnicodeUTF8));
        btnApply->setText(QApplication::translate("SettingsDialog", "&Apply", 0, QApplication::UnicodeUTF8));
        btnOk->setText(QApplication::translate("SettingsDialog", "&OK", 0, QApplication::UnicodeUTF8));
        btnCancel->setText(QApplication::translate("SettingsDialog", "&Cancel", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(SettingsDialog);
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGS_H
