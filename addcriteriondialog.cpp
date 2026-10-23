#include "addcriteriondialog.h"
#include "dbmanager.h"
#include "utils.h"

#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
const QStringList kKeys = {
    "Formation",
    "Clubs", "Leagues", "Nations", "Types", "ExcludeTypes", "FlexibleTypes",
    "Club=", "Club>=", "Club<=",
    "League=", "League>=", "League<=",
    "Nation=", "Nation>=", "Nation<=",
    "Type=", "Type>=", "Type<=",
    "Specials=", "Specials>=", "Specials<=",
    "IconHero=", "IconHero>=", "IconHero<=",
    "MinNations", "MaxNations",
    "MinTypes", "MaxTypes",
    "MinClubs", "MaxClubs",
    "MinLeagues", "MaxLeagues",
    "MinAvgRating", "MaxAvgRating",
    "MinChemistry", "MaxChemistry"
};

const QStringList kFormationPositions = {
    "GK", "LB", "CB", "RB", "CDM", "CM", "CAM", "LM", "RM", "ST", "LW", "RW"
};
}

AddCriterionDialog::AddCriterionDialog(const DbManager& db, QWidget* parent) : QDialog(parent), m_db(db) {
    setWindowTitle("Add criterion");
    setMinimumWidth(440);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems(kKeys);

    // page 0: single integer
    m_intSpin = new QSpinBox(this);
    m_intSpin->setRange(0, 99);
    auto* intPage = new QWidget(this);
    {
        auto* l = new QFormLayout(intPage);
        l->setContentsMargins(0, 0, 0, 0);
        l->addRow("Value:", m_intSpin);
    }

    // page 1: string + integer
    m_stringCombo = new QComboBox(this);
    m_stringCombo->setEditable(true);
    m_stringCombo->setInsertPolicy(QComboBox::NoInsert);
    m_stringIntSpin = new QSpinBox(this);
    m_stringIntSpin->setRange(0, 11);
    auto* strIntPage = new QWidget(this);
    {
        auto* l = new QFormLayout(strIntPage);
        l->setContentsMargins(0, 0, 0, 0);
        l->addRow("Value:", m_stringCombo);
        l->addRow("Count:", m_stringIntSpin);
    }

    // page 2: multi-string
    m_multiInput = new QComboBox(this);
    m_multiInput->setEditable(true);
    m_multiInput->setInsertPolicy(QComboBox::NoInsert);
    m_addValueBtn = new QPushButton("Add", this);
    m_removeValueBtn = new QPushButton("Remove", this);
    m_multiList = new QListWidget(this);
    auto* multiPage = new QWidget(this);
    {
        auto* v = new QVBoxLayout(multiPage);
        v->setContentsMargins(0, 0, 0, 0);

        auto* addRow = new QHBoxLayout;
        addRow->addWidget(new QLabel("Value:", multiPage));
        addRow->addWidget(m_multiInput, 1);
        addRow->addWidget(m_addValueBtn);
        v->addLayout(addRow);

        v->addWidget(new QLabel("Selected values:", multiPage));
        v->addWidget(m_multiList, 1);

        auto* btnRow = new QHBoxLayout;
        btnRow->addStretch(1);
        btnRow->addWidget(m_removeValueBtn);
        v->addLayout(btnRow);
    }

    m_valueStack = new QStackedWidget(this);
    m_valueStack->addWidget(intPage);
    m_valueStack->addWidget(strIntPage);
    m_valueStack->addWidget(multiPage);

    // Preview
    m_preview = new QLabel(this);
    m_preview->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QFont mono = m_preview->font();
    mono.setStyleHint(QFont::Monospace);
    m_preview->setFont(mono);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okBtn = buttons->button(QDialogButtonBox::Ok);

    auto* main = new QVBoxLayout(this);
    auto* typeRow = new QFormLayout;
    typeRow->addRow("Criterion:", m_typeCombo);
    main->addLayout(typeRow);
    main->addWidget(m_valueStack, 1);
    main->addSpacing(6);
    main->addWidget(new QLabel("Preview:", this));
    main->addWidget(m_preview);
    main->addWidget(buttons);

    // Wiring
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AddCriterionDialog::on_type_changed);
    connect(m_addValueBtn, &QPushButton::clicked, this, &AddCriterionDialog::on_add_value);
    connect(m_removeValueBtn, &QPushButton::clicked, this, &AddCriterionDialog::on_remove_value);
    connect(m_multiInput->lineEdit(), &QLineEdit::returnPressed, this, &AddCriterionDialog::on_add_value);

    connect(m_intSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddCriterionDialog::update_preview);
    connect(m_stringIntSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddCriterionDialog::update_preview);
    connect(m_stringCombo, &QComboBox::currentTextChanged, this, &AddCriterionDialog::update_preview);
    connect(m_multiList, &QListWidget::itemSelectionChanged, this, &AddCriterionDialog::update_preview);

    connect(buttons, &QDialogButtonBox::accepted, this, &AddCriterionDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &AddCriterionDialog::reject);

    on_type_changed();
}

AddCriterionDialog::Shape AddCriterionDialog::shapeFor(const QString& key) const {
    static const QStringList kInts = {
        "MinNations", "MaxNations",
        "MinTypes", "MaxTypes",
        "MinClubs", "MaxClubs",
        "MinLeagues", "MaxLeagues",
        "MinAvgRating", "MaxAvgRating",
        "MinChemistry", "MaxChemistry",
        "Specials=", "Specials>=", "Specials<=",
        "IconHero=", "IconHero>=", "IconHero<="
    };
    static const QStringList kStrInts = {
        "Club=", "Club>=", "Club<=",
        "League=", "League>=", "League<=",
        "Nation=", "Nation>=", "Nation<=",
        "Type=", "Type>=", "Type<=",
    };
    if (kInts.contains(key)) return ShapeInt;
    if (kStrInts.contains(key)) return ShapeStringInt;
    return ShapeMulti;
}

QStringList AddCriterionDialog::completerSourceFor(const QString& key) const {
    if (key.startsWith("Club")) return m_db.getClubs();
    if (key.startsWith("League")) return m_db.getLeagues();
    if (key.startsWith("Nation")) return m_db.getCountries();
    if (key == "Types" || key == "ExcludeTypes" || key == "FlexibleTypes" || key.startsWith("Type"))
        return m_db.getTypes();
    if (key == "Formation") {
        QStringList src = formationNames();
        src += kFormationPositions;
        return src;
    }
    return {};
}

void AddCriterionDialog::refreshCompleter() {
    const Shape sh = shapeFor(m_typeCombo->currentText());
    const QStringList src = completerSourceFor(m_typeCombo->currentText());

    if (m_activeCompleter) {
        delete m_activeCompleter;
        m_activeCompleter = nullptr;
    }

    QComboBox* target = (sh == ShapeStringInt) ? m_stringCombo : m_multiInput;
    QComboBox* other = (sh == ShapeStringInt) ? m_multiInput : m_stringCombo;

    other->setCompleter(nullptr);
    other->clear();
    other->setCurrentText("");

    target->clear();
    target->addItems(src);
    target->setCurrentText("");

    if (!src.isEmpty()) {
        m_activeCompleter = new QCompleter(src, this);
        m_activeCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_activeCompleter->setFilterMode(Qt::MatchContains);
        target->setCompleter(m_activeCompleter);
    }
}

void AddCriterionDialog::on_type_changed() {
    switch (shapeFor(m_typeCombo->currentText())) {
    case ShapeInt: m_valueStack->setCurrentIndex(0); break;
    case ShapeStringInt: m_valueStack->setCurrentIndex(1); break;
    case ShapeMulti: m_valueStack->setCurrentIndex(2); break;
    }

    m_intSpin->setValue(0);
    m_stringIntSpin->setValue(0);
    m_multiList->clear();

    refreshCompleter();
    update_preview();

    switch (shapeFor(m_typeCombo->currentText())) {
    case ShapeInt: m_intSpin->setFocus(); m_intSpin->selectAll(); break;
    case ShapeStringInt: m_stringCombo->setFocus(); break;
    case ShapeMulti: m_multiInput->setFocus(); break;
    }
}

void AddCriterionDialog::on_add_value() {
    const QString text = m_multiInput->currentText().trimmed();
    if (text.isEmpty()) return;

    // Formation shortcut: expand "4-3-3" or a raw comma list into positions
    if (m_typeCombo->currentText() == "Formation") {
        const QStringList positions = expandFormation(text);
        for (const QString& p : positions) {
            if (!p.isEmpty()) m_multiList->addItem(p);
        }
        m_multiInput->setCurrentText("");
        update_preview();
        return;
    }

    const bool dup = allowDuplicates();
    for (const QString& raw : text.split(',', Qt::SkipEmptyParts)) {
        const QString v = raw.trimmed();
        if (v.isEmpty()) continue;
        if (dup || m_multiList->findItems(v, Qt::MatchExactly).isEmpty()) {
            m_multiList->addItem(v);
        }
    }
    m_multiInput->setCurrentText("");
    update_preview();
}

void AddCriterionDialog::on_remove_value() {
    const int row = m_multiList->currentRow();
    if (row < 0) return;
    delete m_multiList->takeItem(row);
    update_preview();
}

void AddCriterionDialog::update_preview() {
    const QString key = m_typeCombo->currentText();
    const Shape sh = shapeFor(key);

    QString value;
    bool canSubmit = true;

    if (sh == ShapeInt) {
        value = QString::number(m_intSpin->value());
    }
    else if (sh == ShapeStringInt) {
        value = QString("%1,%2").arg(m_stringCombo->currentText().trimmed())
                    .arg(m_stringIntSpin->value());
        if (m_stringCombo->currentText().trimmed().isEmpty())
            canSubmit = false;
    }
    else {
        QStringList vals;
        for (int i = 0; i < m_multiList->count(); ++i) {
            vals << m_multiList->item(i)->text();
        }
        value = vals.join(',');
        if (vals.isEmpty()) canSubmit = false;
        if (key == "Formation" && vals.size() != 11) canSubmit = false;
    }

    m_preview->setText(QString("%1(%2)").arg(key, value));
    m_okBtn->setEnabled(canSubmit);
}

void AddCriterionDialog::accept() {
    if (shapeFor(m_typeCombo->currentText()) == ShapeMulti) {
        const QString pending = m_multiInput->currentText().trimmed();
        if (!pending.isEmpty()) {
            if (m_typeCombo->currentText() == "Formation") {
                for (const QString& p : expandFormation(pending))
                    if (!p.isEmpty()) m_multiList->addItem(p);
            }
            else {
                const bool dup = allowDuplicates();
                for (const QString& raw : pending.split(',', Qt::SkipEmptyParts)) {
                    const QString v = raw.trimmed();
                    if (v.isEmpty()) continue;
                    if (dup || m_multiList->findItems(v, Qt::MatchExactly).isEmpty()) {
                        m_multiList->addItem(v);
                    }
                }
            }
            m_multiInput->setCurrentText("");
        }
        update_preview();
        if (!m_okBtn->isEnabled()) return;
    }
    QDialog::accept();
}

QString AddCriterionDialog::criterion() const {
    const QString key = m_typeCombo->currentText();
    const Shape sh = shapeFor(key);

    QString value;
    if (sh == ShapeInt) {
        value = QString::number(m_intSpin->value());
    }
    else if (sh == ShapeStringInt) {
        value = QString("%1,%2").arg(m_stringCombo->currentText().trimmed()).arg(m_stringIntSpin->value());
    }
    else {
        QStringList vals;
        for (int i = 0; i < m_multiList->count(); ++i) {
            vals << m_multiList->item(i)->text();
        }
        value = vals.join(',');
    }

    return QString("%1(%2)").arg(key, value);
}

bool AddCriterionDialog::allowDuplicates() const {
    return m_typeCombo->currentText() == "Formation";
}




















