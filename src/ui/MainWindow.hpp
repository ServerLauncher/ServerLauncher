#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    void keyReleaseEvent(QKeyEvent* event) override;
#endif

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H