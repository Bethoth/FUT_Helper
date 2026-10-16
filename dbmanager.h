#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QtSql/QSqlDatabase>
#include <QStringList>

class DbManager
{
public:
    DbManager(const QString& path, const QString& year_);
    ~DbManager();
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    int getLastId() const;
    int getLastRating() const;
    QString getLastType() const;
    QString getLastAdded() const;

    QStringList getCountries() const;
    QStringList getClubs() const;
    QStringList getClubsFromLeague(const QString& league) const;
    QStringList getLeagues() const;
    QStringList getTypes() const;
    QStringList getPlayers() const;
    QStringList getPlayersNames() const;
    QStringList getLayouts() const;

    QString getPlayer(int id) const;
    int getRating(int id) const;
    QString getPosition(int id) const;
    QString getCountry(int id) const;
    QString getClub(int id) const;
    QString getLeague(int id) const;
    QString getName(int id) const;
    QString getType(int id) const;
    QVector<int> getSames(int id) const;

    QStringList getPlayerInfoByName(const QString& name) const;

    QList<QPair<int, QString>> getAll() const;
    QList<QPair<int, QString>> getUnfilled() const;

    bool addCard(int id,
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
                 const QString& type);

    bool updateCard(int id,
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
                    int stat6);

    void addSamePlayer(int card, int same);
    void addNewPlayer(int id);

    bool executeSqlFile(const QString& filePath);

private:
    QSqlDatabase db;
    QString year;
    QString cards_table;
    QString clubs_table;
    QString sameplayers_table;
    QString types_table;
};

#endif // DBMANAGER_H
