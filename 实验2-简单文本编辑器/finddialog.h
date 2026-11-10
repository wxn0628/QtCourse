#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

class TextEditor;

namespace Ui { class FindDialog; }

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent, TextEditor *editor);
    ~FindDialog() override;

private slots:
    void findNext();

private:
    QTextDocument::FindFlags currentFlags() const;

    Ui::FindDialog *ui;
    TextEditor *m_editor;
};

#endif // FINDDIALOG_H
