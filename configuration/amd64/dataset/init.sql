CREATE TABLE t_oil
(
region      text,
country     text,
year        int,
production  int,
consumption int
);

INSERT INTO t_oil (
region, country, year, production, consumption )
VALUES
-- North America - USA
('North America', 'USA', 1965, 9014, 11522),
('North America', 'USA', 1966, 9579, 12100),
-- ... (continue with the rest of the values)

-- North America - Canada
('North America', 'Canada', 1965, 920, 1108),
('North America', 'Canada', 1966, 1012, 1167),
-- ... (continue with the rest of the values)

-- North America - Mexico
('North America', 'Mexico', 1965, 362, 296),
('North America', 'Mexico', 1966, 370, 309),
-- ... (continue with the rest of the values)

-- Middle East - Iran
('Middle East', 'Iran', 1965, 1908, 134),
('Middle East', 'Iran', 1966, 2132, 148),
-- ... (continue with the rest of the values)

-- Middle East - Iraq
('Middle East', 'Iraq', 1965, 1313, NULL),
('Middle East', 'Iraq', 1966, 1392, NULL),
-- ... (continue with the rest of the values)

-- Middle East - Israel
('Middle East', 'Israel', 1965, NULL, 81),
('Middle East', 'Israel', 1966, NULL, 84),
-- ... (continue with the rest of the values)

-- Middle East - Kuwait
('Middle East', 'Kuwait', 1965, 2371, 104),
('Middle East', 'Kuwait', 1966, 2505, 101),
-- ... (continue with the rest of the values)

-- Middle East - Oman
('Middle East', 'Oman', 1965, NULL, NULL),
('Middle East', 'Oman', 1966, NULL, NULL),
-- ... (continue with the rest of the values)

-- Middle East - Qatar
('Middle East', 'Qatar', 1965, 233, 1),
('Middle East', 'Qatar', 1966, 291, 1),
-- ... (continue with the rest of the values)

-- Middle East - Saudi Arabien
('Middle East', 'Saudi Arabien', 1965, 2219, 391),
('Middle East', 'Saudi Arabien', 1966, 2615, 394),
-- ... (continue with the rest of the values)

-- Middle East - Syria
('Middle East', 'Syria', 1965, NULL, NULL),
('Middle East', 'Syria', 1966, NULL, NULL),
-- ... (continue with the rest of the values)

-- Middle East - United Arab Emirates
('Middle East', 'United Arab Emirates', 1965, 282, NULL),
('Middle East', 'United Arab Emirates', 1966, 360, NULL);