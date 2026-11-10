#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "texteditor.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "aboutdialog.h"

#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QLabel>
#include <QMessageBox>
#include <QStyle>
#include <QTextCharFormat>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 状态栏右侧固定信息：字符数、行数、光标行列
    m_statusLabel = new QLabel(this);
    m_statusLabel->setFrameStyle(QFrame::NoFrame);
    statusBar()->addPermanentWidget(m_statusLabel);

    // 工具栏图标使用 Qt 标准图标，免资源文件
    ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionSaveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionUndo->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->actionRedo->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->actionCut->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    ui->actionCopy->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    ui->actionPaste->setIcon(style()->standardIcon(QStyle::SP_TitleBarShadeButton));
    ui->actionFind->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));

    // 文件操作
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    // 编辑操作
    connect(ui->actionUndo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->textEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->textEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->textEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->textEdit, &QPlainTextEdit::selectAll);
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // 格式操作
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::setEditorFont);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::setFontColor);
    connect(ui->actionTextBackgroundColor, &QAction::triggered, this, &MainWindow::setTextBackgroundColor);
    connect(ui->actionEditorBackgroundColor, &QAction::triggered, this, &MainWindow::setEditorBackgroundColor);
    connect(ui->actionWordWrap, &QAction::toggled, this, &MainWindow::toggleWordWrap);
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumbers);

    // 视图操作
    connect(ui->actionToolBar, &QAction::toggled, this, &MainWindow::toggleToolBar);
    connect(ui->actionStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    // 帮助
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 文档状态变化 -> 标题与动作使能
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateActionStates);

    // 剪切/复制的使能随选中状态变化
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable, ui->actionCopy, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable, ui->actionRedo, &QAction::setEnabled);

    // 状态栏信息与动作使能
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusInfo);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateActionStates);
    connect(ui->textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusInfo);
    connect(ui->textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateActionStates);

    // 编辑器初始背景色（淡黄），可通过"编辑器背景色"菜单修改
    QPalette pal = ui->textEdit->palette();
    pal.setColor(QPalette::Base, QColor(255, 255, 204));
    ui->textEdit->setPalette(pal);

    // 定时自动保存：每 10 秒检查一次，已修改且已有文件名时自动保存
    m_autoSaveTimerId = startTimer(10000);

    setCurrentFile(QString());
    updateStatusInfo();
    updateActionStates();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_autoSaveTimerId) {
        // 仅对已保存过且被修改的文件执行自动保存
        if (!m_filePath.isEmpty() && ui->textEdit->document()->isModified()) {
            QFile file(m_filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(ui->textEdit->toPlainText().toUtf8());
                file.close();
                ui->textEdit->document()->setModified(false);
                statusBar()->showMessage(QStringLiteral("已自动保存"), 2000);
                updateWindowTitle();
            }
        }
        return;
    }
    QMainWindow::timerEvent(event);
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("当前文件尚未保存，是否保存修改？"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true; // Discard
}

void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    ui->textEdit->clear();
    setCurrentFile(QString());
    ui->textEdit->document()->setModified(false);
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开文件"), QString(),
        QStringLiteral("文本文件 (*.txt);;所有文件 (*.*)"));
    if (path.isEmpty())
        return;
    loadFile(path);
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("打开文件"),
                             QStringLiteral("无法打开文件 %1：\n%2")
                                 .arg(QDir::toNativeSeparators(path), file.errorString()));
        return;
    }
    ui->textEdit->setPlainText(QString::fromUtf8(file.readAll()));
    setCurrentFile(path);
    ui->textEdit->document()->setModified(false);
    statusBar()->showMessage(QStringLiteral("已打开 %1").arg(QDir::toNativeSeparators(path)), 3000);
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())
        return saveFileAs();

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("保存文件"),
                             QStringLiteral("无法保存文件 %1：\n%2")
                                 .arg(QDir::toNativeSeparators(m_filePath), file.errorString()));
        return false;
    }
    file.write(ui->textEdit->toPlainText().toUtf8());
    file.close();
    ui->textEdit->document()->setModified(false);
    statusBar()->showMessage(QStringLiteral("已保存 %1").arg(QDir::toNativeSeparators(m_filePath)), 3000);
    return true;
}

bool MainWindow::saveFileAs()
{
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("另存为"), QString(),
        QStringLiteral("文本文件 (*.txt);;所有文件 (*.*)"));
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("另存为"),
                             QStringLiteral("无法保存文件 %1：\n%2")
                                 .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }
    file.write(ui->textEdit->toPlainText().toUtf8());
    file.close();
    setCurrentFile(path);
    ui->textEdit->document()->setModified(false);
    statusBar()->showMessage(QStringLiteral("已保存 %1").arg(QDir::toNativeSeparators(path)), 3000);
    return true;
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_filePath = path;
    updateWindowTitle();
    updateStatusInfo();
}

void MainWindow::updateWindowTitle()
{
    QString name = m_filePath.isEmpty()
                       ? QStringLiteral("新建文本文件")
                       : QFileInfo(m_filePath).fileName();
    QString star = ui->textEdit->document()->isModified() ? QStringLiteral("* ") : QString();
    setWindowTitle(QStringLiteral("%1%2 - 编辑器").arg(star, name));
}

void MainWindow::updateStatusInfo()
{
    const QTextCursor cursor = ui->textEdit->textCursor();
    const int length = ui->textEdit->toPlainText().length();
    const int lines = ui->textEdit->document()->blockCount();
    m_statusLabel->setText(QStringLiteral("length: %1   lines: %2   Ln: %3  Col: %4")
                               .arg(length)
                               .arg(lines)
                               .arg(cursor.blockNumber() + 1)
                               .arg(cursor.columnNumber() + 1));
}

void MainWindow::updateActionStates()
{
    const bool hasSelection = ui->textEdit->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionUndo->setEnabled(ui->textEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->textEdit->document()->isRedoAvailable());
    ui->actionPaste->setEnabled(ui->textEdit->canPaste());
    ui->actionSelectAll->setEnabled(!ui->textEdit->toPlainText().isEmpty());
}

void MainWindow::showFindDialog()
{
    if (!m_findDialog)
        m_findDialog = new FindDialog(this, ui->textEdit);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog)
        m_replaceDialog = new ReplaceDialog(this, ui->textEdit);
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

void MainWindow::setEditorFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this,
                                            QStringLiteral("选择字体"));
    if (!ok)
        return;

    QTextCharFormat fmt;
    fmt.setFontFamilies({ font.family() });
    fmt.setFontPointSize(font.pointSizeF() > 0 ? font.pointSizeF() : 11.0);

    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.hasSelection()) {
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        // 未选中时作用于全文
        QTextCursor all(ui->textEdit->document());
        all.beginEditBlock();
        all.select(QTextCursor::Document);
        all.mergeCharFormat(fmt);
        all.endEditBlock();
    }
}

void MainWindow::setFontColor()
{
    const QColor color = QColorDialog::getColor(ui->textEdit->currentCharFormat().foreground().color(), this,
                                                QStringLiteral("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setForeground(color);
    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.hasSelection()) {
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        QTextCursor all(ui->textEdit->document());
        all.beginEditBlock();
        all.select(QTextCursor::Document);
        all.mergeCharFormat(fmt);
        all.endEditBlock();
    }
}

void MainWindow::setTextBackgroundColor()
{
    const QColor color = QColorDialog::getColor(QColor(255, 255, 255), this,
                                                QStringLiteral("选择字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setBackground(color);
    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.hasSelection()) {
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        QTextCursor all(ui->textEdit->document());
        all.beginEditBlock();
        all.select(QTextCursor::Document);
        all.mergeCharFormat(fmt);
        all.endEditBlock();
    }
}

void MainWindow::setEditorBackgroundColor()
{    QPalette pal = ui->textEdit->palette();
    const QColor color = QColorDialog::getColor(pal.color(QPalette::Base), this,
                                                QStringLiteral("选择编辑器背景色"));
    if (!color.isValid())
        return;
    pal.setColor(QPalette::Base, color);
    ui->textEdit->setPalette(pal);
}

void MainWindow::toggleWordWrap(bool enabled)
{
    ui->textEdit->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
}

void MainWindow::toggleLineNumbers(bool enabled)
{
    ui->textEdit->setLineNumbersVisible(enabled);
}

void MainWindow::toggleToolBar(bool visible)
{
    ui->toolBar->setVisible(visible);
}

void MainWindow::toggleStatusBar(bool visible)
{
    statusBar()->setVisible(visible);
}

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}
