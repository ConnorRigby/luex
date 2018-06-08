LUAROCKS_VERSION := 2.4.4
LUAROCKS_NAME := luarocks-$(LUAROCKS_VERSION)
LUAROCKS_DL := v$(LUAROCKS_VERSION).tar.gz
LUAROCKS_DL_URL := "https://github.com/luarocks/luarocks/archive/$(LUAROCKS_DL)"

LUAROCKS_DIR := $(DEPS_DIR)/$(LUAROCKS_NAME)
LUAROCKS_BUILD_DIR := $(BUILD_DIR)/$(LUAROCKS_NAME)

LUAROCKS := $(LUAROCKS_BUILD_DIR)/bin/luarocks

ALL += $(LUAROCKS)
PHONY += luarocks_clean luarocks_fullclean

$(LUAROCKS_DIR):
	$(WGET)   $(LUAROCKS_DL_URL)
	$(TAR_XF) $(LUAROCKS_DL)
	$(RM)     $(LUAROCKS_DL)
	$(MV)     $(LUAROCKS_NAME) $(DEPS_DIR)

$(LUAROCKS_BUILD_DIR):
	$(MKDIR_P) $(LUAROCKS_BUILD_DIR)

$(LUAROCKS): | $(LUAROCKS_BUILD_DIR) $(LUAROCKS_DIR)
	cd $(LUAROCKS_DIR) && ./configure \
	--prefix=$(LUAROCKS_BUILD_DIR) \
	--with-lua=$(LUA_INSTALL_DIR) \
	--with-downloader=wget \
	--force-config && make build && make install

luarocks_clean:
	cd $(LUAROCKS_DIR) && make clean && make uninstall

luarocks_fullclean: lua_clean
	$(RM) -r $(C_SRC_DIR)/$(LUAROCKS_NAME)
	$(RM) -r $(LUAROCKS_BUILD_DIR)
