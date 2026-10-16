#include "helper.h"
#include "./ui_helper.h"

#include <QDebug>
#include <QCompleter>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QTimer>

#include "utils.h"
#include "addcriteriondialog.h"

namespace {
struct TypeStyle { QColor fg; QColor bg; };

TypeStyle styleFortype(const QString& type) {
    if (type == "Bronze") return { QColor(0x7A, 0x4A, 0x1E), QColor(0xF6, 0xEA, 0xDD) };
    if (type == "Silver") return { QColor(0x55, 0x55, 0x55), QColor(0xEE, 0xEE, 0xEE) };
    if (type == "Gold")   return { QColor(0x8A, 0x6A, 0x00), QColor(0xFD, 0xF5, 0xDD) };
    if (type == "Icon")   return { QColor(0x9A, 0x74, 0x2E), QColor(0xFA, 0xF1, 0xDD) };
    else                  return { QColor(0x8A, 0x2B, 0xBE), QColor(0xF3, 0xEA, 0xFA) };
}
}

Helper::Helper(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Helper)
    , db("D:\\Users\\shoto\\Documents\\FUT_Helper\\bdd.db",
         QString("%1").arg(QInputDialog::getInt(this, "Version", "Enter the version of the game you wish to use.",
                                                27, 25, 27, 1)))
    , position_radiobuttons()
    , position_checkboxes()
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());

    ui->tabWidget->setCurrentIndex(0);

    const auto types = db.getTypes();

    ui->type_combobox->addItems(types);
    ui->type_combobox->setCurrentText("");

    radio_buttons = {ui->lw_radiobutton, ui->st_radiobutton, ui->rw_radiobutton,
                     ui->cam_radiobutton, ui->lm_radiobutton,
                     ui->cm_radiobutton, ui->rm_radiobutton, ui->cdm_radiobutton,
                     ui->lb_radiobutton, ui->cb_radiobutton, ui->rb_radiobutton,
                     ui->gk_radiobutton};

    checkboxes = {ui->lw_checkbox, ui->st_checkbox, ui->rw_checkbox,
                  ui->cam_checkbox, ui->lm_checkbox, ui->cm_checkbox, ui->rm_checkbox,
                  ui->cdm_checkbox, ui->lb_checkbox, ui->cb_checkbox,
                  ui->rb_checkbox, ui->gk_checkbox};

    for (const auto& radio_button : radio_buttons) {
        position_radiobuttons.addButton(radio_button);
    }

    for (const auto& checkbox : checkboxes) {
        position_checkboxes.addButton(checkbox);
        checkbox->setChecked(false);
    }
    position_checkboxes.setExclusive(false);

    checkbox_count = 0;

    ui->gk_radiobutton->toggle();

    ui->last_added_label->setText("(Last added: " + db.getLastAdded() + ")");

    ui->rating_slider->setValue(db.getLastRating());
    ui->rating_value_label->setNum(ui->rating_slider->value());

    ui->type_combobox->setCurrentText(db.getLastType());

    ui->name_entry->setFocus();

    QCompleter *countries_completer = new QCompleter(db.getCountries(), this);
    countries_completer->setCaseSensitivity(Qt::CaseInsensitive);
    countries_completer->setFilterMode(Qt::MatchContains);
    ui->country_entry->setCompleter(countries_completer);

    QCompleter *leagues_completer = new QCompleter(db.getLeagues(), this);
    leagues_completer->setCaseSensitivity(Qt::CaseInsensitive);
    leagues_completer->setFilterMode(Qt::MatchContains);
    ui->league_entry->setCompleter(leagues_completer);

    QCompleter *clubs_completer = new QCompleter(db.getClubs(), this);
    clubs_completer->setCaseSensitivity(Qt::CaseInsensitive);
    clubs_completer->setFilterMode(Qt::MatchContains);
    ui->club_entry->setCompleter(clubs_completer);

    connect(countries_completer, QOverload<const QString&>::of(&QCompleter::activated),
            [=](){ui->league_entry->setFocus();});
    connect(leagues_completer, QOverload<const QString&>::of(&QCompleter::activated),
            [=](){ui->club_entry->setFocus();});

    unfilled_cards = db.getUnfilled();
    for (const auto& pair : unfilled_cards) {
        ui->unfilled_listwidget->addItem(pair.second);
    }
}

Helper::~Helper() {
    delete ui;
}

void Helper::closeEvent(QCloseEvent* event) {
    if (m_runningWatcher) {
        if (m_runningSolver) m_runningSolver->cancel();
        m_runningWatcher->waitForFinished();
        delete m_runningSolver; m_runningSolver = nullptr;
        delete m_runningWatcher; m_runningWatcher = nullptr;
    }
    QMainWindow::closeEvent(event);
}

void Helper::on_rating_slider_valueChanged(int value) {
    ui->rating_value_label->setNum(value);
}

void Helper::on_lw_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_st_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_rw_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_cam_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_lm_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_cm_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_rm_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_cdm_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_lb_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_cb_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_rb_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_gk_radiobutton_clicked() {
    ui->country_entry->setFocus();
}

void Helper::on_gk_radiobutton_toggled(bool checked) {
    ui->country_entry->setFocus();

    if (checked) {
        ui->stat1_label->setText(QString("DIV"));
        ui->stat2_label->setText(QString("REF"));
        ui->stat3_label->setText(QString("HAN"));
        ui->stat4_label->setText(QString("SPD"));
        ui->stat5_label->setText(QString("KIC"));
        ui->stat6_label->setText(QString("POS"));
    }
    else {
        ui->stat1_label->setText(QString("PAC"));
        ui->stat2_label->setText(QString("DRI"));
        ui->stat3_label->setText(QString("SHO"));
        ui->stat4_label->setText(QString("DEF"));
        ui->stat5_label->setText(QString("PAS"));
        ui->stat6_label->setText(QString("PHY"));
    }
}

void Helper::on_league_entry_editingFinished() {
    qDebug() << "got here (league editing finished)" << ui->league_entry->text();
    QCompleter *clubs_completer = new QCompleter(db.getClubsFromLeague(ui->league_entry->text()), this);
    clubs_completer->setCaseSensitivity(Qt::CaseInsensitive);
    clubs_completer->setFilterMode(Qt::MatchContains);
    ui->club_entry->setCompleter(clubs_completer);
}

void Helper::on_stat1_slider_valueChanged(int value) {
    ui->stat1_value_label->setNum(value);
}

void Helper::on_stat2_slider_valueChanged(int value) {
    ui->stat2_value_label->setNum(value);
}

void Helper::on_stat3_slider_valueChanged(int value) {
    ui->stat3_value_label->setNum(value);
}

void Helper::on_stat4_slider_valueChanged(int value) {
    ui->stat4_value_label->setNum(value);
}

void Helper::on_stat5_slider_valueChanged(int value) {
    ui->stat5_value_label->setNum(value);
}

void Helper::on_stat6_slider_valueChanged(int value) {
    ui->stat6_value_label->setNum(value);
}

void Helper::on_submit_pushbutton_pressed() {
    const int id = db.getLastId() + 1;
    const QString name = capitalize(ui->name_entry->text());
    const int rating = ui->rating_slider->value();
    const QString position = position_radiobuttons.checkedButton()->text();
    const QStringList sec_positions = getSecPositions(checkboxes);
    const int nsecpos = sec_positions.size();
    const QString secpos1 = nsecpos >= 1 ? sec_positions[0] : "";
    const QString secpos2 = nsecpos >= 2 ? sec_positions[1] : "";
    const QString secpos3 = nsecpos >= 3 ? sec_positions[2] : "";
    const QString secpos4 = nsecpos >= 4 ? sec_positions[3] : "";
    const QString secpos5 = nsecpos >= 5 ? sec_positions[4] : "";
    const QString country = ui->country_entry->text();
    const QString league = ui->league_entry->text();
    const QString club = ui->club_entry->text();
    const int stat1 = ui->stat1_slider->value();
    const int stat2 = ui->stat2_slider->value();
    const int stat3 = ui->stat3_slider->value();
    const int stat4 = ui->stat4_slider->value();
    const int stat5 = ui->stat5_slider->value();
    const int stat6 = ui->stat6_slider->value();
    const QString type = ui->type_combobox->currentText();

    if (name != "") {
        const QStringList current_countries = db.getCountries();
        const QStringList current_leagues = db.getLeagues();
        const QStringList current_clubs = db.getClubs();

        bool no_problem = true;
        if (country != "" && !current_countries.contains(country)) {
            no_problem = checkEntry("country", &country);
        }
        if (league != "" && !current_leagues.contains(league)) {
            no_problem = checkEntry("league", &league);
        }
        if (club != "" && !current_clubs.contains(club)) {
            no_problem = checkEntry("club", &club);
        }

        if (no_problem) {
            db.addCard(id, name, rating, position, secpos1, secpos2, secpos3, secpos4, secpos5, country, league,
                       club, stat1, stat2, stat3, stat4, stat5, stat6, type);

            QString last_added(QString("%1, %2, %3").arg(name).arg(rating).arg(type));
            ui->last_added_label->setText("(Last added: " + last_added + ")");

            ui->name_entry->clear();
            ui->country_entry->clear();
            //ui->league_entry->clear();
            //ui->club_entry->clear();
            ui->stat1_slider->setValue(0);
            ui->stat2_slider->setValue(0);
            ui->stat3_slider->setValue(0);
            ui->stat4_slider->setValue(0);
            ui->stat5_slider->setValue(0);
            ui->stat6_slider->setValue(0);
            uncheckAll(checkboxes);
            checkbox_count = 0;

            ui->name_entry->setFocus();

            const auto players = db.getPlayersNames();

            if (players.contains(name)) {
                QMessageBox msg_box;
                msg_box.setText("The name you entered was already saved in the database. Are they the same "
                                "player as one of these?");

                const auto players_info = db.getPlayerInfoByName(name);
                QString info_display;
                int compt = 0;
                for (const QString& player_info : players_info) {
                    info_display += QString("%1. %2\n").arg(compt).arg(player_info);
                    compt++;
                }
                qDebug() << "got there" << info_display;

                msg_box.setInformativeText(info_display);
                msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msg_box.setDefaultButton(QMessageBox::Yes);

                if (msg_box.exec() == QMessageBox::Yes) {
                    int same_player_id;
                    int row = 0;
                    if (players_info.size() > 1) {
                        row = QInputDialog::getInt(&msg_box, "ID", "Enter the row of the corresponding player.",
                                                   0, 0, players_info.size() - 1, 1);
                    }
                    same_player_id = info_display.split('\n')[row].split(". ")[1].split(',')[0].toInt();
                    db.addSamePlayer(same_player_id, id);
                }
                else {
                    db.addNewPlayer(id);
                }
            }
            else {
                db.addNewPlayer(id);
            }
        }
    }
}

void Helper::on_edit_pushbutton_pressed() {
    const QStringList sec_positions = getSecPositions(checkboxes);
    const int nsecpos = sec_positions.size();
    const QString secpos1 = nsecpos >= 1 ? sec_positions[0] : "";
    const QString secpos2 = nsecpos >= 2 ? sec_positions[1] : "";
    const QString secpos3 = nsecpos >= 3 ? sec_positions[2] : "";
    const QString secpos4 = nsecpos >= 4 ? sec_positions[3] : "";
    const QString secpos5 = nsecpos >= 5 ? sec_positions[4] : "";
    const QString country = ui->country_entry->text();
    const QString league = ui->league_entry->text();
    const QString club = ui->club_entry->text();
    const int stat1 = ui->stat1_slider->value();
    const int stat2 = ui->stat2_slider->value();
    const int stat3 = ui->stat3_slider->value();
    const int stat4 = ui->stat4_slider->value();
    const int stat5 = ui->stat5_slider->value();
    const int stat6 = ui->stat6_slider->value();

    const QStringList current_countries = db.getCountries();
    const QStringList current_leagues = db.getLeagues();
    const QStringList current_clubs = db.getClubs();

    bool no_problem = true;
    if (country != "" && !current_countries.contains(country)) {
        no_problem = checkEntry("country", &country);
    }
    if (league != "" && !current_leagues.contains(league)) {
        no_problem = checkEntry("league", &league);
    }
    if (club != "" && !current_clubs.contains(club)) {
        no_problem = checkEntry("club", &club);
    }

    if (no_problem) {
        qDebug() << "id = " << current_edit_id;

        db.updateCard(current_edit_id, secpos1, secpos2, secpos3, secpos4, secpos5, country, league, club, stat1,
                      stat2, stat3, stat4, stat5, stat6);

        ui->name_entry->clear();
        ui->country_entry->clear();
        ui->league_entry->clear();
        ui->club_entry->clear();
        ui->stat1_slider->setValue(0);
        ui->stat2_slider->setValue(0);
        ui->stat3_slider->setValue(0);
        ui->stat4_slider->setValue(0);
        ui->stat5_slider->setValue(0);
        ui->stat6_slider->setValue(0);
        uncheckAll(checkboxes);
        checkbox_count = 0;

        ui->name_entry->setFocus();
    }
}

void Helper::on_unfilled_listwidget_itemClicked(QListWidgetItem *item) {
    current_edit_id = getIdFromStrRep(unfilled_cards, item->text());
    qDebug() << "id = " << current_edit_id;
    ui->tabWidget->setCurrentIndex(0);

    const QStringList selected_values = item->text().split(", ");
    ui->name_entry->setText(selected_values[0]);
    ui->rating_slider->setValue(selected_values[1].toInt());
    for (auto& button : radio_buttons) {
        if (button->text() == selected_values[2]) {
            button->toggle();
        }
    }
    ui->type_combobox->setCurrentText(selected_values[3]);

    delete item;

    ui->country_entry->setFocus();
}

void Helper::on_lw_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_st_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_rw_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_cam_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_lm_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_cm_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_rm_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_cdm_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_lb_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_cb_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_rb_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_gk_checkbox_checkStateChanged(const Qt::CheckState &state) {
    if (state) {
        checkbox_count += 1;
        if (checkbox_count == 5) lockUnchecked(checkboxes);
    }
    else {
        if (checkbox_count == 5) unlockAll(checkboxes);
        checkbox_count -= 1;
    }
}

void Helper::on_long_search_checkbox_toggled(bool checked) {
    ui->time_budget_spinbox->setEnabled(checked);
}

void Helper::applyCriterion(SBC_Solver* s, const QString& crit) const {
    const int lp = crit.indexOf('(');
    const int rp = crit.lastIndexOf(')');
    if (lp < 0 || rp < lp) return;

    const QString key = crit.left(lp).trimmed();
    const QString val = crit.mid(lp + 1, rp - lp - 1);
    const QStringList parts = val.split(',', Qt::SkipEmptyParts);

    auto toInt = [](const QString& x){ return x.trimmed().toInt(); };

    if (key == "Club")        for (const auto& v : parts) s->club(v.trimmed());
    else if (key == "Clubs")  for (const auto& v : parts) s->club(v.trimmed());
    else if (key == "Club=")  s->clubExactly(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Club>=") s->clubAtLeast(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Club<=") s->clubAtMost(parts[0].trimmed(), toInt(parts[1]));

    else if (key == "League")   for (const auto& v : parts) s->league(v.trimmed());
    else if (key == "Leagues")  for (const auto& v : parts) s->league(v.trimmed());
    else if (key == "League=")  s->leagueExactly(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "League>=") s->leagueAtLeast(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "League<=") s->leagueAtMost(parts[0].trimmed(), toInt(parts[1]));

    else if (key == "Nation")   for (const auto& v : parts) s->nation(v.trimmed());
    else if (key == "Nations")  for (const auto& v : parts) s->nation(v.trimmed());
    else if (key == "Nation=")  s->nationExactly(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Nation>=") s->nationAtLeast(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Nation<=") s->nationAtMost(parts[0].trimmed(), toInt(parts[1]));

    else if (key == "ExcludeTypes") s->excludeTypes(parts);
    else if (key == "Types")        s->includeTypes(parts);
    else if (key == "Type=")        s->typeExactly(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Type>=")       s->typeAtLeast(parts[0].trimmed(), toInt(parts[1]));
    else if (key == "Type<=")       s->typeAtMost(parts[0].trimmed(), toInt(parts[1]));

    else if (key == "Specials=")  s->specialExactly(toInt(parts[0]));
    else if (key == "Specials>=") s->specialAtLeast(toInt(parts[0]));
    else if (key == "Specials<=") s->specialAtMost(toInt(parts[0]));

    else if (key == "MinNations")   s->minNations(toInt(parts[0]));
    else if (key == "MaxNations")   s->maxNations(toInt(parts[0]));
    else if (key == "MinTypes")     s->minTypes(toInt(parts[0]));
    else if (key == "MaxTypes")     s->maxTypes(toInt(parts[0]));
    else if (key == "MinClubs")     s->minClubs(toInt(parts[0]));
    else if (key == "MaxClubs")     s->maxClubs(toInt(parts[0]));
    else if (key == "MinLeagues")   s->minLeagues(toInt(parts[0]));
    else if (key == "MaxLeagues")   s->maxLeagues(toInt(parts[0]));
    else if (key == "MinAvgRating") s->minAvgRating(toInt(parts[0]));
    else if (key == "MaxAvgRating") s->maxAvgRating(toInt(parts[0]));
    else if (key == "MinChemistry") s->minChemistry(toInt(parts[0]));
    else if (key == "MaxChemistry") s->maxChemistry(toInt(parts[0]));

    else if (key == "FlexibleTypes") s->flexibleTypes(parts);

    else if (key == "Formation") {
        QVector<Slot> slots_;
        int no = 1;
        const QStringList positions = expandFormation(val);
        for (const QString& p : positions) {
            if (p.isEmpty()) continue;
            slots_.append({ no++, p.trimmed() });
        }
        s->formation(slots_);
    }

    else if (key == "ExcludeCards") {
        QSet<int> ids;
        for (const auto& v : parts) ids.insert(v.trimmed().toInt());
        s->excludeCardIds(ids);
    }
}

void Helper::on_solve_button_clicked() {
    if (ui->criteria_listwidget->count() == 0) {
        QMessageBox::warning(this, "SBC Solver", "Add at least one criterion first.");
        return;
    }

    SBC_Solver* solver = buildSolverFromCriteria();
    if (!solver) return;

    solver->prepare();

    ui->solve_button->setEnabled(false);
    ui->cancel_button->setEnabled(true);
    ui->solutions_listwidget->clear();
    ui->solutionscount_label->setText("solving...");

    m_runningSolver = solver;

    auto* watcher = new QFutureWatcher<void>(this);
    m_runningWatcher = watcher;
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, solver, watcher](){
        m_runningSolver = nullptr;
        m_runningWatcher = nullptr;

        ui->cancel_button->setEnabled(false);
        ui->solve_button->setEnabled(true);

        m_currentSolutions = solver->getResults();
        m_lastSolverCards = solver->allCards();
        m_lastFormation = solver->getSlots();

        const bool nodeHit = solver->hitNodeBudget();
        const bool timeHit = solver->hitTimeBudget();
        const int solCount = solver->getResults().size();
        const QString report = solver->leafRejectionReport();
        const int timeLimit = ui->time_budget_spinbox->value();

        delete solver;
        watcher->deleteLater();

        displaySolutions(m_currentSolutions);

        if ((nodeHit || timeHit) && solCount == 0) {
            QMessageBox dlg(this);
            dlg.setWindowTitle("Search incomplete");
            dlg.setText(timeHit
                            ? QString("The search reached its %1-second time limit without finding a solution.")
                                  .arg(timeLimit)
                            : "The search reached its node budget without finding a solution");
            dlg.setInformativeText("The criteria may still be satisfiable.");
            dlg.setDetailedText(report);
            dlg.exec();
        }
        else if (nodeHit || timeHit) {
            QMessageBox::information(this, "Partial results",
                                     QString("The search reached its %1 limit with %2 solution(s) found. "
                                             "More may exist")
                                         .arg(timeHit ? QString("%1-second time").arg(timeLimit) : "node")
                                         .arg(solCount));
        }
    });

    watcher->setFuture(QtConcurrent::run([solver](){ solver->search(); }));
}

void Helper::on_cancel_button_clicked() {
    if (!m_runningSolver) return;

    m_runningSolver->cancel();
    ui->cancel_button->setEnabled(false);
    ui->solutionscount_label->setText("cancelling...");
}

SBC_Solver* Helper::buildSolverFromCriteria() const {
    auto* s = new SBC_Solver(db, 50, {"SBC Premium", "Evo", "Elite Evo", "LTM"});

    bool hasFormation = false;
    for (int i = 0; i < ui->criteria_listwidget->count(); ++i) {
        const QString c = ui->criteria_listwidget->item(i)->text();
        if (c.startsWith("Formation(")) {
            hasFormation = true;
            break;
        }
    }
    if (!hasFormation) {
        s->formation({{1,"GK"},{2,"LB"},{3,"CB"},{4,"CB"},{5,"RB"},
                      {6,"CDM"},{7,"CDM"},{8,"CM"},{9,"LW"},{10,"ST"},{11,"RW"}});
    }

    for (int i = 0; i < ui->criteria_listwidget->count(); ++i) {
        applyCriterion(s, ui->criteria_listwidget->item(i)->text());
    }

    if (ui->long_search_checkbox->isChecked()) {
        const double seconds = double(ui->time_budget_spinbox->value());
        s->maxNodes(0).maxSeconds(seconds);
    }
    else
        s->maxNodes(5'000'000).maxSeconds(0.0);

    return s;
}

void Helper::displaySolutions(const QVector<SBC_Solver::Solution>& sols) {
    ui->solutionscount_label->setText(QString::number(sols.size()));
    ui->solutions_listwidget->clear();

    for (int r = 0; r < sols.size(); ++r) {
        auto* item = new QListWidgetItem(QString("Solution %1 (cost %2)").arg(r + 1).arg(sols[r].totalCost));
        item->setData(Qt::UserRole, r);
        ui->solutions_listwidget->addItem(item);
    }
}

void Helper::on_solutions_listwidget_itemClicked(QListWidgetItem* item) {
    const int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_currentSolutions.size()) return;

    const auto& sol = m_currentSolutions[idx];

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Solution %1").arg(idx + 1));
    if (m_lastSolutionDialogSize.isValid())
        dlg.resize(m_lastSolutionDialogSize);
    else
        dlg.setMinimumSize(600, 450);

    // header: total rating + chemistry
    auto* header = new QLabel(
        QString("<b>Solution %1</b> &nbsp;—&nbsp; Rating: <b>%2</b> &nbsp;—&nbsp; Chemistry: <b>%3</b>"
                " &nbsp;—&nbsp; Cost: <b>%4</b>")
            .arg(idx + 1)
            .arg(sol.gameRating)
            .arg(sol.chemistry)
            .arg(sol.totalCost),
        &dlg);
    header->setTextFormat(Qt::RichText);

    // table
    auto* table = new QTableWidget(sol.chosen.size(), 7, &dlg);
    table->setHorizontalHeaderLabels({"Slot", "Name", "Rating", "Nation", "Club", "Type", "Use"});
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setShowGrid(false);

    QHeaderView* hh = table->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(1, QHeaderView::Stretch);
    hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(6, QHeaderView::ResizeToContents);

    for (int s = 0; s < sol.chosen.size(); ++s) {
        const int cardIdx = sol.chosen[s];
        if (cardIdx < 0 || cardIdx >= m_lastSolverCards.size()) continue;

        const Card& c = m_lastSolverCards[cardIdx];
        const QString pos = s < m_lastFormation.size() ? m_lastFormation[s].pos : QString("?");

        const TypeStyle st = styleFortype(c.type);

        auto makeItem = [&](const QString& text, Qt::Alignment al = Qt::AlignLeft | Qt::AlignCenter){
            auto* it = new QTableWidgetItem(text);
            it->setTextAlignment(al);
            it->setBackground(st.bg);
            it->setForeground(st.fg);
            return it;
        };

        table->setItem(s, 0, makeItem(pos, Qt::AlignCenter));
        table->setItem(s, 1, makeItem(c.name));
        table->setItem(s, 2, makeItem(QString::number(c.rating), Qt::AlignCenter));
        table->setItem(s, 3, makeItem(c.country));
        table->setItem(s, 4, makeItem(c.club));

        auto* typeItem = makeItem(c.type, Qt::AlignCenter);
        QFont tf = typeItem->font();
        tf.setBold(true);
        typeItem->setFont(tf);
        table->setItem(s, 5, typeItem);

        auto* useItem = new QTableWidgetItem();
        useItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        useItem->setCheckState(Qt::Checked);
        useItem->setBackground(st.bg);
        table->setItem(s, 6, useItem);

        table->setRowHeight(s, 28);
    }

    // buttons: copy + close
    auto* buttons = new QDialogButtonBox(&dlg);
    auto* copyBtn = buttons->addButton("Copy lineup", QDialogButtonBox::ActionRole);
    auto* excludeBtn = buttons->addButton("Exclude selected", QDialogButtonBox::ActionRole);
    auto* closeBtn = buttons->addButton(QDialogButtonBox::Close);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(copyBtn, &QPushButton::clicked, &dlg, [&dlg, &sol, this, idx, copyBtn](){
        QString tsv;
        tsv += QString("Solution %1\tRating %2\tChemistry %3\n")
                   .arg(idx + 1)
                   .arg(sol.gameRating)
                   .arg(sol.chemistry);
        tsv += "Slot\tName\tRating\tNation\tClub\tType\n";
        for (int s = 0; s < sol.chosen.size(); ++s) {
            const int cardIdx = sol.chosen[s];
            if (cardIdx < 0 || cardIdx >= m_lastSolverCards.size()) continue;
            const Card& c = m_lastSolverCards[cardIdx];
            const QString pos = (s < m_lastFormation.size())
                                    ? m_lastFormation[s].pos : QString("?");

            tsv += QString("%1\t%2\t%3\t%4\t%5\t%6\n")
                       .arg(pos)
                       .arg(c.name)
                       .arg(c.rating)
                       .arg(c.country)
                       .arg(c.club)
                       .arg(c.type);
        }
        QApplication::clipboard()->setText(tsv);
        copyBtn->setText("Copied!");
        QTimer::singleShot(1200, copyBtn, [copyBtn](){ copyBtn->setText("Copy lineup"); });
    });
    bool excludeAndResolve = false;
    connect(excludeBtn, &QPushButton::clicked, &dlg, [&dlg, &excludeAndResolve](){
        excludeAndResolve = true;
        dlg.accept();
    });

    auto* hint = new QLabel("Uncheck a card to mark it as unusable, then click <b>Exclude selected</b> to re-solve",
                            &dlg);
    hint->setTextFormat(Qt::RichText);
    hint->setWordWrap(true);

    auto* v = new QVBoxLayout(&dlg);
    v->addWidget(header);
    v->addWidget(table, 1);
    v->addWidget(hint);
    v->addWidget(buttons);

    dlg.exec();

    if (excludeAndResolve) {
        QSet<int> excluded;
        for (int s = 0; s < sol.chosen.size(); ++s) {
            auto* it = table->item(s, 6);
            if (!it || it->checkState() == Qt::Checked) continue;
            const int cardIdx = sol.chosen[s];
            if (cardIdx < 0 || cardIdx >= m_lastSolverCards.size()) continue;
            excluded.insert(m_lastSolverCards[cardIdx].id);
        }
        if (!excluded.isEmpty()) {
            QStringList ids;
            for (int id : excluded) ids << QString::number(id);
            ui->criteria_listwidget->addItem(QString("ExcludeCards(%1)").arg(ids.join(',')));
            QTimer::singleShot(0, this, &Helper::on_solve_button_clicked);
        }
    }

    m_lastSolutionDialogSize = dlg.size();
}

void Helper::on_criteria_listwidget_itemClicked(QListWidgetItem* item) {
    delete ui->criteria_listwidget->takeItem(ui->criteria_listwidget->row(item));
}

void Helper::on_add_button_clicked() {
    AddCriterionDialog dlg(db, this);

    if (dlg.exec() != QDialog::Accepted) return;

    const QString c = dlg.criterion();
    if (c.isEmpty()) return;

    ui->criteria_listwidget->addItem(c);
}






