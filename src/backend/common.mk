#
# Common make rules for backend
#
# $PostgreSQL: pgsql/src/backend/common.mk,v 1.3 2008/02/26 07:20:38 petere Exp $
#

# When including this file, set OBJS to the object files created in
# this directory and SUBDIRS to subdirectories containing more things
# to build.

ifdef PARTIAL_LINKING
# old style: linking using SUBSYS.o
subsysfilename = SUBSYS.o
else
# new style: linking all object files at once
subsysfilename = objfiles.txt
endif

SUBDIROBJS = $(SUBDIRS:%=%/$(subsysfilename))

# top-level backend directory obviously has its own "all" target
ifneq ($(subdir), src/backend)
all: $(subsysfilename)
endif

SUBSYS.o: $(SUBDIROBJS) $(OBJS)
	$(LD) $(LDREL) $(LDOUT) $@ $^

objfiles.txt:: $(MAKEFILE_LIST) | $(SUBDIROBJS) $(OBJS)
	( $(if $(SUBDIROBJS),cat $(SUBDIROBJS); )echo $(addprefix $(subdir)/,$(OBJS)) ) >$@

objfiles.txt:: $(SUBDIROBJS) $(OBJS)
	touch $@

# make function to expand objfiles.txt contents
expand_subsys = $(foreach file,$(filter %/objfiles.txt,$(1)),$(patsubst ../../src/backend/%,%,$(addprefix $(top_builddir)/,$(shell cat $(file))))) $(filter-out %/objfiles.txt,$(1))

# Parallel make trickery
$(SUBDIROBJS): $(SUBDIRS:%=%-recursive) ;

.PHONY: $(SUBDIRS:%=%-recursive)
$(SUBDIRS:%=%-recursive):
	$(MAKE) -C $(subst -recursive,,$@) all

clean: clean-local
clean-local:
ifdef SUBDIRS
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean || exit; done
endif
	rm -f $(subsysfilename) $(OBJS)
