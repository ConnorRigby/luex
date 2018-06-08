#include <erl_nif.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lualib.h>

#include "luex_nif.h"

static int l_luex_print(lua_State* L) {
  int nargs = lua_gettop(L);
  for (int i=1; i <= nargs; ++i) {
    enif_fprintf(stderr, lua_tostring(L, i));
    enif_fprintf(stderr, " ");
  }
  enif_fprintf(stderr, "\r\n");
  return 0;
}

static const struct luaL_Reg LUEX_NIF_LUA_LIB [] = {
  {"print", l_luex_print},
  {NULL, NULL}
};

static void rt_dtor(ErlNifEnv *env, void *obj) {
    resource_data_t *rd = (resource_data_t *)obj;
    enif_fprintf(stderr, "rt_dtor called\r\n");
    lua_close(rd->L);
}

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    priv_data_t* data = enif_alloc(sizeof(priv_data_t));
    if (data == NULL) return 1;

    data->atom_ok = enif_make_atom(env, "ok");
    data->atom_undefined = enif_make_atom(env, "undefined");
    data->atom_error = enif_make_atom(env, "error");
    data->atom_nil = enif_make_atom(env, "nil");

    *priv = (void*)data;

    resource_type = enif_open_resource_type(env, "Elixir.Luex", "luex_nif", &rt_dtor, ERL_NIF_RT_CREATE, NULL);
    return !resource_type;
}

static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    return 0;
}

static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
    // return load(env, priv, info);
    return 0;
}

static void unload(ErlNifEnv* env, void* priv) {
    enif_free(priv);
}

static ERL_NIF_TERM luex_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t *rd;

    ERL_NIF_TERM res;

    rd = enif_alloc_resource(resource_type, sizeof(resource_data_t));

    lua_state_t *L;
    L = luaL_newstate();

    // Open the lib, get the global variable _G
    luaL_openlibs(L);
    lua_getglobal(L, "_G");

    // Inject functions and pop _G
    luaL_setfuncs(L, LUEX_NIF_LUA_LIB, 0);
    lua_pop(L, 1);

    rd->L = L;

    res = enif_make_resource(env, rd);
    enif_release_resource(rd);

    return enif_make_tuple2(env, priv->atom_ok, res);
}

static ERL_NIF_TERM luex_dostring(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  priv_data_t* priv = enif_priv_data(env);
  resource_data_t *rd;

  if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd)) {
    return enif_make_badarg(env);
  }

  ErlNifBinary bin;

  if(!enif_inspect_binary(env, argv[1], &bin)) {
    return enif_make_badarg(env);
  }

  if(luaL_dostring(rd->L, bin.data) != LUA_OK) {
    enif_fprintf(stderr, "Error in script: \r\n\t%s\r\n", lua_tostring(rd->L, -1));
    return priv->atom_error;
  }
  return priv->atom_ok;
}
