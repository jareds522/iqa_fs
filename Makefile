# ==========================================================================
#   Copyright (C) 2016 Ken Pettit. All rights reserved.
#   Author: Ken Pettit <pettitkd@gmail.com>
#
#   nxfuse is a Linux FUSE filesystem that allows native mounting of
#          NuttX filesystems within Linux.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ==========================================================================

.silent:

-include .config

APPNAME     = iqa_fs

OBJDIR		=  obj
CXXOBJDIR	=  obj_cpp
DEPDIR		=  dep
SRCDIR      =  src
GENROMFS    = $(shell which genromfs)

CC          =  $(CROSS_COMPILE)gcc
CXX         =  $(CROSS_COMPILE)g++
LDFLAGS     +=  -g
LIBFILES    +=  -lm -lpthread
ifeq ($(RELEASE),)
CFLAGS      +=  -g -Wall -I include -I include/nuttx -DFAR= -DTRUE=1 -DFALSE=0 -Wno-unused-value -D_FILE_OFFSET_BITS=64
else
CFLAGS      +=  -O2 -Wall -I include -I include/nuttx -DFAR= -DTRUE=1 -DFALSE=0 -Wno-unused-value -D_FILE_OFFSET_BITS=64
endif

# The C and CPP files need to be in two different variables because they have different build rules.
# The 'wildcard' keyword in make just matches anything with that pattern.
CSRCS       =  $(wildcard $(SRCDIR)/*.c)
CXXSRCS     =  $(wildcard $(SRCDIR)/*.cpp)

ifeq ($(CONFIG_FS_ROMFS),y)
CSRCS      += $(wildcard $(SRCDIR)/romfs/*.c)
endif

ifeq ($(CONFIG_FS_SMARTFS),y)
CSRCS       += $(wildcard $(SRCDIR)/smartfs/*.c)
endif

# The two lines below strip the "src/" from the filename using pattern substitution (patsubst)
CSRCTMP     =  $(patsubst $(SRCDIR)/%,%,$(CSRCS))
CXXSRCTMP   =  $(patsubst $(SRCDIR)/%,%,$(CXXSRCS))

# The following two lines say "replace .c with .o" in all entries of the CSRCTMP / CXXSRCTMP variable
COBJTMP	    =  $(CSRCTMP:.c=.o)
CXXOBJTMP   =  $(CXXSRCTMP:.cpp=.o)

# The line below prepends "obj/" or "obj_cpp/" to the iqa_fs.o, diagrams.o, etc. files
OBJECTS     =  $(patsubst %,$(OBJDIR)/%,$(COBJTMP)) $(patsubst %,$(CXXOBJDIR)/%,$(CXXOBJTMP))

# These line define what the "automatically generated dependency" files look like
DEPS1       =  $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$(OBJECTS))
DEPS        =  $(patsubst $(CXXOBJDIR)/%.o,$(DEPDIR)/%.d,$(DEPS1))
CCONFIG     =  include/nuttx/config.h
CONFIG      =  .config

ifeq ($(V),)
Q = @
else
Q =
endif

all: init $(APPNAME)

# Since all is the first rule in the file, it is the default rule.
# and since init is after the colon, init is a dependency of 'all'.
# so init always runs.  And below we say it is .PHONY, so that tells
# make there is no real file associated with it, and the rule just
# needs to run.
#
# So after the ifeq test to ensure genromfs Linux utility is loaded,
# we are now creating the /tmp/diagrams directory and then copying
# our local diagrams directory to it.  Then specifying /tmp/diagrams
# as the source, which contains the diagrams directory. 
.PHONY: init clean context depend
init:
ifeq ($(GENROMFS),)
	@echo
	@echo Please sudo apt-get install genromfs first
	@echo
	@exit 1
endif
	$(Q)mkdir -p /tmp/diagrams
	$(Q)cp -rf diagrams /tmp/diagrams
	$(Q)cp .asceerc /tmp/diagrams
	$(Q)genromfs -d /tmp/diagrams -f diagrams.fs
	$(Q)xxd -i diagrams.fs > src/diagrams.c
	@rm -rf diagrams.fs
	@mkdir -p $(OBJDIR)/nxffs
	@mkdir -p $(DEPDIR)/nxffs
	@mkdir -p $(OBJDIR)/smartfs
	@mkdir -p $(DEPDIR)/smartfs
	@mkdir -p $(DEPDIR)/romfs
	@mkdir -p $(OBJDIR)/romfs
	@mkdir -p $(CXXOBJDIR)

#Include our built dependencies.  This is where those files get included in make.  
# The minus means "don't generate an error if the file doesn't exist"
# Did that help any, or was it too fast?  :)  haha it was a little fast but if you leave the comments here i can have documentation for later, thanks! :)
-include $(DEPS)

# Create dependencies
SOURCES = $(CSRCS) $(CXXSRCS)

.depend: init Makefile $(SOURCES)
	$(Q) $(MKDEP) $(ROOTDEPPATH) "$(CC)" -- $(CFLAGS) -- $(SOURCES) >Make.dep
	$(Q) touch $@

depend: .depend

# ============================================================
# Rule for compiling C source files.
# This says "any file that has the format obj/*.o that is
#  built from a file that looks like src/*.c, please use
#  this rule.
# ============================================================
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM -MT $(OBJDIR)/$*.o $(CFLAGS) $(SRCDIR)/$*.c > $(DEPDIR)/$*.d

# ============================================================
# Rule for compiling C++ source files.
# This says "any file that has the format obj_cpp/*.o that is
#  built from a file that looks like src/*.cpp, please use
#  this rule.
# ============================================================
$(CXXOBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo Compiling $<
	@$(CXX) $(CFLAGS) -c -o $@ $<
	@$(CXX) -MM -MT $(CXXOBJDIR)/$*.o $(CFLAGS) $(SRCDIR)/$*.cpp > $(DEPDIR)/$*.d

# The line above creates a "dependency" file automatically by scanning the C / CPP file
# and determining all headers that it includes.  That way if any header changes, then
# make knows to recompile the source file.  Same with the last line in the C rule.

# ========================
# Rule to build the app
#
# We have to use the g++ compiler for the link so it knows to pull
# in the c++ libraries automatically.  Was gcc previously.
# ========================
$(APPNAME): Makefile $(OBJECTS)
	@echo Linking $@
	@g++ $(LDFLAGS) $(OBJECTS) $(LIBFILES) -o $@

# =============================
# Rule to clean all build files
# =============================
.PHONY: clean
clean:
	@echo "=== cleaning ===";
	@rm -rf $(OBJDIR) $(DEPDIR) $(CXXOBJDIR)
	@rm -rf *.o
	@rm -f $(APPNAME) 


# ==================================
# Rule to install nxfuse 
#
# Note: 
#    This is a simple installation
#    in lieu of having an actual
#    autoconf configure script.
#
# This install script always installs
# nxfuse to /usr/bin and the man
# page to /usr/share/man/man1
#
# Run 'make install' with sudo
# ==================================
.PHONY: install
install: $(APPNAME)
	@echo "=== installing to /usr/bin ===";
	@cp $(APPNAME) /usr/bin
	@cp man/nxfuse.1 /usr/share/man/man1

