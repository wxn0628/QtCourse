#include "replacedialog.h"
#include "ui_replacedialog.h"

#include "texteditor.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QTextDocument>

ReplaceDialog::ReplaceDialog(QWidget *parent, TextEditor *editor)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    connect(ui->findNextButton, &QPushButton::clicked,
            this, &ReplaceDialog::findNext);
    connect(ui->replaceButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceCurrent);
    connect(ui->replaceAllButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceAll);
    connect(ui->findLineEdit, &QLineEdit::returnPressed,
            this, &ReplaceDialog::findNext);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QTextDocument::FindFlags ReplaceDialog::currentFlags() const
{
    QTextDocument::FindFlags flags;
    if (ui->caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->upRadioButton->isChecked())
        flags |= QTextDocument::FindBackward;
    return flags;
}

void ReplaceDialog::findNext()
{
    const QString text = ui->findLineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("请输入查找内容。"));
        ui->findLineEdit->setFocus();
        return;
    }

    const QTextDocument::FindFlags flags = currentFlags();
    QTextCursor found = m_editor->document()->find(text, m_editor->textCursor(), flags);

    if (found.isNull()) {
        QTextCursor start(m_editor->document());
        if (flags & QTextDocument::FindBackward)
            start.movePosition(QTextCursor::End);
        else
            start.movePosition(QTextCursor::Start);
        found = m_editor->document()->find(text, start, flags);
    }

    if (found.isNull()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("未找到“%1”。").arg(text));
        return;
    }

    m_editor->setTextCursor(found);
    m_editor->ensureCursorVisible();
}

void ReplaceDialog::replaceCurrent()
{
    const QString text = ui->findLineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("请输入查找内容。"));
        ui->findLineEdit->setFocus();
        return;
    }

    QTextCursor cursor = m_editor->textCursor();
    // 当前选中内容与查找目标一致时直接替换
    if (cursor.hasSelection() && cursor.selectedText() == text) {
        cursor.insertText(ui->replaceLineEdit->text());
        m_editor->setTextCursor(cursor);
    }
    findNext();
}

void ReplaceDialog::replaceAll()
{
    const QString text = ui->findLineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("请输入查找内容。"));
        ui->findLineEdit->setFocus();
        return;
    }

    const QString replaceText = ui->replaceLineEdit->text();
    const QTextDocument::FindFlags flags = currentFlags();

    int count = 0;
    QTextCursor cursor(m_editor->document());
    cursor.beginEditBlock();
    QTextCursor found = m_editor->document()->find(text, cursor, flags);
    while (!found.isNull()) {
        found.insertText(replaceText);
        ++count;
        found = m_editor->document()->find(text, found, flags);
    }
    cursor.endEditBlock();

    if (count > 0)
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("共替换 %1 处。").arg(count));
    else
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("未找到要替换的内容。"));
}
