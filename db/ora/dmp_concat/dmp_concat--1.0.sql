/* contrib/dmp_concat/dmp_concat--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION dmp_concat" to load this file. \quit

CREATE OR REPLACE FUNCTION ora_string_agg_transfn(internal, text)
RETURNS internal
AS '$libdir/dmp_concat', 'ora_string_agg_transfn'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION ora_string_agg_finalfn(internal)
RETURNS text
AS '$libdir/dmp_concat', 'ora_string_agg_finalfn'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION ora_distinct_string_agg_transfn(internal, text)
RETURNS internal
AS '$libdir/dmp_concat', 'ora_distinct_string_agg_transfn'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION ora_distinct_string_agg_finalfn(internal)
RETURNS text
AS '$libdir/dmp_concat', 'ora_distinct_string_agg_finalfn'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE dmp_concat(text)
(
 SFUNC = ora_string_agg_transfn,
 STYPE = internal,
 finalfunc = ora_string_agg_finalfn,
 PARALLEL = SAFE
);

CREATE AGGREGATE dmp_concat_distinct(text)
(
 SFUNC = ora_distinct_string_agg_transfn,
 STYPE = internal,
 finalfunc = ora_distinct_string_agg_finalfn,
 PARALLEL = SAFE
);

CREATE AGGREGATE dmp_concat_dup(text)
(
 SFUNC = ora_distinct_string_agg_transfn,
 STYPE = internal,
 finalfunc = ora_distinct_string_agg_finalfn,
 PARALLEL = SAFE
);
