#  Copyright (C) 2001-2006 Artifex Software, Inc.
#  All Rights Reserved.
#
#  This software is provided AS-IS with no warranty, either express or
#  implied.
#
#  This software is distributed under license and may not be copied, modified
#  or distributed except as expressly authorized under the terms of that
#  license.  Refer to licensing information at http://www.artifex.com/
#  or contact Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134,
#  San Rafael, CA  94903, U.S.A., +1(415)492-9861, for further information.
#
# $Id$
# Partial makefile common to all Unix and Desqview/X configurations.
# This is the next-to-last part of the makefile for these configurations.

# Define the rule for building standard configurations.
STDDIRS:
	@if test ! -d $(BINDIR); then mkdir $(BINDIR); fi
	@if test ! -d $(GLGENDIR); then mkdir $(GLGENDIR); fi
	@if test ! -d $(GLOBJDIR); then mkdir $(GLOBJDIR); fi
	@if test ! -d $(AUXDIR); then mkdir $(AUXDIR); fi
	@if test ! -d $(PSGENDIR); then mkdir $(PSGENDIR); fi
	@if test ! -d $(PSGENDIR)/cups; then mkdir $(PSGENDIR)/cups; fi
	@if test ! -d $(PSOBJDIR); then mkdir $(PSOBJDIR); fi

# Define a rule for building profiling configurations.
PGDEFS=GENOPT='-DPROFILE' CFLAGS='$(CFLAGS_PROFILE) $(GCFLAGS) $(XCFLAGS)'\
 LDFLAGS='$(XLDFLAGS) -pg' XLIBS='Xt SM ICE Xext X11'\
 BUILDDIRPREFIX=$(PGDIRPREFIX)

pg:
	$(MAKE) $(PGDEFS) std

pgclean:
	$(MAKE) $(PGDEFS) clean

# Define a rule for building debugging configurations.
DEBUGDEFS=GENOPT='-DDEBUG' CFLAGS='$(CFLAGS_DEBUG) $(GCFLAGS) $(XCFLAGS)'\
 BUILDDIRPREFIX=$(DEBUGDIRPREFIX)

debug:
	$(MAKE) $(DEBUGDEFS) std

debugclean:
	$(MAKE) $(DEBUGDEFS) clean

# Emacs tags maintenance.

TAGS:
	etags -t $(GLSRC)*.[ch] $(PSSRC)*.[ch]
