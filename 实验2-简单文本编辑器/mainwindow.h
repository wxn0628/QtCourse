#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;
class FindDialog;
class ReplaceDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void showFindDialog();
    void showReplaceDialog();

    void setEditorFont();
    void setFontColor();
    void setTextBackgroundColor();
    void setEditorBackgroundColor();
    void toggleWordWrap(bool enabled);
    void toggleLineNumbers(bool enabled);
    void toggleToolBar(bool visible);
    void toggleStatusBar(bool visible);
    void showAboutDialog();

    void updateWindowTitle();
    void updateStatusInfo();
    void updateActionStates();

private:
    bool maybeSave();
    void setCurrentFile(const QString &path);
    void loadFile(const QString &path);

    Ui::MainWindow *ui;
    QString m_filePath;
    QLabel *m_statusLabel;
    int m_autoSaveTimerId = 0;
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
};

#endif // MAINWINDOW_H
