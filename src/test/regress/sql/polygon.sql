-- *************testing built-in type polygon ****************
--
-- polygon logic
--
-- 3	      o
--	      |
-- 2	    + |
--	   /  |
-- 1	  # o +
--       /    |
-- 0	#-----o-+
--
--	0 1 2 3 4
--

CREATE TABLE POLYGON_TBL(f1 polygon);


INSERT INTO POLYGON_TBL(f1) VALUES ('(2.0,0.0),(2.0,4.0),(0.0,0.0)');

INSERT INTO POLYGON_TBL(f1) VALUES ('(3.0,1.0),(3.0,3.0),(1.0,0.0)');

-- degenerate polygons 
INSERT INTO POLYGON_TBL(f1) VALUES ('(0.0,0.0)');

INSERT INTO POLYGON_TBL(f1) VALUES ('(0.0,1.0),(0.0,1.0)');

-- bad polygon input strings 
INSERT INTO POLYGON_TBL(f1) VALUES ('0.0');

INSERT INTO POLYGON_TBL(f1) VALUES ('(0.0 0.0');

INSERT INTO POLYGON_TBL(f1) VALUES ('(0,1,2)');

INSERT INTO POLYGON_TBL(f1) VALUES ('(0,1,2,3');

INSERT INTO POLYGON_TBL(f1) VALUES ('asdf');


SELECT '' AS four, POLYGON_TBL.*;

-- overlap 
SELECT '' AS three, p.*
   FROM POLYGON_TBL p
   WHERE p.f1 && '(3.0,1.0),(3.0,3.0),(1.0,0.0)';

-- left overlap 
SELECT '' AS four, p.* 
   FROM POLYGON_TBL p
   WHERE p.f1 &< '(3.0,1.0),(3.0,3.0),(1.0,0.0)';

-- right overlap 
SELECT '' AS two, p.* 
   FROM POLYGON_TBL p
   WHERE p.f1 &> '(3.0,1.0),(3.0,3.0),(1.0,0.0)';

-- left of 
SELECT '' AS one, p.*
   FROM POLYGON_TBL p
   WHERE p.f1 << '(3.0,1.0),(3.0,3.0),(1.0,0.0)';

-- right of 
SELECT '' AS zero, p.*
   FROM POLYGON_TBL p
   WHERE p.f1 >> '(3.0,1.0),(3.0,3.0),(1.0,0.0)';

-- contained 
SELECT '' AS one, p.* 
   FROM POLYGON_TBL p
   WHERE p.f1 @ '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon;

-- same 
SELECT '' AS one, p.*
   FROM POLYGON_TBL p
   WHERE p.f1 ~= '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon;

-- contains 
SELECT '' AS one, p.*
   FROM POLYGON_TBL p
   WHERE p.f1 ~ '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon;

--
-- polygon logic
--
-- 3	      o
--	      |
-- 2	    + |
--	   /  |
-- 1	  / o +
--       /    |
-- 0	+-----o-+
--
--	0 1 2 3 4
--
-- left of 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon << '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS false;

-- left overlap 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon << '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS true;

-- right overlap 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon &> '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS true;

-- right of 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon >> '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS false;

-- contained in 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon @ '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS false;

-- contains 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon ~ '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS false;

-- same 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon ~= '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS false;

-- overlap 
SELECT '(2.0,0.0),(2.0,4.0),(0.0,0.0)'::polygon && '(3.0,1.0),(3.0,3.0),(1.0,0.0)'::polygon AS true;

