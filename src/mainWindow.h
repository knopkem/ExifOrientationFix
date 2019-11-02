#ifndef mainWindow_h__
#define mainWindow_h__

#include <QDialog>
#include "mover.h"

class QGroupBox;
class MainWindowPrivate;

/**
 * The main window of the desktop application
 */
class MainWindow : public QDialog
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow(void);

protected slots:
    void onSourceButtonClicked();
    void onTargetButtonClicked();

    void doCopy();

protected:
    QGroupBox* createIntroGroup();
    QGroupBox* createSourceGroup();
    QGroupBox* createTargetGroup();
    QGroupBox* createActionGroup();

    QString openFileDialog();
    bool validateSelection();
    void readSettings();
    void writeSettings();

private:
    MainWindowPrivate* d;
};

#endif // mainWindow_h__
