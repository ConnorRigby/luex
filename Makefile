## Root Makefile.
# Available variables to be overwrote:
# MIX_ENV
# BUILD_DIR
# C_SRC_DIR
# PRIV_DIR
# ERL_EI_INCLUDE_DIR
# ERL_EI_LIBDIR

MIX_ENV   ?= dev
BUILD_DIR ?= $(PWD)/_build/$(MIX_ENV)
DEPS_DIR  ?= $(PWD)/deps
C_SRC_DIR ?= $(PWD)/c_src
PRIV_DIR  ?= $(PWD)/priv

# Commands that can be overwrote:
MKDIR_P ?= mkdir -p
MV      ?= mv
TAR_XF  ?= tar -xf
WGET    ?= wget

# Look for the EI library and header files
# For crosscompiled builds, ERL_EI_INCLUDE_DIR and ERL_EI_LIBDIR must be
# passed into the Makefile.
ifeq ($(ERL_EI_INCLUDE_DIR),)
$(warning ERL_EI_INCLUDE_DIR not set. Invoke via mix)
else
ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
endif
ifeq ($(ERL_EI_LIBDIR),)
$(warning ERL_EI_LIBDIR not set. Invoke via mix)
else
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR)
endif

ALL   := $(BUILD_DIR) $(DEPS_DIR) $(PRIV_DIR)
CLEAN :=
PHONY :=

include $(C_SRC_DIR)/lua/lua.Makefile
# include $(C_SRC_DIR)/lua/luarocks.Makefile
include $(C_SRC_DIR)/luex/luex.Makefile

.DEFAULT_GOAL := all

.PHONY: all clean $(PHONY)

all: $(ALL)

clean: $(CLEAN)

$(BUILD_DIR):
	$(MKDIR_P) $@

$(DEPS_DIR):
	$(MKDIR_P) $@

$(PRIV_DIR):
	$(MKDIR_P) $@
