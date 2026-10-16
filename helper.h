#ifndef HELPER_H
#define HELPER_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QSignalMapper>
#include <QRadioButton>
#include <QCheckBox>
#include <QListWidget>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QSize>

#include "dbmanager.h"
#include "sbc_solver.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Helper;
}
QT_END_NAMESPACE

class Helper : public QMainWindow
{
    Q_OBJECT

public:
    Helper(QWidget *parent = nullptr);
    ~Helper();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_rating_slider_valueChanged(int value);

    void on_lw_radiobutton_clicked();
    void on_st_radiobutton_clicked();
    void on_rw_radiobutton_clicked();
    void on_cam_radiobutton_clicked();
    void on_lm_radiobutton_clicked();
    void on_cm_radiobutton_clicked();
    void on_rm_radiobutton_clicked();
    void on_cdm_radiobutton_clicked();
    void on_lb_radiobutton_clicked();
    void on_cb_radiobutton_clicked();
    void on_rb_radiobutton_clicked();
    void on_gk_radiobutton_clicked();

    void on_gk_radiobutton_toggled(bool checked);

    void on_stat1_slider_valueChanged(int value);
    void on_stat2_slider_valueChanged(int value);
    void on_stat3_slider_valueChanged(int value);
    void on_stat4_slider_valueChanged(int value);
    void on_stat5_slider_valueChanged(int value);
    void on_stat6_slider_valueChanged(int value);

    void on_submit_pushbutton_pressed();
    void on_edit_pushbutton_pressed();

    void on_add_button_clicked();
    void on_solve_button_clicked();
    void on_cancel_button_clicked();

    void on_unfilled_listwidget_itemClicked(QListWidgetItem *item);
    void on_criteria_listwidget_itemClicked(QListWidgetItem *item);
    void on_solutions_listwidget_itemClicked(QListWidgetItem *item);

    void on_league_entry_editingFinished();

    void on_lw_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_st_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_rw_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_cam_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_lm_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_cm_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_rm_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_cdm_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_lb_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_cb_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_rb_checkbox_checkStateChanged(const Qt::CheckState &state);
    void on_gk_checkbox_checkStateChanged(const Qt::CheckState &state);

    void on_long_search_checkbox_toggled(bool checked);

private:
    Ui::Helper *ui;

    DbManager db;

    QList<QRadioButton*> radio_buttons;
    QButtonGroup position_radiobuttons;

    QList<QCheckBox*> checkboxes;
    QButtonGroup position_checkboxes;
    int checkbox_count;

    QList<QPair<int, QString>> unfilled_cards;
    int current_edit_id;

    static QList<QPair<int, QString>> all_cards;

    SBC_Solver* buildSolverFromCriteria() const;
    void applyCriterion(SBC_Solver* s, const QString& crit) const;
    void displaySolutions(const QVector<SBC_Solver::Solution>& sols);

    QVector<SBC_Solver::Solution> m_currentSolutions;
    QVector<Card> m_lastSolverCards;
    QVector<Slot> m_lastFormation;

    QSize m_lastSolutionDialogSize;

    SBC_Solver* m_runningSolver = nullptr;

    QFutureWatcher<void>* m_runningWatcher = nullptr;
};
#endif // HELPER_H
