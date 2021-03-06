LUA_VERSION := 5.3.4
LUA_NAME := lua-$(LUA_VERSION)
LUA_DL := $(LUA_NAME).tar.gz
LUA_DL_URL := "https://www.lua.org/ftp/$(LUA_DL)"
LUA_SRC_DIR := $(DEPS_DIR)/$(LUA_NAME)/src
LUA_INSTALL_DIR := $(BUILD_DIR)/$(LUA_NAME)

LUA_INCLUDE_DIR := $(LUA_INSTALL_DIR)/include
LUA_LIBDIR := $(LUA_INSTALL_DIR)/lib

# for external thins to use.
LUA_CFLAGS := -I$(LUA_INCLUDE_DIR)
LUA_LDFLAGS := -L$(LUA_LIBDIR)

LUA_LIB := $(LUA_LIBDIR)/liblua.a
LUA_BIN := $(PRIV_DIR)/lua
LUA_C   := $(PRIV_DIR)/luac

LUA_BUILD_CFLAGS ?= -Wall -std=gnu99 -DDEBUG -g
LUA_BUILD_LDFLAGS ?= -pthread

LUA_TARGET :=

ifeq ($(OS),Windows_NT)
LUA_TARGET := mingw
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
LUA_TARGET := linux
endif
ifeq ($(UNAME_S),Darwin)
LUA_TARGET := macosx
endif
endif

# Add build targets to global manifest.
# Don't add a clean task here, since it doesn't really need to ever be rebuilt.
ALL += $(LUA_LIB) $(LUA_BIN) $(LUA_C)
PHONY += lua_clean lua_fullclean

$(LUA_SRC_DIR):
	$(WGET)   $(LUA_DL_URL)
	$(TAR_XF) $(LUA_DL)
	$(RM)     $(LUA_DL)
	$(MV)     $(LUA_NAME) $(DEPS_DIR)
	cd $(DEPS_DIR)/$(LUA_NAME) && patch -p1 -i $(C_SRC_DIR)/lua/lua.patch

$(LUA_INSTALL_DIR):
	mkdir -p $(LUA_INSTALL_DIR)

$(LUA_LIB): | $(LUA_INSTALL_DIR) $(LUA_SRC_DIR)
	cd $(DEPS_DIR)/$(LUA_NAME) && make MYCFLAGS="$(LUA_BUILD_CFLAGS) -fPIC -DLUA_COMPAT_5_2 -DLUA_COMPAT_5_1" MYLDFLAGS="$(LUA_BUILD_LDFLAGS)" $(LUA_TARGET)
	cd $(DEPS_DIR)/$(LUA_NAME) && make -e TO_LIB="liblua.a liblua.so liblua.so.$(LUA_VERSION)" INSTALL_DATA='cp -d' INSTALL_TOP=$(LUA_INSTALL_DIR) INSTALL_MAN= INSTALL_LMOD= INSTALL_CMOD= install

$(LUA_BIN): $(LUA_LIB)
	cp $(LUA_INSTALL_DIR)/bin/lua $(LUA_BIN)

$(LUA_C): $(LUA_LIB)
	cp $(LUA_INSTALL_DIR)/bin/luac $(LUA_C)

lua_clean:
	cd $(LUA_SRC_DIR) && make clean

lua_fullclean: lua_clean
	$(RM) -r $(DEPS_DIR)/$(LUA_NAME)
	$(RM) -r $(LUA_INSTALL_DIR)
