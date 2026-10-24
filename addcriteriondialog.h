#ifndef ADDCRITERIONDIALOG_H
#define ADDCRITERIONDIALOG_H

#include <QDialog>

class DbManager;
class QComboBox;
class QCompleter;
class QLabel;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;

class AddCriterionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCriterionDialog(const DbManager &db, QWidget *parent = nullptr);

    QString criterion() const;

protected:
    void accept() override;

private slots:
    void on_type_changed();
    void on_add_value();
    void on_remove_value();
    void update_preview();

private:
    enum Shape { ShapeInt, ShapeStringInt, ShapeMulti };

    Shape shapeFor(const QString &key) const;
    QStringList completerSourceFor(const QString &key) const;
    void refreshCompleter();

    const DbManager &m_db;

    QComboBox *m_typeCombo = nullptr;
    QStackedWidget *m_valueStack = nullptr;
    QSpinBox *m_intSpin = nullptr;
    QComboBox *m_stringCombo = nullptr;
    QSpinBox *m_stringIntSpin = nullptr;
    QComboBox *m_multiInput = nullptr;
    QListWidget *m_multiList = nullptr;
    QPushButton *m_addValueBtn = nullptr;
    QPushButton *m_removeValueBtn = nullptr;
    QLabel *m_preview = nullptr;
    QPushButton *m_okBtn = nullptr;
    QCompleter *m_activeCompleter = nullptr;

    bool allowDuplicates() const;
};

#endif // ADDCRITERIONDIALOG_H
