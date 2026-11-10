#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include <QTextDocument>

class TextEditor;

namespace Ui { class ReplaceDialog; }

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent, TextEditor *editor);
    ~ReplaceDialog() override;

private slots:
    void findNext();
    void replaceCurrent();
    void replaceAll();

private:
    QTextDocument::FindFlags currentFlags() const;

    Ui::ReplaceDialog *ui;
    TextEditor *m_editor;
};

#endif // REPLACEDIALOG_H
