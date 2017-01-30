#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appsmodel.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    appRunning(false)
{
    ui->setupUi(this);

    ui->listView->setModel(new AppsModel(this));
    ui->listView->selectionModel()->select(ui->listView->model()->index(0, 0), QItemSelectionModel::SelectCurrent);
    connect(ui->listView, &QListView::activated, this, &MainWindow::on_actionPlay_Game_triggered);

    QGamepadManager *gm = QGamepadManager::instance();
    //connect(gm, &QGamepadManager::gamepadAxisEvent, this, &MainWindow::slotAxisEvent);
    connect(gm, &QGamepadManager::gamepadButtonPressEvent, this, &MainWindow::slotButtonPressEvent);
    //connect(gm, &QGamepadManager::connectedGamepadsChanged, [gm](){ qDebug() << "conn" << gm->connectedGamepads(); });

    tray.setIcon(windowIcon());
    tray.show();
    tray.showMessage("Gamepad Launcher", "Gamepad Launcher started");
    connect(&tray, &QSystemTrayIcon::activated, this, &MainWindow::show);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotAxisEvent(int device, QGamepadManager::GamepadAxis axis, double d)
{
    //if(device == 0 && axis == QGamepadManager::ButtonCenter)
    qDebug() << device << axis << d;
}

void MainWindow::slotButtonPressEvent(int device, QGamepadManager::GamepadButton button)
{
    //qDebug() << device << button;
    if(appRunning)
        return;
    if(device != 0)
        return;
    QModelIndex idx = ui->listView->currentIndex();
    switch(button)
    {
    case QGamepadManager::ButtonDown:
    case QGamepadManager::ButtonRight:
        idx = ui->listView->model()->index(idx.row()+1, idx.column());
        break;
    case QGamepadManager::ButtonUp:
    case QGamepadManager::ButtonLeft:
        idx = ui->listView->model()->index(idx.row()-1, idx.column());
        break;
    case QGamepadManager::ButtonA:
        on_actionPlay_Game_triggered();
        return;
    case QGamepadManager::ButtonGuide:
    case QGamepadManager::ButtonStart:
        if(isVisible())
            hide();
        else
            show();
    default:
        return;
    }
    if(!idx.isValid())
        idx = ui->listView->model()->index(0, 0);
    ui->listView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
}

void MainWindow::on_actionAdd_Game_triggered()
{
    AppItem i;
    i.cmd = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, "Select executable or shortcut", QString(),
                                     #ifdef Q_OS_WIN
                                         "Executable (*.exe);;Shortcut (*.lnk)"
                                     #else
                                         QString()
                                     #endif
                                         ));
    if(i.cmd.isEmpty())
        return;

    i.iconpath = i.cmd;
    i.icon = loadIcon(i.cmd);
    if(i.icon.isNull())
    {
        i.iconpath = QString();
        i.icon = QPixmap(":/no-icon.png");
    }

    i.name = QInputDialog::getText(this, "Enter caption", "Caption", QLineEdit::Normal, QFileInfo(i.cmd).fileName());

    AppsModel* m = static_cast<AppsModel*>(ui->listView->model());
    m->items().append(i);
    m->update();
}

void MainWindow::on_actionRemove_Game_triggered()
{
    QModelIndex idx = ui->listView->currentIndex();
    if(!idx.isValid())
        return;
    AppsModel* m = static_cast<AppsModel*>(ui->listView->model());
    m->items().removeAt(idx.row());
    m->update();
}

void MainWindow::on_actionPlay_Game_triggered()
{
    QModelIndex idx = ui->listView->currentIndex();
    if(!idx.isValid())
        return;
    AppsModel* m = static_cast<AppsModel*>(ui->listView->model());
    launchApp(m->items().at(idx.row()));
}

void MainWindow::on_actionHide_to_tray_triggered()
{
    hide();
}

void MainWindow::launchApp(const AppItem &i)
{
    QProcess* p = new QProcess(this);
    p->setWorkingDirectory(QFileInfo(i.cmd).dir().absolutePath());
    p->start(i.cmd, QStringList());
    p->waitForStarted();
    if(p->state() != QProcess::Running)
    {
        QString msg = i.cmd + " failed to start (error = "
                + QString::number(p->error()) + ", state = " +  QString::number(p->state()) + ")";
        qDebug() << msg;
        QMessageBox::warning(this, "Error", msg);
        delete p;
        return;
    }
    connect(p, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished, this, &MainWindow::appClosed);
    connect(p, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished, p, &QProcess::deleteLater);
    appRunning = true;
    hide();
}

void MainWindow::appClosed(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* p = qobject_cast<QProcess*>(sender());
    qDebug() << p->program() << "has finished with status" << exitCode << exitStatus;
    appRunning = false;
    show();
}

void MainWindow::show()
{
    setWindowState(Qt::WindowNoState);
    QWidget::showNormal();
    activateWindow();
    raise();
}

void MainWindow::hide()
{
    QWidget::hide();
}

void MainWindow::changeEvent(QEvent *e)
{
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        case QEvent::WindowStateChange:
        if (windowState() & Qt::WindowMinimized)
        {
            QTimer::singleShot(0, this, SLOT(hide()));
            return;
        }
        default:
            break;
    }

    QMainWindow::changeEvent(e);
}
