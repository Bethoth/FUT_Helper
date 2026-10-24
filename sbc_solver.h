#ifndef SBC_SOLVER_H
#define SBC_SOLVER_H

#include "dbmanager.h"

#include <QElapsedTimer>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include <array>
#include <atomic>
#include <cmath>

struct Card
{
    int id;
    QString name;
    int rating;
    QStringList positions;
    QString country, league, club, type;
};

struct Slot
{
    int slotNo;
    QString pos;
};

struct CountConstraint
{
    enum Field { Club = 0, League = 1, Nation = 2, Type = 3, SpecialType = 4, IconHeroType = 5 };
    Field field;
    QString value; // unused for SpecialType
    int minCount = 0;
    int maxCount = -1; // -1 = no upper bound
};

class SBC_Solver
{
public:
    SBC_Solver(const DbManager &db, int maxSolutions, const QStringList &baseExcludeTypes);

    SBC_Solver &club(const QString &c);
    SBC_Solver &clubs(const QStringList &cs);
    SBC_Solver &clubExactly(const QString &club, int n);
    SBC_Solver &clubAtLeast(const QString &club, int n);
    SBC_Solver &clubAtMost(const QString &club, int n);
    SBC_Solver &league(const QString &l);
    SBC_Solver &leagues(const QStringList &ls);
    SBC_Solver &leagueExactly(const QString &league, int n);
    SBC_Solver &leagueAtLeast(const QString &league, int n);
    SBC_Solver &leagueAtMost(const QString &league, int n);
    SBC_Solver &nation(const QString &n);
    SBC_Solver &nations(const QStringList &ns);
    SBC_Solver &nationExactly(const QString &nation, int n);
    SBC_Solver &nationAtLeast(const QString &nation, int n);
    SBC_Solver &nationAtMost(const QString &nation, int n);
    SBC_Solver &includeTypes(const QStringList &ts);
    SBC_Solver &excludeTypes(const QStringList &ts);
    SBC_Solver &typeExactly(const QString &type, int n);
    SBC_Solver &typeAtLeast(const QString &type, int n);
    SBC_Solver &typeAtMost(const QString &type, int n);
    SBC_Solver &specialExactly(int n);
    SBC_Solver &specialAtLeast(int n);
    SBC_Solver &specialAtMost(int n);
    SBC_Solver &iconHeroExactly(int n);
    SBC_Solver &iconHeroAtLeast(int n);
    SBC_Solver &iconHeroAtMost(int n);
    SBC_Solver &minAvgRating(int r);
    SBC_Solver &maxAvgRating(int r);
    SBC_Solver &minNations(int n);
    SBC_Solver &minTypes(int n);
    SBC_Solver &maxNations(int n);
    SBC_Solver &maxTypes(int n);
    SBC_Solver &minClubs(int n);
    SBC_Solver &maxClubs(int n);
    SBC_Solver &minLeagues(int n);
    SBC_Solver &maxLeagues(int n);
    SBC_Solver &formation(const QVector<Slot> &formation);
    SBC_Solver &minChemistry(int c);
    SBC_Solver &maxChemistry(int c = -1); // -1 = no upper bound
    SBC_Solver &flexibleTypes(const QStringList &ts);
    SBC_Solver &typeCost(const QString &type, int cost);
    SBC_Solver &excludeCardIds(const QSet<int> &ids);
    SBC_Solver &includeCardIds(const QSet<int> &ids);
    SBC_Solver &maxNodes(quint64 n);    // 0 = unlimited
    SBC_Solver &maxSeconds(double sec); // 0 = unlimited

    void prepare();
    void search();
    void solve();
    void printResults(int n) const;

    struct Solution
    {
        QVector<int> chosen;
        int chemistry = 0;
        int totalRating = 0;
        int totalCost = 0;
        double gameRating = 0.0;
    };
    const QVector<Solution> &getResults() const { return results; }
    const QVector<Card> &allCards() const { return cards; }
    const QVector<Slot> &getSlots() const { return slots_; }

    void cancel();

    bool hitNodeBudget() const { return m_hitNodeBudget; }
    bool hitTimeBudget() const { return m_hitTimeBudget; }
    quint64 nodeCount() const { return m_nodeCount.load(); }
    qint64 elapsedMs() const { return m_searchTimer.elapsed(); }

    QString leafRejectionReport() const;

    double computeRating(const QVector<int> &chosenMRV) const;

private:
    void recurse(int slotIdx,
                 QVector<int> &chosen,
                 QVector<char> &used,
                 QVector<int> &lastIdPerGroup,
                 QVector<int> &countHits,
                 int requiredPlacedCount,
                 int nationMask,
                 int nationCount,
                 int typeMask,
                 int typeCount,
                 int clubMask,
                 int clubCount,
                 int leagueMask,
                 int leagueCount,
                 int totalRating,
                 int totalCost,
                 int costCap);

    static inline int popcount(int x)
    {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_popcount(x);
#else
        int c = 0;
        while (x) {
            x &= x - 1;
            ++c;
        }
        return c;
#endif
    }

    static inline bool isSpecialType(const QString &type)
    {
        return type != "Gold" && type != "Silver" && type != "Bronze";
    }

    bool canSatisfyCount(int k, int minNeeded) const;

    const DbManager &db;
    int maxSolutions;

    QVector<Slot> slots_;

    QVector<Card> cards;
    QHash<int, int> idToIdx;

    QVector<QVector<int>> slotCandidates;

    QVector<QVector<int>> conflicts;

    QVector<int> m_mrvToFormation;

    QVector<Slot> orderedSlots;
    QVector<QVector<int>> orderedSlotsCandidates;
    QVector<int> slotGroup;
    int numGroups = 0;

    QHash<QString, int> nationBit, typeBit, clubBit, leagueBit;
    QVector<int> cardNationBit, cardTypeBit, cardClubBit, cardLeagueBit;
    QVector<int> suffixNationAvail, suffixTypeAvail, suffixClubAvail, suffixLeagueAvail;
    QVector<int> suffixRatingMinSum, suffixRatingMaxSum;

    QVector<CountConstraint> m_countConstraints;
    QVector<int> m_constraintMin;
    QVector<int> m_constraintMax;
    QVector<quint64> m_cardCountBits;
    QVector<QVector<int>> suffixCountAvail;

    QVector<Solution> results;
    std::atomic<bool> aborted{false};
    bool m_prepared = false;

    QStringList m_clubs;
    QStringList m_leagues;
    QStringList m_nations;
    QStringList m_includeTypes, m_baseExcludeTypes, m_specificExcludeTypes;
    int m_minAvgRating = 0, m_maxAvgRating = 99;
    int m_minNations = 0, m_maxNations = 11;
    int m_minTypes = 0, m_maxTypes = 11;
    int m_minClubs = 0, m_maxClubs = 11;
    int m_minLeagues = 0, m_maxLeagues = 11;
    int m_minChemistry = 0, m_maxChemistry = -1;
    QSet<int> m_excludedCardIds, m_includedCardIds;

    int computeChemistry(const QVector<int> &chosenMRV) const;
    QStringList m_flexibleTypes = {"Icon", "Debut Icon", "Hero", "Hall of FUT"};

    int cardCost(const Card &c) const;
    QHash<QString, int> m_typeCost;
    QVector<int> suffixMinCost, suffixMaxCost;
    int m_minPossibleCost = 0, m_maxPossibleCost = 0;

    quint64 m_maxNodes = 5'000'000;
    double m_maxSeconds = 0.0;
    std::atomic<bool> m_hitNodeBudget{false};
    std::atomic<bool> m_hitTimeBudget{false};
    QElapsedTimer m_searchTimer;
    std::atomic<quint64> m_nodeCount{0};

    enum LeafOutcome {
        LeafCostCapMismatch = 0,
        LeafCountMinFailed,
        LeafCountMaxFailed,
        LeafMinNations,
        LeafMaxNations,
        LeafMinTypes,
        LeafMaxTypes,
        LeafMinClubs,
        LeafMaxClubs,
        LeafMinLeagues,
        LeafMaxLeagues,
        LeafMinRating,
        LeafMaxRating,
        LeafMinChemistry,
        LeafMaxChemistry,
        LeafRequiredCountFailed,
        LeafAccepted,
        LeafOutcomeCount
    };
    std::array<quint64, LeafOutcomeCount> m_leafOutcomes{};

    double m_ratingBonusSlack = 0.0;
};

#endif // SBC_SOLVER_H
