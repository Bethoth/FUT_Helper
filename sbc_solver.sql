WITH RECURSIVE

-- 1. Define the formation
slots(slot_no, pos) AS (
	VALUES
		(1, "GK"),
		(2, "LB"),
		(3, "CB"),
		(4, "CB"),
		(5, "RB"),
		(6, "CDM"),
		(7, "CDM"),
		(8, "CM"),
		(9, "LW"),
		(10, "ST"),
		(11, "RW")
),

-- 2. Filter eligible cards
candidates AS (
	SELECT c.id, c.name, c.rating, c.position, c.sec_position1, c.sec_position2, c.sec_position3, c.sec_position4, c.sec_position5, c.country, c.league, c.club, c.type
	FROM cards27 c
	JOIN clubs27 cl ON cl.club = c.club AND cl.league = c.league
	JOIN countries co ON co.name = c.country
	JOIN types27 ty ON ty.type = c.type
	WHERE
		c.club = "Aston Villa"
		AND c.league = "Premier League"
		AND c.type NOT IN ("SBC Premium", "Evo", "Elite Evo")
),

-- 3. Create bitmasks to optimise computations
nation_lookup AS (
	SELECT country, ROW_NUMBER() OVER (ORDER BY country) - 1 AS bit_no
	FROM (SELECT DISTINCT country FROM candidates)
),

type_lookup AS (
	SELECT type, ROW_NUMBER() OVER (ORDER BY type) - 1 AS bit_no
	FROM (SELECT DISTINCT type FROM candidates)
),

candidates_indexed AS (
	SELECT
		c.*,
		1 << n.bit_no AS nation_bit,
		1 << t.bit_no AS type_bit
	FROM candidates c
	JOIN nation_lookup n ON n.country = c.country
	JOIN type_lookup t ON t.type = c.type
),

-- 4. Convert main and secondary positions into one row per card-position combination
candidate_positions AS (
	SELECT id, position AS pos FROM candidates_indexed
	UNION
	SELECT id, sec_position1 FROM candidates_indexed
	UNION
	SELECT id, sec_position2 FROM candidates_indexed
	UNION
	SELECT id, sec_position3 FROM candidates_indexed
	UNION
	SELECT id, sec_position4 FROM candidates_indexed
	UNION
	SELECT id, sec_position5 FROM candidates_indexed
),

-- 5. Build lineups slot by slot
squads(
	slot_no,
	last_id,
	player_ids,
	nation_mask,
	nation_count,
	type_mask,
	type_count
) AS (
	-- Initial slot: goalkeeper
	SELECT
		1,
		c.id,
		CAST(c.id AS TEXT),
		c.nation_bit,
		1,
		c.type_bit,
		1
	FROM slots sl
	JOIN candidate_positions cp ON cp.pos = sl.pos
	JOIN candidates_indexed c ON c.id = cp.id
	WHERE sl.slot_no = 1
	
	UNION ALL
	
	-- Add a player to the next formation slot
	SELECT
		nxt.slot_no,
		c.id,
		s.player_ids || "," || c.id,
		s.nation_mask | c.nation_bit,
		s.nation_count + CASE
			WHEN (s.nation_mask & c.nation_bit) = 0
			THEN 1
			ELSE 0
		END,
		s.type_mask | c.type_bit,
		s.type_count + CASE
			WHEN (s.type_mask & c.type_bit) = 0
			THEN 1
			ELSE 0
		END
	FROM squads s
	JOIN slots cur ON cur.slot_no = s.slot_no
	JOIN slots nxt ON nxt.slot_no = s.slot_no + 1
	JOIN candidate_positions cp ON cp.pos = nxt.pos
	JOIN candidates_indexed c ON c.id = cp.id
	
	-- A card cannot occupy two slots
	WHERE instr("," || s.player_ids || ",", "," || c.id || ",") = 0
	
	-- Order identical-position slots to avoid duplicates
	AND (cur.pos <> nxt.pos OR c.id > s.last_id)
	
	-- Do not use two cards for the same player
	AND NOT EXISTS (
		SELECT 1
		FROM sameplayers27 sp
		WHERE (
			sp.card = c.id AND instr("," || s.player_ids || ",", "," || sp.same || ",") > 0
			OR
			sp.same = c.id AND instr("," || s.player_ids || ",", "," || sp.card || ",") > 0
		)
	)
	
	-- Nation feasabilty pruning
	AND (
		s.nation_count
		+
		MIN(
			11 - s.slot_no, (
				SELECT COUNT(DISTINCT c2.country)
				FROM slots sl2
				JOIN candidate_positions cp2 ON cp2.pos = sl2.pos
				JOIN candidates_indexed c2 ON c2.id = cp2.id
				WHERE sl2.slot_no > s.slot_no AND (s.nation_mask & c2.nation_bit) = 0
			)
		)
	) >= 7
	
	-- Card type feasabilty pruning
	AND (
		s.type_count
		+
		MIN(
			11 - s.slot_no, (
				SELECT COUNT(DISTINCT c2.type)
				FROM slots sl2
				JOIN candidate_positions cp2 ON cp2.pos = sl2.pos
				JOIN candidates_indexed c2 ON c2.id = cp2.id
				WHERE sl2.slot_no > s.slot_no AND (s.type_mask & c2.type_bit) = 0
			)
		)
	) >= 3
)

-- 6. Return qualifying complete lineups
SELECT
	s.player_ids,
	s.nation_count AS distinct_nations,
	s.type_count AS distinct_types,
	33 AS chemistry
FROM squads s
WHERE s.slot_no = 11 AND s.nation_count >= 7 AND s.type_count >= 3 AND chemistry >= 30;