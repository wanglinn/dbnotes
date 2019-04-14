/* contrib/test/test--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION test" to load this file. \quit

CREATE OR REPLACE FUNCTION test(cstring)
RETURNS bool
AS '$libdir/test', 'test'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
