# contrib/dmp_concat/Makefile

MODULES = dmp_concat

EXTENSION = dmp_concat
DATA = dmp_concat--1.0.sql
PGFILEDESC = "dmp_concat - concat string, delimite by ','"

REGRESS = dmp_concat

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/dmp_concat
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif