#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QtGui/QDialog>
#include <QModelIndexList>

namespace Ui {
    class FindDialog;
}

class SortFilterProxyModel;
class QTableView;

class FindDialog : public QDialog {
    Q_OBJECT
public:
    FindDialog(QWidget *parent, SortFilterProxyModel* model, QTableView* console);
    ~FindDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FindDialog *m_ui;
    SortFilterProxyModel *_model;
    QTableView *_console;
    QString _findStr;
    QModelIndexList _results;
    int _currentHit;

private slots:
    bool processFind();
    void findNext();
    void findPrevious();
    void findStringChanged(QString);
};

#endif // FINDDIALOG_H
