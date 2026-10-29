#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QMessageBox>
#include <QDirIterator>
#include <QString>
#include <QModelIndexList>
#include <QListWidgetItem>
#include <QProcess>
#include <QTimer>


#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QAbstractItemView>
#include <QListWidget>
#include <QMessageBox>
#include <QDirIterator>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr,
                        const QString &repoFile = QStringLiteral("/etc/scratchpkg.repo"),
                        const QString &scratchProgram = QStringLiteral("/usr/bin/scratch"));
    ~MainWindow();

private slots:
    void on_listWidget_itemChanged(QListWidgetItem *item);

    void on_pushButton_clicked();

    //void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void setStdout();
    void command(QString text);

    void on_actionAbout_triggered();

    void on_pushButton_2_clicked();
    void loadPackages();
    void searchPackages(QString term);
    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_plainTextEdit_textChanged();

    void on_pushButton_5_clicked();

private:
    QString repositoryConfig;
    QString dependencyCommand;
    QStringList selectedPackages;
    QStringList previewTargets;
    QString previewHeading;
    quint64 previewRevision = 0;
    quint64 runningPreviewRevision = 0;
    QProcess *previewProcess;
    QTimer *previewTimer;
    void updateInstallPreview();
    void startInstallPreview();
    void finishInstallPreview(int exitCode, QProcess::ExitStatus status);
    Ui::MainWindow *ui;
    QProcess *m_process;
    void resizeEvent(QResizeEvent*);
};
#endif // MAINWINDOW_H
