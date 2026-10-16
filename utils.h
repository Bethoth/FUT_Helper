#ifndef UTILS_H
#define UTILS_H

#include <QList>
#include <QListWidget>
#include <QMessageBox>
#include <QCheckBox>

inline int getIdFromStrRep(const QList<QPair<int, QString>>& pairs, const QString& str_rep) {
    for (const auto& pair : pairs) {
        if (pair.second == str_rep) {
            return pair.first;
        }
    }
    return -1;
}

inline QStringList getListWidgetItems(const QListWidget& listwidget) {
    QStringList result;
    for (int i = 0; i < listwidget.count(); i++) {
        result << listwidget.item(i)->text();
    }
    return result;
}

inline QString getSingleCriteria(const QStringList& criteria_list, const QString& criteria) {
    return criteria_list.filter(criteria).at(0);
}

inline QString getCriteriaValue(const QString& criteria) {
    return criteria.split('(')[1].chopped(1);
}

inline QList<QStringList> permutations(const QStringList& qsl) {
    QList<QStringList> result;
    int n = qsl.size();
    for (int i = 0; i < n; i++) {
        QStringList perm;
        for (int j = 0; j < n; j++) {
            perm << qsl[(i + j) % n];
        }
        result << perm;
    }
    return result;
}

inline bool allDifferent(const QVector<int>& v) {
    return std::all_of(v.constBegin(), v.constEnd(), [v](int x){return v.count(x) == 1;});
}

inline bool checkEntry(const QString& field, const QString *entered) {
    QMessageBox msg_box;
    msg_box.setText("The " + field + " you entered was not saved in the database yet.");
    msg_box.setInformativeText("Are you sure it is correct?\nYou entered \"" + *entered + "\".");
    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg_box.setDefaultButton(QMessageBox::Yes);

    return msg_box.exec() == QMessageBox::Yes;
}

inline QString capitalize(const QString& s) {
    QString res = "";
    for (const QString& word : s.split(' ')) {
        res += QString(word[0]).toUpper() + word.last(word.size() - 1).toLower() + " ";
    }
    return res.chopped(1);
}

inline void lockUnchecked(const QList<QCheckBox*>& checkboxes) {
    for (auto& cb : checkboxes) {
        if (!cb->isChecked()) cb->setCheckable(false);
    }
}

inline void unlockAll(const QList<QCheckBox*>& checkboxes) {
    for (auto& cb : checkboxes) {
        cb->setCheckable(true);
    }
}

inline void uncheckAll(const QList<QCheckBox*>& checkboxes) {
    for (auto& cb : checkboxes) {
        cb->setChecked(false);
    }
}

inline QStringList getSecPositions(const QList<QCheckBox*>& checkboxes) {
    QStringList result;
    for (const auto& cb : checkboxes) {
        if (cb->isChecked()) result << cb->text();
    }
    return result;
}

inline QStringList expandFormation(const QString& input) {
    static const QHash<QString, QStringList> kFormations = {
        {"3-5-2",         {"GK","CB","CB","CB","LM","CDM","CDM","CAM","RM","ST","ST"}},
        {"3-4-3",         {"GK","CB","CB","CB","LM","CM","CM","RM","LW","ST","RW"}},
        {"3-4-2-1",       {"GK","CB","CB","CB","LM","CM","CM","RM","CAM","CAM","ST"}},
        {"3-1-4-2",       {"GK","CB","CB","CB","LM","CM","CM","RM","CDM","ST","ST"}},
        {"3-4-1-2",       {"GK","CB","CB","CB","LM","CM","CM","RM","CAM","ST","ST"}},

        {"4-4-2",         {"GK","LB","CB","CB","RB","LM","CM","CM","RM","ST","ST"}},
        {"4-4-2 (2)",     {"GK","LB","CB","CB","RB","LM","CDM","CDM","RM","ST","ST"}},
        {"4-2-4",         {"GK","LB","CB","CB","RB","LW","CM","CM","RW","ST","ST"}},
        {"4-3-3",         {"GK","LB","CB","CB","RB","CM","CM","CM","LW","ST","RW"}},
        {"4-3-3 (2)",     {"GK","LB","CB","CB","RB","CM","CDM","CM","LW","ST","RW"}},
        {"4-3-3 (3)",     {"GK","LB","CB","CB","RB","CDM","CDM","CM","LW","ST","RW"}},
        {"4-3-3 (4)",     {"GK","LB","CB","CB","RB","CM","CAM","CM","LW","ST","RW"}},
        {"4-2-3-1",       {"GK","LB","CB","CB","RB","CDM","CDM","CAM","CAM","CAM","ST"}},
        {"4-3-1-2",       {"GK","LB","CB","CB","RB","CM","CM","CM","CAM","ST","ST"}},
        {"4-3-2-1",       {"GK","LB","CB","CB","RB","CM","CM","CM","CAM","CAM","ST"}},
        {"4-2-3-1 (2)",   {"GK","LB","CB","CB","RB","CDM","CDM","CAM","LM","RM","ST"}},
        {"4-2-1-3",       {"GK","LB","CB","CB","RB","CDM","CDM","CAM","LW","RW","ST"}},
        {"4-2-2-2",       {"GK","LB","CB","CB","RB","CDM","CDM","CAM","CAM","ST","ST"}},
        {"4-1-2-1-2",     {"GK","LB","CB","CB","RB","CDM","LM","RM","CAM","ST","ST"}},
        {"4-1-2-1-2 (2)", {"GK","LB","CB","CB","RB","CDM","CM","CM","CAM","ST","ST"}},
        {"4-1-3-2",       {"GK","LB","CB","CB","RB","CDM","LM","RM","CM","ST","ST"}},
        {"4-1-4-1",       {"GK","LB","CB","CB","RB","CDM","LM","CM","CM","RM","ST"}},
        {"4-4-1-1",       {"GK","LB","CB","CB","RB","LM","CM","CM","RM","CAM","ST"}},
        {"4-5-1",         {"GK","LB","CB","CB","RB","LM","CM","CAM","CAM","RM","ST"}},
        {"4-5-1 (2)",     {"GK","LB","CB","CB","RB","LM","CM","CM","CM","RM","ST"}},

        {"5-3-2",         {"GK","LB","CB","CB","CB","RB","CM","CM","CM","ST","ST"}},
        {"5-2-3",         {"GK","LB","CB","CB","CB","RB","CM","CM","LW","ST","RW"}},
        {"5-2-1-2",       {"GK","LB","CB","CB","CB","RB","CM","CM","CAM","ST","ST"}},
        {"5-4-1",         {"GK","LB","CB","CB","CB","RB","CM","CM","LM","RM","ST"}}
    };

    const QString trimmed = input.trimmed();

    if (trimmed.contains(',')) {
        QStringList parts = trimmed.split(',', Qt::SkipEmptyParts);
        for (auto& p : parts) p = p.trimmed();
        return parts;
    }

    for (auto it = kFormations.constBegin(); it != kFormations.constEnd(); ++it) {
        if (QString::compare(it.key(), trimmed, Qt::CaseInsensitive) == 0)
            return it.value();
    }

    return {trimmed};
}

inline QStringList formationNames() {
    return {
        "3-5-2", "3-4-3", "3-4-2-1", "3-1-4-2", "3-4-1-2",

        "4-4-2", "4-4-2 (2)", "4-2-4", "4-3-3", "4-3-3 (2)", "4-3-3 (3)", "4-3-3 (4)", "4-2-3-1", "4-3-1-2",
        "4-3-2-1", "4-2-3-1 (2)", "4-2-1-3", "4-2-2-2", "4-1-2-1-2", "4-1-2-1-2 (2)", "4-1-3-2", "4-1-4-1",
        "4-4-1-1", "4-5-1", "4-5-1 (2)",

        "5-3-2", "5-2-3", "5-2-1-2", "5-4-1"
    };
}

#endif // UTILS_H












