# contrib/test/Makefile

MODULES = test

EXTENSION = test
DATA = test--1.0.sql
PGFILEDESC = "test - concat string, delimite by ','"

REGRESS = test

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/test
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
