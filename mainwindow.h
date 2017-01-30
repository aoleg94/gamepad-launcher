#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGamepad>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

struct AppItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slotAxisEvent(int device, QGamepadManager::GamepadAxis axis, double d);
    void slotButtonPressEvent(int device, QGamepadManager::GamepadButton button);
    void on_actionAdd_Game_triggered();
    void on_actionRemove_Game_triggered();
    void on_actionPlay_Game_triggered();
    void on_actionHide_to_tray_triggered();
    void launchApp(const AppItem& i);
    void appClosed(int exitCode, QProcess::ExitStatus exitStatus);
    void show();
    void hide();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon tray;
    bool appRunning;

    // QWidget interface
protected:
    virtual void changeEvent(QEvent *) override;
};

#endif // MAINWINDOW_H
