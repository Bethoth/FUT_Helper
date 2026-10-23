#include "sbc_solver.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QElapsedTimer>
#include <functional>
#include <random>
#include <algorithm>

bool SBC_Solver::canSatisfyCount(int k, int minNeeded) const {
    const int S = slots_.size();

    // Adjacency: for each formation slot, the cards that can occupy it AND satisfy constraint k
    QVector<QVector<int>> adj(S);
    for (int s = 0; s < S; ++s) {
        for (int i : slotCandidates[s]) {
            if (m_cardCountBits[i] & (quint64(1) << k)) adj[s].append(i);
        }
    }

    // Kuhn's algorithm: augmenting-path bipartite matching
    QHash<int, int> cardToSlot;
    int matches = 0;
    for (int s = 0; s < S; ++s) {
        QVector<char> visited(cards.size(), 0);
        std::function<bool(int)> tryAugment = [&](int slot) -> bool {
            for (int i : adj[slot]) {
                if (visited[i]) continue;
                visited[i] = 1;
                if (!cardToSlot.contains(i)) {
                    cardToSlot[i] = slot;
                    return true;
                }
                if (tryAugment(cardToSlot[i])) {
                    cardToSlot[i] = slot;
                    return true;
                }
            }
            return false;
        };
        if (tryAugment(s)) ++matches;
    }
    return matches >= minNeeded;
}

SBC_Solver::SBC_Solver(const DbManager& db, int maxSolutions, const QStringList& baseExcludeTypes)
    : db(db), maxSolutions(maxSolutions), m_baseExcludeTypes(baseExcludeTypes) {}

SBC_Solver& SBC_Solver::club(const QString& c) {
    m_clubs.append(c);
    return *this;
}

SBC_Solver& SBC_Solver::clubs(const QStringList& cs) {
    m_clubs << cs;
    return *this;
}

SBC_Solver& SBC_Solver::clubExactly(const QString& club, int n) {
    m_countConstraints.append({CountConstraint::Club, club, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::clubAtLeast(const QString& club, int n) {
    m_countConstraints.append({CountConstraint::Club, club, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::clubAtMost(const QString& club, int n) {
    m_countConstraints.append({CountConstraint::Club, club, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::league(const QString& l) {
    m_leagues.append(l);
    return *this;
}

SBC_Solver& SBC_Solver::leagues(const QStringList& ls) {
    m_leagues << ls;
    return *this;
}

SBC_Solver& SBC_Solver::leagueExactly(const QString& league, int n) {
    m_countConstraints.append({CountConstraint::League, league, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::leagueAtLeast(const QString& league, int n) {
    m_countConstraints.append({CountConstraint::League, league, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::leagueAtMost(const QString& league, int n) {
    m_countConstraints.append({CountConstraint::League, league, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::nation(const QString& n) {
    m_nations.append(n);
    return *this;
}

SBC_Solver& SBC_Solver::nations(const QStringList& ns) {
    m_nations << ns;
    return *this;
}

SBC_Solver& SBC_Solver::nationExactly(const QString& nation, int n) {
    m_countConstraints.append({CountConstraint::Nation, nation, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::nationAtLeast(const QString& nation, int n) {
    m_countConstraints.append({CountConstraint::Nation, nation, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::nationAtMost(const QString& nation, int n) {
    m_countConstraints.append({CountConstraint::Nation, nation, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::includeTypes(const QStringList& ts) {
    m_includeTypes = ts;
    return *this;
}

SBC_Solver& SBC_Solver::excludeTypes(const QStringList& ts) {
    m_specificExcludeTypes << ts;
    return *this;
}

SBC_Solver& SBC_Solver::typeExactly(const QString& type, int n) {
    m_countConstraints.append({CountConstraint::Type, type, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::typeAtLeast(const QString& type, int n) {
    m_countConstraints.append({CountConstraint::Type, type, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::typeAtMost(const QString& type, int n) {
    m_countConstraints.append({CountConstraint::Type, type, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::specialExactly(int n) {
    m_countConstraints.append({CountConstraint::SpecialType, {}, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::specialAtLeast(int n) {
    m_countConstraints.append({CountConstraint::SpecialType, {}, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::specialAtMost(int n) {
    m_countConstraints.append({CountConstraint::SpecialType, {}, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::iconHeroExactly(int n) {
    m_countConstraints.append({CountConstraint::IconHeroType, {}, n, n});
    return *this;
}

SBC_Solver& SBC_Solver::iconHeroAtLeast(int n) {
    m_countConstraints.append({CountConstraint::IconHeroType, {}, n, -1});
    return *this;
}

SBC_Solver& SBC_Solver::iconHeroAtMost(int n) {
    m_countConstraints.append({CountConstraint::IconHeroType, {}, 0, n});
    return *this;
}

SBC_Solver& SBC_Solver::minAvgRating(int r) {
    m_minAvgRating = r;
    return *this;
}

SBC_Solver& SBC_Solver::maxAvgRating(int r) {
    m_maxAvgRating = r;
    return *this;
}

SBC_Solver& SBC_Solver::minNations(int n) {
    m_minNations = n;
    return *this;
}

SBC_Solver& SBC_Solver::maxNations(int n) {
    m_maxNations = n;
    return *this;
}

SBC_Solver& SBC_Solver::minTypes(int n) {
    m_minTypes = n;
    return *this;
}

SBC_Solver& SBC_Solver::maxTypes(int n) {
    m_maxTypes = n;
    return *this;
}

SBC_Solver& SBC_Solver::minClubs(int n) {
    m_minClubs = n;
    return *this;
}

SBC_Solver& SBC_Solver::maxClubs(int n) {
    m_maxClubs = n;
    return *this;
}

SBC_Solver& SBC_Solver::minLeagues(int n) {
    m_minLeagues = n;
    return *this;
}

SBC_Solver& SBC_Solver::maxLeagues(int n) {
    m_maxLeagues = n;
    return *this;
}

SBC_Solver& SBC_Solver::formation(const QVector<Slot>& formation) {
    slots_ = formation;
    return *this;
}

SBC_Solver& SBC_Solver::minChemistry(int c) {
    m_minChemistry = c;
    return *this;
}

SBC_Solver& SBC_Solver::maxChemistry(int c) {
    m_maxChemistry = c;
    return *this;
}

SBC_Solver& SBC_Solver::flexibleTypes(const QStringList& ts) {
    m_flexibleTypes = ts;
    return *this;
}

SBC_Solver& SBC_Solver::typeCost(const QString& type, int cost) {
    m_typeCost[type] = cost;
    return *this;
}

SBC_Solver& SBC_Solver::excludeCardIds(const QSet<int>& ids) {
    for (int id : ids) m_excludedCardIds.insert(id);
    return *this;
}

SBC_Solver& SBC_Solver::includeCardIds(const QSet<int>& ids) {
    for (int id : ids) m_includedCardIds.insert(id);
    return *this;
}

SBC_Solver& SBC_Solver::maxNodes(quint64 n) {
    m_maxNodes = n;
    return *this;
}

SBC_Solver& SBC_Solver::maxSeconds(double sec) {
    m_maxSeconds = sec;
    return *this;
}

int SBC_Solver::cardCost(const Card& c) const {
    auto it = m_typeCost.find(c.type);
    if (it != m_typeCost.end()) return it.value();

    const QString t = c.type.toLower();
    const int r = c.rating;

    // Silver / bronze: no surcharge
    if (t == "bronze") return 3;
    if (t == "silver") return 2;

    const bool isGold = t == "gold";

    int ratingAdd = 0;
    if (isGold) {
        if      (r >= 88) ratingAdd = 3;
        else if (r >= 84) ratingAdd = 2;
        else if (r >= 80) ratingAdd = 1;
    }
    else {
        if      (r >= 95) ratingAdd = 4;
        else if (r >= 90) ratingAdd = 3;
        else if (r >= 85) ratingAdd = 2;
        else if (r >= 80) ratingAdd = 1;
    }

    return (isGold ? 0 : 3) + ratingAdd;
}

void SBC_Solver::prepare() {
    m_prepared = false;

    cards.clear();
    idToIdx.clear();
    slotCandidates.clear();
    conflicts.clear();
    orderedSlots.clear();
    orderedSlotsCandidates.clear();
    slotGroup.clear();
    numGroups = 0;
    nationBit.clear();
    typeBit.clear();
    clubBit.clear();
    leagueBit.clear();
    cardNationBit.clear();
    cardTypeBit.clear();
    cardClubBit.clear();
    cardLeagueBit.clear();
    suffixNationAvail.clear();
    suffixTypeAvail.clear();
    suffixClubAvail.clear();
    suffixLeagueAvail.clear();
    m_constraintMin.clear();
    m_constraintMax.clear();
    m_cardCountBits.clear();
    suffixCountAvail.clear();
    suffixMinCost.clear();
    suffixMaxCost.clear();
    m_minPossibleCost = 0;
    m_maxPossibleCost = 0;
    m_ratingBonusSlack = 0.0;

    QString sql = R"SQL(
SELECT c.id, c.name, c.rating, c.position, c.sec_position1, c.sec_position2, c.sec_position3, c.sec_position4,
    c.sec_position5, c.country, c.league, c.club, c.type
FROM cards27 c
LEFT JOIN clubs27     cl ON cl.club = c.club AND cl.league = c.league
JOIN countries   co ON co.name = c.country
JOIN types27     ty ON ty.type = c.type
WHERE (c.club = '' OR cl.club IS NOT NULL)
)SQL";
    QVariantList binds;
    if (!m_clubs.empty()) {
        QStringList ph;
        for (int i = 0; i < m_clubs.size(); ++i) ph << "?";
        sql += " AND c.club IN (" + ph.join(",") + ")";
        for (const QString& c : m_clubs) binds << c;
    }
    if (!m_leagues.empty()) {
        QStringList ph;
        for (int i = 0; i < m_leagues.size(); ++i) ph << "?";
        sql += " AND c.league IN (" + ph.join(",") + ")";
        for (const QString& l : m_leagues) binds << l;
    }
    if (!m_nations.empty()) {
        QStringList ph;
        for (int i = 0; i < m_nations.size(); ++i) ph << "?";
        sql += " AND c.country IN (" + ph.join(",") + ")";
        for (const QString& n : m_nations) binds << n;
    }
    if (!m_includeTypes.empty()) {
        QStringList ph;
        for (int i = 0; i < m_includeTypes.size(); ++i) ph << "?";
        sql += " AND c.type IN (" + ph.join(",") + ")";
        for (const QString& t : m_includeTypes) binds << t;
    }
    if (!m_baseExcludeTypes.empty()) {
        QStringList ph;
        for (int i = 0; i < m_baseExcludeTypes.size(); ++i) ph << "?";
        sql += " AND c.type NOT IN (" + ph.join(",") + ")";
        for (const QString& t : m_baseExcludeTypes) binds << t;
    }
    if (!m_specificExcludeTypes.empty()) {
        QStringList ph;
        for (int i = 0; i < m_specificExcludeTypes.size(); ++i) ph << "?";
        sql += " AND c.type NOT IN (" + ph.join(",") + ")";
        for (const QString& t : m_specificExcludeTypes) binds << t;
    }
    sql += " ORDER BY c.id";

    QSqlQuery query;
    query.prepare(sql);
    for (const QVariant& b : binds) query.addBindValue(b);
    if (!query.exec()) {
        qDebug() << "Candidates query failed:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        const int cid = query.value(0).toInt();
        if (m_excludedCardIds.contains(cid)) continue;

        Card c;
        c.id = query.value(0).toInt();
        c.name = query.value(1).toString();
        c.rating = query.value(2).toInt();
        c.positions << query.value(3).toString();
        for (int i = 4; i <= 8; ++i) {
            const QVariant v = query.value(i);
            if (v.isNull()) continue;
            const QString s = v.toString();
            if (!s.isEmpty() && !c.positions.contains(s))
                c.positions << s;
        }
        c.country = query.value(9).toString();
        c.league = query.value(10).toString();
        c.club = query.value(11).toString();
        c.type = query.value(12).toString();

        idToIdx[c.id] = cards.size();
        cards.append(c);
    }
    const int N = cards.size();
    qDebug() << "Loaded " << N << "candidate cards";

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
    idToIdx.clear();
    for (int i = 0; i < N; ++i) idToIdx[cards[i].id] = i;

    cardNationBit.resize(N);
    cardTypeBit.resize(N);
    cardClubBit.resize(N);
    cardLeagueBit.resize(N);
    int nBit = 0, tBit = 0, cBit = 0, lBit = 0;
    for (int i = 0; i < N; ++i) {
        const Card& c = cards[i];

        if (!nationBit.contains(c.country)) nationBit[c.country] = nBit++;
        if (!typeBit.contains(c.type)) typeBit[c.type] = tBit++;
        if (!leagueBit.contains(c.league)) leagueBit[c.league] = lBit++;

        cardNationBit[i] = 1 << nationBit[c.country];
        cardTypeBit[i] = 1 << typeBit[c.type];
        cardLeagueBit[i] = 1 << leagueBit[c.league];

        // club can be empty (icons, heroes, ...) so special treatment
        if (c.club.isEmpty()) cardClubBit[i] = 0;
        else {
            if (!clubBit.contains(c.club)) clubBit[c.club] = cBit++;
            cardClubBit[i] = 1 << clubBit[c.club];
        }
    }

    slotCandidates.assign(slots_.size(), {});
    for (int s = 0; s < slots_.size(); ++s) {
        const QString& pos = slots_[s].pos;
        for (int i = 0; i < N; ++i) {
            if (cards[i].positions.contains(pos)) {
                slotCandidates[s].append(i);
            }
        }
    }

    // Conflicts ----------------------------

    QVector<int> parent(N);
    std::iota(parent.begin(), parent.end(), 0);

    std::function<int(int)> findRoot = [&](int x){
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    };
    auto unite = [&](int a, int b){
        const int ra = findRoot(a), rb = findRoot(b);
        if (ra != rb) parent[ra] = rb;
    };

    QSqlQuery spq;
    if (spq.exec("SELECT card, same FROM sameplayers27")) {
        while (spq.next()) {
            const int a = spq.value(0).toInt();
            const int b = spq.value(1).toInt();
            auto ia = idToIdx.find(a);
            auto ib = idToIdx.find(b);
            if (ia == idToIdx.end() || ib == idToIdx.end()) continue;
            unite(*ia, *ib);
        }
    }

    QHash<int, QVector<int>> groups;
    for (int i = 0; i < N; ++i) {
        groups[findRoot(i)].append(i);
    }

    conflicts.assign(N, {});
    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        const QVector<int>& members = it.value();
        if (members.size() < 2) continue;
        for (int i = 0; i < members.size(); ++i) {
            for (int j = i + 1; j < members.size(); ++j) {
                const int a = members[i], b = members[j];
                conflicts[a].append(b);
                conflicts[b].append(a);
            }
        }
    }

    // --------------------------------------

    QVector<int> order(slots_.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b){
        return slotCandidates[a].size() < slotCandidates[b].size();
    });
    orderedSlots.clear();
    orderedSlotsCandidates.clear();
    for (int s : order) {
        orderedSlots.append(slots_[s]);
        orderedSlotsCandidates.append(slotCandidates[s]);
    }
    for (auto& lst : orderedSlotsCandidates) {
        std::stable_sort(lst.begin(), lst.end(), [&](int a, int b){
            return cardCost(cards[a]) < cardCost(cards[b]);
        });
    }

    m_mrvToFormation.resize(orderedSlots.size());
    for (int mrv = 0; mrv < order.size(); ++mrv) {
        m_mrvToFormation[mrv] = order[mrv];
    }

    QHash<QString, int> posGroup;
    slotGroup.resize(orderedSlots.size());
    for (int i = 0; i < orderedSlots.size(); ++i) {
        const QString& p = orderedSlots[i].pos;
        if (!posGroup.contains(p)) {
            posGroup[p] = numGroups++;
        }
        slotGroup[i] = posGroup[p];
    }

    suffixNationAvail.assign(orderedSlots.size(), 0);
    suffixTypeAvail.assign(orderedSlots.size(), 0);
    suffixClubAvail.assign(orderedSlots.size(), 0);
    suffixLeagueAvail.assign(orderedSlots.size(), 0);
    int nMask = 0, tMask = 0, cMask = 0, lMask = 0;
    for (int s = orderedSlots.size() - 1; s >= 0; --s) {
        for (int idx : orderedSlotsCandidates[s]) {
            nMask |= cardNationBit[idx];
            tMask |= cardTypeBit[idx];
            cMask |= cardClubBit[idx];
            lMask |= cardLeagueBit[idx];
        }
        suffixNationAvail[s] = nMask;
        suffixTypeAvail[s] = tMask;
        suffixClubAvail[s] = cMask;
        suffixLeagueAvail[s] = lMask;
    }

    suffixRatingMinSum.assign(orderedSlots.size(), 0);
    suffixRatingMaxSum.assign(orderedSlots.size(), 0);
    int minSum = 0, maxSum = 0;
    for (int s = orderedSlots.size() - 1; s >= 0; --s) {
        int slotMin = INT_MAX, slotMax = 0;
        for (int idx : orderedSlotsCandidates[s]) {
            slotMin = qMin(slotMin, cards[idx].rating);
            slotMax = qMax(slotMax, cards[idx].rating);
        }
        if (slotMin == INT_MAX) slotMin = 0; // empty slot: no contribution
        minSum += slotMin;
        maxSum += slotMax;
        suffixRatingMinSum[s] = minSum;
        suffixRatingMaxSum[s] = maxSum;
    }

    const int C = m_countConstraints.size();
    m_constraintMin.resize(C);
    m_constraintMax.resize(C);
    for (int k = 0; k < C; ++k) {
        m_constraintMin[k] = m_countConstraints[k].minCount;
        m_constraintMax[k] = m_countConstraints[k].maxCount;
    }

    m_cardCountBits.assign(N, 0);
    for (int i = 0; i < N; ++i) {
        quint64 bits = 0;
        for (int k = 0; k < C && k < 64; ++k) {
            const auto& cc = m_countConstraints[k];
            bool matches = false;
            switch (cc.field) {
            case CountConstraint::Club: matches = !cards[i].club.isEmpty() && cards[i].club == cc.value; break;
            case CountConstraint::League: matches = cards[i].league == cc.value; break;
            case CountConstraint::Nation: matches = cards[i].country == cc.value; break;
            case CountConstraint::Type: matches = cards[i].type == cc.value; break;
            case CountConstraint::SpecialType: matches = isSpecialType(cards[i].type); break;
            case CountConstraint::IconHeroType:
                matches = cards[i].type == "Icon" || cards[i].type == "Hero" || cards[i].type == "Hall of FUT";
                break;
            }
            if (matches) bits |= (quint64(1) << k);
        }
        m_cardCountBits[i] = bits;
    }

    suffixCountAvail.assign(orderedSlots.size(), QVector<int>(C, 0));
    for (int s = orderedSlots.size() - 1; s >= 0; --s) {
        for (int k = 0; k < C; ++k) {
            const int next = (s + 1 < orderedSlots.size()) ? suffixCountAvail[s + 1][k] : 0;
            bool has = false;
            for (int idx : orderedSlotsCandidates[s]) {
                if (m_cardCountBits[idx] & (quint64(1) << k)) {
                    has = true;
                    break;
                }
            }
            suffixCountAvail[s][k] = next + (has ? 1 : 0);
        }
    }

    suffixMinCost.assign(orderedSlots.size(), 0);
    suffixMaxCost.assign(orderedSlots.size(), 0);
    int costMin = 0, costMax = 0;
    for (int s = orderedSlots.size() - 1; s >= 0; --s) {
        int slotMin = INT_MAX, slotMax = 0;
        for (int idx : orderedSlotsCandidates[s]) {
            const int k = cardCost(cards[idx]);
            slotMin = qMin(slotMin, k);
            slotMax = qMax(slotMax, k);
        }
        if (slotMin == INT_MAX) slotMin = 0;
        costMin += slotMin;
        costMax += slotMax;
        suffixMinCost[s] = costMin;
        suffixMaxCost[s] = costMax;
    }
    m_minPossibleCost = suffixMinCost[0];
    m_maxPossibleCost = suffixMaxCost[0];

    int poolMin = INT_MAX, poolMax = 0;
    for (const Card& c : cards) {
        poolMin = qMin(poolMin, c.rating);
        poolMax = qMax(poolMax, c.rating);
    }
    if (poolMin == INT_MAX) poolMin = 0;
    m_ratingBonusSlack = double(poolMax - poolMin) / 4.0; // theoretical value

    m_specificExcludeTypes.clear();
    m_prepared = true;
}

void SBC_Solver::search() {
    Q_ASSERT_X(m_prepared, "SBC_Solver::search",
               "prepare() must be called (on the main thread) before search()");

    if (!m_prepared) {
        qWarning() << "SBC_Solver::search called before prepare() — returning";
        return;
    }

    auto cannotSatisfy = [&](const QString& name, int requested, int available){
        if (requested > available) {
            qWarning() << name << requested << "impossible: pool only has" << available;
            return true;
        }
        return false;
    };

    if (cannotSatisfy("MinNations", m_minNations, popcount(suffixNationAvail[0]))) return;
    if (cannotSatisfy("MinTypes", m_minTypes, popcount(suffixTypeAvail[0])))       return;
    if (cannotSatisfy("MinClubs", m_minClubs, popcount(suffixClubAvail[0])))       return;
    if (cannotSatisfy("MinLeagues", m_minLeagues, popcount(suffixLeagueAvail[0]))) return;

    for (int k = 0; k < m_countConstraints.size(); ++k) {
        if (m_constraintMin[k] <= 0) continue;
        if (m_constraintMin[k] > suffixCountAvail[0][k]) {
            qWarning() << "Count constrait #" << k << "requires" << m_constraintMin[k] << "matches but only"
                       << suffixCountAvail[0][k] << "slots can host one";
            return;
        }
    }

    // Stronger check: for each constraint, verify a slot<->card matching of size >= minCount exists. Catches
    // structural impossibilities the suffix prune cannot see (e.g. 7 required but only 6 cards fit in the slots
    // that would accept them, despite 8 slots having candidates).
    for (int k = 0; k < m_countConstraints.size(); ++k) {
        if (m_constraintMin[k] <= 0) continue;
        if (!canSatisfyCount(k, m_constraintMin[k])) {
            qWarning() << "Count constraint #" << k << "requires" << m_constraintMin[k] << "matches but no valid"
                       << "slot <-> card assignment achieves it";
            return;
        }
    }

    results.clear();
    aborted = false;
    m_nodeCount = 0;
    m_hitNodeBudget = false;
    m_hitTimeBudget = false;
    m_leafOutcomes.fill(0);

    m_searchTimer.start();

    const int N = orderedSlots.size();

    QVector<int> chosen(N, -1);
    QVector<char> used(cards.size(), 0);
    QVector<int> lastIdPerGroup(numGroups, -1);
    QVector<int> countHits(m_countConstraints.size(), 0);

    QElapsedTimer timer; timer.start();

    recurse(0, chosen, used, lastIdPerGroup, countHits, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, INT_MAX);

    std::stable_sort(results.begin(), results.end(), [](const Solution& a, const Solution& b){
        return a.totalCost < b.totalCost;
    });

    if (m_hitNodeBudget)
        qWarning() << "Node budget exceeded after" << m_nodeCount.load() << "nodes," << results.size()
                   << "solutions found";
    else if (m_hitTimeBudget)
        qWarning() << "Time budget exceeded after" << m_searchTimer.elapsed() << "ms,"
                   << results.size() << "solutions found,"
                   << m_nodeCount.load() << "nodes explored";
    else
        qDebug() << "Search finalised in" << timer.elapsed() << "ms,"
                 << results.size() << "solutions found,"
                 << m_nodeCount.load() << "nodes explored"
                 << (aborted ? "(stopped early)" : "(exhausted)");

    qDebug().noquote() << leafRejectionReport();
}

void SBC_Solver::solve() {
    prepare();
    search();
}

void SBC_Solver::recurse(int slotIdx,
                         QVector<int>& chosen,
                         QVector<char>& used,
                         QVector<int>& lastIdPerGroup,
                         QVector<int>& countHits,
                         int requiredPlacedCount,
                         int nationMask, int nationCount,
                         int typeMask, int typeCount,
                         int clubMask, int clubCount,
                         int leagueMask, int leagueCount,
                         int totalRating,
                         int totalCost, int costCap) {
    if (aborted) return;
    if (costCap != INT_MAX && totalCost > costCap) return;

    ++m_nodeCount;

    // Node budget: cheap, checked every node
    if (m_maxNodes > 0 && m_nodeCount > m_maxNodes) {
        m_hitNodeBudget = true;
        aborted = true;
        return;
    }

    // Time budget: amortised. QElapsedTimer::elapsed is fast but not free;
    // checking every 1024 nodes keeps overhead below 0.01% at 1M nodes/s
    // while keeping detection latency around 1 ms
    if (m_maxSeconds > 0.0 && (m_nodeCount & 0x3FFu) == 0) {
        const auto t = m_searchTimer.elapsed();
        if (t > qint64(m_maxSeconds * 1000.0)) {
            m_hitTimeBudget = true;
            aborted = true;
            return;
        }
    }

    if (requiredPlacedCount + (orderedSlots.size() - slotIdx) < m_includedCardIds.size()) return;

    if (slotIdx == orderedSlots.size()) {
        if (costCap != INT_MAX && totalCost != costCap) {
            m_leafOutcomes[LeafCostCapMismatch]++;
            return;
        }

        for (int k = 0; k < m_countConstraints.size(); ++k) {
            if (countHits[k] < m_constraintMin[k]) {
                m_leafOutcomes[LeafCountMinFailed]++;
                return;
            }
            if (m_constraintMax[k] >= 0 && countHits[k] > m_constraintMax[k]) {
                m_leafOutcomes[LeafCountMaxFailed]++;
                return;
            }
        }

        if (nationCount < m_minNations) { m_leafOutcomes[LeafMinNations]++; return; }
        if (nationCount > m_maxNations) { m_leafOutcomes[LeafMaxNations]++; return; }
        if (typeCount < m_minTypes)     { m_leafOutcomes[LeafMinTypes]++; return; }
        if (typeCount > m_maxTypes)     { m_leafOutcomes[LeafMaxTypes]++; return; }
        if (clubCount < m_minClubs)     { m_leafOutcomes[LeafMinClubs]++; return; }
        if (clubCount > m_maxClubs)     { m_leafOutcomes[LeafMaxClubs]++; return; }
        if (leagueCount < m_minLeagues) { m_leafOutcomes[LeafMinLeagues]++; return; }
        if (leagueCount > m_maxLeagues) { m_leafOutcomes[LeafMaxLeagues]++; return; }

        const double gameRating = computeRating(chosen);
        if (gameRating < double(m_minAvgRating)) { m_leafOutcomes[LeafMinRating]++; return; }
        if (gameRating > double(m_maxAvgRating)) { m_leafOutcomes[LeafMaxRating]++; return; }

        const int chem = computeChemistry(chosen);
        if (chem < m_minChemistry) {
            m_leafOutcomes[LeafMinChemistry]++;
            return;
        }
        if (m_maxChemistry >= 0 && chem > m_maxChemistry) {
            m_leafOutcomes[LeafMaxChemistry]++;
            return;
        }

        if (requiredPlacedCount != m_includedCardIds.size()) {
            m_leafOutcomes[LeafRequiredCountFailed]++;
            return;
        }

        // All checks passed
        m_leafOutcomes[LeafAccepted]++;

        Solution sol;
        sol.chosen.resize(orderedSlots.size());
        for (int mrv = 0; mrv < chosen.size(); ++mrv) {
            sol.chosen[m_mrvToFormation[mrv]] = chosen[mrv];
        }
        sol.chemistry = chem;
        sol.totalRating = totalRating;
        sol.totalCost = totalCost;
        sol.gameRating = gameRating;
        results.append(sol);

        if (results.size() >= maxSolutions) aborted = true;
        return;
    }

    for (int k = 0; k < m_countConstraints.size(); ++k) {
        if (countHits[k] + suffixCountAvail[slotIdx][k] < m_constraintMin[k]) return;
    }

    const int remaining = orderedSlots.size() - slotIdx;

    const int newNations = popcount(suffixNationAvail[slotIdx] & ~nationMask);
    if (nationCount + qMin(remaining, newNations) < m_minNations) return;

    const int newTypes = popcount(suffixTypeAvail[slotIdx] & ~typeMask);
    if (typeCount + qMin(remaining, newTypes) < m_minTypes) return;

    const int newClubs = popcount(suffixClubAvail[slotIdx] & ~clubMask);
    if (clubCount + qMin(remaining, newClubs) < m_minClubs) return;

    const int newLeagues = popcount(suffixLeagueAvail[slotIdx] & ~leagueMask);
    if (leagueCount + qMin(remaining, newLeagues) < m_minLeagues) return;

    if (nationCount > m_maxNations) return;
    if (typeCount > m_maxTypes) return;
    if (clubCount > m_maxClubs) return;
    if (leagueCount > m_maxLeagues) return;

    const double N = double(orderedSlots.size());
    const double minPossibleRating = double(totalRating + suffixRatingMinSum[slotIdx]) / N;
    const double maxPossibleRating = double(totalRating + suffixRatingMaxSum[slotIdx]) / N + m_ratingBonusSlack;
    if (minPossibleRating > double(m_maxAvgRating)) return;
    if (maxPossibleRating < double(m_minAvgRating)) return;

    if (costCap != INT_MAX && totalCost + suffixMinCost[slotIdx] > costCap) return;
    if (costCap != INT_MAX && totalCost + suffixMaxCost[slotIdx] < costCap) return;

    const int grp = slotGroup[slotIdx];
    const int minIdSoFar = lastIdPerGroup[grp];

    QVector<char> forced(m_countConstraints.size(), 0);
    bool anyForced = false;
    for (int k = 0; k < m_countConstraints.size(); ++k) {
        if (m_constraintMin[k] <= 0) continue;
        // If we have exactly enough slots left to meet the minimum
        if (countHits[k] + suffixCountAvail[slotIdx][k] == m_constraintMin[k]) {
            bool slotCanMatch = false;
            for (int i : orderedSlotsCandidates[slotIdx]) {
                if (m_cardCountBits[i] & (quint64(1) << k)) {
                    slotCanMatch = true;
                    break;
                }
            }
            if (slotCanMatch) {
                forced[k] = 1;
                anyForced = true;
            }
        }
    }

    bool forceNewType = typeCount + qMin(remaining, newTypes) == m_minTypes && newTypes > 0;
    bool forceNewNation = nationCount + qMin(remaining, newNations) == m_minNations && newNations > 0;
    bool forceNewClub = clubCount + qMin(remaining, newClubs) == m_minClubs && newClubs > 0;
    bool forceNewLeague = leagueCount + qMin(remaining, newLeagues) == m_minLeagues && newLeagues > 0;

    // Dynamic Heuristic Sorting
    // If we are at risk of missing a min-constraint, score candidates based on how much diversity they provide
    // and sort them to explore them first

    // We add 1 to the minima to start prioritising slightly before it is forced
    bool needTypes =
        typeCount + qMin(remaining, popcount(suffixTypeAvail[slotIdx] & ~typeMask)) < m_minTypes + 1;
    bool needNations =
        nationCount + qMin(remaining, popcount(suffixNationAvail[slotIdx] & ~nationMask)) < m_minNations + 1;
    bool needClubs =
        clubCount + qMin(remaining, popcount(suffixClubAvail[slotIdx] & ~clubMask)) < m_minClubs + 1;
    bool needLeagues =
        leagueCount + qMin(remaining, popcount(suffixLeagueAvail[slotIdx] & ~leagueMask)) < m_minLeagues + 1;

    bool hasUnmetCounts = false;
    for (int k = 0; k < m_countConstraints.size(); ++k) {
        if (m_constraintMin[k] > countHits[k]) {
            hasUnmetCounts = true;
            break;
        }
    }

    bool hasRequired = requiredPlacedCount < m_includedCardIds.size();

    double currentMaxPossibleAvg = double(totalRating + suffixRatingMaxSum[slotIdx]) / N + m_ratingBonusSlack;
    bool needRating = currentMaxPossibleAvg < double(m_minAvgRating) + 2.0;

    QVector<int> sortedCandidates;

    // Only sort if there is actually something we are struggling to meet
    if (needTypes || needNations || needClubs || needLeagues || hasUnmetCounts || hasRequired || anyForced ||
        needRating) {
        QVector<QPair<int, int>> scored; // <score, cardIndex>

        for (int idx : orderedSlotsCandidates[slotIdx]) {
            int score = 0;

            if (needTypes && (cardTypeBit[idx] & ~typeMask))       score += 10;
            if (needNations && (cardNationBit[idx] & ~nationMask)) score += 10;
            if (needClubs && (cardClubBit[idx] & ~clubMask))       score += 10;
            if (needLeagues && (cardLeagueBit[idx] & ~leagueMask)) score += 10;

            if (needRating) score += cards[idx].rating - 70;

            for (int k = 0; k < m_countConstraints.size(); ++k) {
                if (m_constraintMin[k] > countHits[k] && (m_cardCountBits[idx] & (quint64(1) << k)))
                    score += 20;
            }

            if (hasRequired && m_includedCardIds.contains(cards[idx].id)) score += 50;

            scored.append({score, idx});
        }

        std::stable_sort(scored.begin(), scored.end(), [](const QPair<int, int>& a, const QPair<int, int>& b){
            return a.first > b.first;
        });

        for (const auto& p : scored) sortedCandidates.append(p.second);
    }
    else
        sortedCandidates = orderedSlotsCandidates[slotIdx];

    for (int idx : sortedCandidates) {
        if (aborted) return;

        if (anyForced) {
            bool satisfiesAll = true;
            for (int k = 0; k < m_countConstraints.size(); ++k) {
                if (forced[k] && !(m_cardCountBits[idx] & (quint64(1) << k))) {
                    satisfiesAll = false;
                    break;
                }
            }
            if (!satisfiesAll) continue;
        }

        if (forceNewType && !(cardTypeBit[idx] & ~typeMask)) continue;
        if (forceNewNation && !(cardNationBit[idx] & ~nationMask)) continue;
        if (forceNewClub && !(cardClubBit[idx] & ~clubMask)) continue;
        if (forceNewLeague && !(cardLeagueBit[idx] & ~leagueMask)) continue;

        if (used[idx]) continue;

        if (cards[idx].id <= minIdSoFar) continue;

        bool conflict = false;
        for (int c : conflicts[idx]) {
            if (used[c]) {
                conflict = true;
                break;
            }
        }
        if (conflict) continue;

        bool overMax = false;
        for (int k = 0; k < m_countConstraints.size(); ++k) {
            if ((m_cardCountBits[idx] & (quint64(1) << k)) &&
                m_constraintMax[k] >= 0 &&
                countHits[k] + 1 > m_constraintMax[k]) {
                overMax = true;
                break;
            }
        }
        if (overMax) continue;

        const int nb = cardNationBit[idx];
        const int tb = cardTypeBit[idx];
        const int lb = cardLeagueBit[idx];
        const int newNCount = nationCount + ((nationMask & nb) ? 0 : 1);
        const int newTCount = typeCount + ((typeMask & tb) ? 0 : 1);
        const int newLCount = leagueCount + ((leagueMask & lb) ? 0 : 1);

        // club can be empty (icons, heroes, ...) so special treatment
        const int cb = cardClubBit[idx];
        const int newCCount = clubCount + (cb == 0 || (clubMask & cb) ? 0 : 1);

        if (newNCount > m_maxNations) continue;
        if (newTCount > m_maxTypes) continue;
        if (newCCount > m_maxClubs) continue;
        if (newLCount > m_maxLeagues) continue;

        const int savedGroupId = lastIdPerGroup[grp];
        lastIdPerGroup[grp] = cards[idx].id;

        chosen[slotIdx] = idx;
        used[idx] = 1;

        const quint64 hitBits = m_cardCountBits[idx];
        for (int k = 0; k < m_countConstraints.size(); ++k) {
            if (hitBits & (quint64(1) << k)) countHits[k]++;
        }

        const bool isRequired = m_includedCardIds.contains(cards[idx].id);
        const int newRequiredCount = requiredPlacedCount + (isRequired ? 1 : 0);

        recurse(slotIdx + 1, chosen, used, lastIdPerGroup, countHits, newRequiredCount,
                nationMask | nb, newNCount,
                typeMask | tb, newTCount,
                clubMask | cb, newCCount,
                leagueMask | lb, newLCount,
                totalRating + cards[idx].rating,
                totalCost + cardCost(cards[idx]), costCap);

        for (int k = 0; k < m_countConstraints.size(); ++k) {
            if (hitBits & (quint64(1) << k)) countHits[k]--;
        }

        used[idx] = 0;
        chosen[slotIdx] = -1;
        lastIdPerGroup[grp] = savedGroupId;
    }
}

void SBC_Solver::printResults(int n) const {
    for (int r = 0; r < qMin(results.size(), n); ++r) {
        qDebug() << "--- Lineup" << r + 1 << "---";
        for (int s = 0; s < slots_.size(); ++s) {
            const Card& c = cards[results[r].chosen[s]];
            qDebug().nospace() << slots_[s].pos << "  "
                               << c.name << " (" << c.id << ")  "
                               << c.rating << "  " << c.country << " / " << c.type;
        }
    }
}

QString SBC_Solver::leafRejectionReport() const {
    struct Row { LeafOutcome outcome; const char* label; };
    static const Row rows[] = {
        { LeafCostCapMismatch,        "cost-cap mismatch"             },
        { LeafCountMinFailed,         "count-constraint min not met"  },
        { LeafCountMaxFailed,         "count-constraint max exceeded" },
        { LeafMinNations,             "min-nations"                   },
        { LeafMaxNations,             "max-nations"                   },
        { LeafMinTypes,               "min-types"                     },
        { LeafMaxTypes,               "max-types"                     },
        { LeafMinClubs,               "min-clubs"                     },
        { LeafMaxClubs,               "max-clubs"                     },
        { LeafMinLeagues,             "min-leagues"                   },
        { LeafMaxLeagues,             "max-leagues"                   },
        { LeafMinRating,              "min-rating"                    },
        { LeafMaxRating,              "max-rating"                    },
        { LeafMinChemistry,           "min-chemistry"                 },
        { LeafMaxChemistry,           "max-chemistry"                 },
        { LeafRequiredCountFailed,    "required cards count not met"  },
        { LeafAccepted,               "accepted"                      },
    };

    quint64 total = 0;
    for (quint64 v : m_leafOutcomes) total += v;

    auto out = QString("Leaf outcomes (%1 total):\n").arg(total);
    for (const Row& r : rows) {
        const quint64 v = m_leafOutcomes[r.outcome];
        if (v == 0) continue;
        const double pct = total ? 100.0 * double(v) / double(total) : 0.0;
        out += QString("  %1 %2  (%3)\n")
                   .arg(QString(r.label).leftJustified(32))
                   .arg(v)
                   .arg(pct, 0, 'f', 1);
    }

    return out;
}

int SBC_Solver::computeChemistry(const QVector<int>& chosenMRV) const {
    const int N = chosenMRV.size();
    if (N == 0) return 0;

    QHash<QString, int> leagueCount, nationCount, clubCount;
    int flexCount = 0;
    for (int s = 0; s < N; ++s) {
        const Card& c = cards[chosenMRV[s]];

        nationCount[c.country]++;

        if (m_flexibleTypes.contains(c.type)) {
            flexCount++;
        }
        else {
            leagueCount[c.league] += (c.type == "Squad Foundations" ? 2 : 1);
            clubCount[c.club]++;
        }
    }

    auto leagueScore = [](int n){ return n >= 8 ? 3 : n >= 5 ? 2 : n >= 3 ? 1 : 0; };
    auto nationScore = [](int n){ return n >= 8 ? 3 : n >= 5 ? 2 : n >= 2 ? 1 : 0; };
    auto clubScore = [](int n){ return n >= 7 ? 3 : n >= 4 ? 2 : n >= 2 ? 1 : 0; };

    int total = 0;
    for (int s = 0; s < N; ++s) {
        const Card& c = cards[chosenMRV[s]];
        if (m_flexibleTypes.contains(c.type)) {
            total += 3;
            continue;
        }
        else {
            const int chem =
                leagueScore(leagueCount[c.league] + flexCount) +
                nationScore(nationCount[c.country]) +
                clubScore(clubCount[c.club]);
            total += qMin(chem, 3);
        }
    }

    return total;
}

double SBC_Solver::computeRating(const QVector<int>& chosenMRV) const {
    const int N = chosenMRV.size();
    if (N == 0) return 0.0;

    int sum = 0;
    for (int s = 0; s < N; ++s) sum += cards[chosenMRV[s]].rating;

    const double avg1 = double(sum) / N;

    double bonus = 0.0;
    for (int s = 0; s < N; ++s) {
        const int r = cards[chosenMRV[s]].rating;
        if (r > avg1) bonus += r - avg1;
    }

    const double newTotal = sum + bonus;
    const double roundedTotal = std::round(newTotal);
    const double avg2 = roundedTotal / N;
    return std::floor(avg2);
}

void SBC_Solver::cancel() {
    aborted = true;
}



















