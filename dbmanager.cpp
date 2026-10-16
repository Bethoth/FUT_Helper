#include "dbmanager.h"

#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QFile>

DbManager::DbManager(const QString& path, const QString& year_) {
    if (!QFile::exists(path)) {
        qDebug() << "Error: database does not exist";
        return;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
    }
    else {
        qDebug() << "Database: connection okay";
    }

    year = year_;
    cards_table = QString("cards%1").arg(year);
    clubs_table = QString("clubs%1").arg(year);
    sameplayers_table = QString("sameplayers%1").arg(year);
    types_table = QString("types%1").arg(year);
}

DbManager::~DbManager() {
    if (db.isOpen()) {
        db.commit();
        db.close();
    }
}

int DbManager::getLastId() const {
    QSqlQuery query(QString("SELECT MAX(id) FROM %1").arg(cards_table));
    query.first();

    return query.value(0).toInt();
}

int DbManager::getLastRating() const {
    QSqlQuery query(QString("SELECT rating FROM %1 ORDER BY id DESC LIMIT 1").arg(cards_table));
    query.first();

    return query.value(0).toInt();
}

QString DbManager::getLastType() const {
    QSqlQuery query(QString("SELECT type FROM %1 ORDER BY id DESC LIMIT 1").arg(cards_table));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getLastAdded() const {
    QSqlQuery query(QString("SELECT name, rating, type FROM %1 ORDER BY id DESC LIMIT 1").arg(cards_table));
    query.first();

    const QString name = query.value(0).toString();
    const int rating = query.value(1).toInt();
    const QString type = query.value(2).toString();

    return QString("%1, %2, %3").arg(name).arg(rating).arg(type);
}

QStringList DbManager::getCountries() const {
    QSqlQuery query("SELECT name FROM countries");
    QStringList result;

    while (query.next()) {
        result << query.value(0).toString();
    }

    return result;
}

QStringList DbManager::getClubs() const {
    QSqlQuery query(QString("SELECT club FROM %1").arg(clubs_table));
    QStringList result;

    while (query.next()) {
        result << query.value(0).toString();
    }

    return result;
}

QStringList DbManager::getClubsFromLeague(const QString& league) const {
    QSqlQuery query(QString("SELECT club FROM %1 WHERE league = \"%2\"").arg(clubs_table).arg(league));
    QStringList result;
    qDebug() << "given clubs: ";

    while (query.next()) {
        qDebug() << query.value(0).toString();
        result << query.value(0).toString();
    }

    return result;
}

QStringList DbManager::getLeagues() const {
    QSqlQuery query(QString("SELECT DISTINCT league FROM %1").arg(clubs_table));
    QStringList result;

    while (query.next()) {
        result << query.value(0).toString();
    }

    return result;
}

QStringList DbManager::getTypes() const {
    QSqlQuery query(QString("SELECT type FROM %1").arg(types_table));
    QStringList result;

    while (query.next()) {
        result << query.value(0).toString();
    }

    return result;
}

QStringList DbManager::getPlayers() const {
    QSqlQuery query(QString("SELECT name, type, id FROM %1 JOIN (SELECT DISTINCT card FROM %2) ON card = id")
                        .arg(cards_table)
                        .arg(sameplayers_table));
    QStringList result;

    while (query.next()) {
        const QString name = query.value(0).toString();
        const QString type = query.value(1).toString();
        const int id = query.value(2).toInt();

        result << QString("%1, %2, %3").arg(name).arg(type).arg(id);
    }

    return result;
}

QStringList DbManager::getPlayersNames() const {
    QSqlQuery query(QString("SELECT name FROM %1 JOIN (SELECT DISTINCT card FROM %2) ON card = id")
                        .arg(cards_table)
                        .arg(sameplayers_table));
    QStringList result;

    while (query.next()) {
        result << query.value(0).toString();
    }

    return result;
}

// TODO : check this weird function and see if it can be refactored like the next one
QString DbManager::getPlayer(int id) const {
    QSqlQuery query(QString("SELECT name, rating, type FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    QString result;

    while (query.next()) {
        const QString name = query.value(0).toString();
        const int rating = query.value(1).toInt();
        const QString type = query.value(2).toString();

        result = QString("%1, %2, %3").arg(name).arg(rating).arg(type);
    }

    return result;
}

int DbManager::getRating(int id) const {
    QSqlQuery query(QString("SELECT rating FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toInt();
}

QString DbManager::getPosition(int id) const {
    QSqlQuery query(QString("SELECT position FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getCountry(int id) const {
    QSqlQuery query(QString("SELECT country FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getClub(int id) const {
    QSqlQuery query(QString("SELECT club FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getName(int id) const {
    QSqlQuery query(QString("SELECT name FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getType(int id) const {
    QSqlQuery query(QString("SELECT type FROM %1 WHERE id = %2")
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QString DbManager::getLeague(int id) const {
    QSqlQuery query(QString("SELECT league FROM %1 AS cl JOIN %2 AS ca ON ca.club = cl.club WHERE id = %3")
                        .arg(clubs_table)
                        .arg(cards_table)
                        .arg(id));
    query.first();

    return query.value(0).toString();
}

QVector<int> DbManager::getSames(int id) const {
    QSqlQuery query(QString("SELECT same FROM %1 WHERE card = %2")
                        .arg(sameplayers_table)
                        .arg(id));
    QVector<int> result;

    while (query.next()) {
        result << query.value(0).toInt();
    }

    return result;
}

QStringList DbManager::getPlayerInfoByName(const QString& name) const {
    QSqlQuery query(QString("SELECT id, name, rating, type, country, club FROM %1 JOIN %2 ON id = card WHERE "
                            "name = \"%3\"")
                        .arg(cards_table)
                        .arg(sameplayers_table)
                        .arg(name));
    QStringList result;

    while (query.next()) {
        result << QString("%1,%2,%3,%4,%5,%6")
                      .arg(query.value(0).toInt())
                      .arg(query.value(1).toString())
                      .arg(query.value(2).toInt())
                      .arg(query.value(3).toString())
                      .arg(query.value(4).toString())
                      .arg(query.value(5).toString())
                      .arg(query.value(6).toString());
    }

    qDebug() << result;

    return result;
}

QList<QPair<int, QString>> DbManager::getAll() const {
    QSqlQuery query(QString("SELECT id, name, rating, position, type FROM %1").arg(cards_table));
    QList<QPair<int, QString>> result;

    while (query.next()) {
        const int id = query.value(0).toInt();
        const QString name = query.value(1).toString();
        const int rating = query.value(2).toInt();
        const QString position = query.value(3).toString();
        const QString type = query.value(4).toString();

        const QString str_rep = QString("%1, %2, %3, %4")
                                    .arg(name)
                                    .arg(rating)
                                    .arg(position)
                                    .arg(type);

        result << QPair<int, QString>(id, str_rep);
    }

    return result;
}

QList<QPair<int, QString>> DbManager::getUnfilled() const {
    QSqlQuery query(QString("SELECT id, name, rating, position, type FROM %1 WHERE stat1 = 0 OR stat2 = 0 OR"
                            " stat3 = 0 OR stat4 = 0 OR stat5 = 0 OR stat6 = 0")
                        .arg(cards_table));
    QList<QPair<int, QString>> result;

    while (query.next()) {
        const int id = query.value(0).toInt();
        const QString name = query.value(1).toString();
        const int rating = query.value(2).toInt();
        const QString position = query.value(3).toString();
        const QString type = query.value(4).toString();

        const QString str_rep = QString("%1, %2, %3, %4")
                                    .arg(name)
                                    .arg(rating)
                                    .arg(position)
                                    .arg(type);

        result << QPair<int, QString>(id, str_rep);
    }

    return result;
}

bool DbManager::addCard(int id,
                        const QString& name,
                        int rating,
                        const QString& position,
                        const QString& secpos1,
                        const QString& secpos2,
                        const QString& secpos3,
                        const QString& secpos4,
                        const QString& secpos5,
                        const QString& country,
                        const QString& league,
                        const QString& club,
                        int stat1,
                        int stat2,
                        int stat3,
                        int stat4,
                        int stat5,
                        int stat6,
                        const QString& type) {
    bool success = false;

    QSqlQuery query;
    query.prepare(QString("INSERT INTO %1 VALUES (%2, \"%3\", %4, \"%5\", \"%6\", \"%7\", \"%8\", \"%9\", \"%10\", \"%11\", \"%12\","
                          " \"%13\", %14, %15, %16, %17, %18, %19, \"%20\")")
                      .arg(cards_table)
                      .arg(id)
                      .arg(name)
                      .arg(rating)
                      .arg(position)
                      .arg(secpos1)
                      .arg(secpos2)
                      .arg(secpos3)
                      .arg(secpos4)
                      .arg(secpos5)
                      .arg(country)
                      .arg(league)
                      .arg(club)
                      .arg(stat1)
                      .arg(stat2)
                      .arg(stat3)
                      .arg(stat4)
                      .arg(stat5)
                      .arg(stat6)
                      .arg(type));

    if (query.exec()) {
        success = true;
    }
    else {
        qDebug() << "addCard error: " << query.lastError();
    }

    return success;
}

bool DbManager::updateCard(int id,
                           const QString& secpos1,
                           const QString& secpos2,
                           const QString& secpos3,
                           const QString& secpos4,
                           const QString& secpos5,
                           const QString& country,
                           const QString& league,
                           const QString& club,
                           int stat1,
                           int stat2,
                           int stat3,
                           int stat4,
                           int stat5,
                           int stat6) {
    bool success = false;

    QSqlQuery query;
    query.prepare(QString("UPDATE %1 SET sec_position1 = \"%2\", sec_position2 = \"%3\", sec_position3 = \"%4\","
                          " sec_position4 = \"%5\", sec_position5 = \"%6\", country = \"%7\", league = \"%8\", club = \"%9\", stat1 = %10,"
                          " stat2 = %11, stat3 = %12, stat4 = %13, stat5 = %14, stat6 = %15 WHERE id = %16")
                      .arg(cards_table)
                      .arg(secpos1)
                      .arg(secpos2)
                      .arg(secpos3)
                      .arg(secpos4)
                      .arg(secpos5)
                      .arg(country)
                      .arg(league)
                      .arg(club)
                      .arg(stat1)
                      .arg(stat2)
                      .arg(stat3)
                      .arg(stat4)
                      .arg(stat5)
                      .arg(stat6)
                      .arg(id));

    if (query.exec()) {
        success = true;
        qDebug() << "Successfully updated card with id " << id << " with: country = " << country
                 << ", club = " << club << ", stats = " << stat1 << stat2 << stat3 << stat4 << stat5 << stat6;
    }
    else {
        qDebug() << "updateCard error: " << query.lastError();
    }

    return success;
}

void DbManager::addSamePlayer(int card, int same) {
    QSqlQuery query;
    query.prepare(QString("INSERT INTO %1 VALUES (%2, %3)")
                      .arg(sameplayers_table)
                      .arg(card)
                      .arg(same));

    if (!query.exec()) {
        qDebug() << "addSamePlayer error: " << query.lastError();
    }
}

void DbManager::addNewPlayer(int id) {
    QSqlQuery query;
    query.prepare(QString("INSERT INTO %1 VALUES (%2, %3)")
                      .arg(sameplayers_table)
                      .arg(id)
                      .arg(id));

    if (!query.exec()) {
        qDebug() << "addNewPlayer error: " << query.lastError();
    }
}

bool DbManager::executeSqlFile(const QString& filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open SQL file:" << file.errorString();
        return false;
    }

    QString sql = QString::fromUtf8(file.readAll());
    file.close();

    qDebug() << "got after reading the sql file";

    QSqlQuery query;

    if (!query.exec(sql)) {
        qDebug() << "SQL execution failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QString playerIds = query.value(0).toString();
        int distinctNations = query.value(1).toInt();
        int distinctTypes = query.value(2).toInt();
        int chemistry = query.value(3).toInt();

        qDebug() << playerIds << distinctNations << distinctTypes << chemistry;
    }

    return true;
}
