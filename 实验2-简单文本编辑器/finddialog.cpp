#include "finddialog.h"
#include "ui_finddialog.h"

#include "texteditor.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QTextDocument>

FindDialog::FindDialog(QWidget *parent, TextEditor *editor)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    connect(ui->findNextButton, &QPushButton::clicked,
            this, &FindDialog::findNext);
    connect(ui->findLineEdit, &QLineEdit::returnPressed,
            this, &FindDialog::findNext);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QTextDocument::FindFlags FindDialog::currentFlags() const
{
    QTextDocument::FindFlags flags;
    if (ui->caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->upRadioButton->isChecked())
        flags |= QTextDocument::FindBackward;
    return flags;
}

void FindDialog::findNext()
{
    const QString text = ui->findLineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("查找"),
                                 QStringLiteral("请输入查找内容。"));
        ui->findLineEdit->setFocus();
        return;
    }

    const QTextDocument::FindFlags flags = currentFlags();
    QTextCursor found = m_editor->document()->find(text, m_editor->textCursor(), flags);

    // 到达边界后循环查找
    if (found.isNull()) {
        QTextCursor start(m_editor->document());
        if (flags & QTextDocument::FindBackward)
            start.movePosition(QTextCursor::End);
        else
            start.movePosition(QTextCursor::Start);
        found = m_editor->document()->find(text, start, flags);
    }

    if (found.isNull()) {
        QMessageBox::information(this, QStringLiteral("查找"),
                                 QStringLiteral("未找到“%1”。").arg(text));
        return;
    }

    m_editor->setTextCursor(found);
    m_editor->ensureCursorVisible();
}
