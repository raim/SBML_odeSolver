# Last changed Time-stamp: <2005-03-09 22:39:10 xtof>
# $Id: Makefile.am,v 1.1 2005/05/30 19:49:13 raimc Exp $
SOURCES = LibODES.i Makefile.PL
EXTRA_DIST = LibODES.i Makefile.PL t/*.t
LibODES_wrap.c: LibODES.i
	$(SWIG) -Wall -perl5 -proxy -I../../src/ LibODES.i
Makefile-perl: Makefile.PL
	$(PERL) Makefile.PL
all-local: LibODES_wrap.c Makefile-perl
	$(MAKE) -f Makefile-perl
install-exec-local: all-local
	$(MAKE) -f Makefile-perl install
check-local: all-local
	$(MAKE) -f Makefile-perl test
mostlyclean-local: Makefile-perl
	$(MAKE) -f Makefile-perl clean
