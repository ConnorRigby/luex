LUEX_NIF_SRC_DIR := $(C_SRC_DIR)/luex
LUEX_NIF_SRC     := $(wildcard $(LUEX_NIF_SRC_DIR)/*.c)
LUEX_NIF_OBJ     := $(LUEX_NIF_SRC:.c=.o)
LUEX_NIF         := $(PRIV_DIR)/luex_nif.so

LUEX_NIF_CFLAGS ?= -fPIC -O2 -DDEBUG -g -Wunused
LUEX_NIF_LDFLAGS ?= -fPIC -shared -pedantic

ALL   += luex_nif
CLEAN += luex_nif_clean
PHONY += luex_nif luex_nif_clean

luex_nif: $(LUA_LIB) $(LUEX_NIF)

luex_nif_clean:
	$(RM) $(LUEX_NIF)
	$(RM) $(LUEX_NIF_OBJ)

$(LUEX_NIF): $(LUEX_NIF_OBJ)
	$(CC) $^ $(ERL_LDFLAGS) $(LUA_LDFLAGS) $(LUEX_NIF_LDFLAGS) $(LUA_LIBDIR)/liblua.a -o $@

$(LUEX_NIF_SRC_DIR)/%.o: $(LUEX_NIF_SRC_DIR)/%.c
	$(CC) -c $(ERL_CFLAGS) $(LUA_CFLAGS) $(LUEX_NIF_CFLAGS) -o $@ $<
