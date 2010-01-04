#ifndef PREFDIALOG_H
#define PREFDIALOG_H

#include <QDialog>

namespace Ui {
    class PrefDialog;
}

class PrefDialog : public QDialog {
    Q_OBJECT
public:
    PrefDialog(QWidget *parent = 0);
    ~PrefDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PrefDialog *ui;
};

#endif // PREFDIALOG_H
