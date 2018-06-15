#include <erl_nif.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lualib.h>

#include "luex_nif.h"

static int luex_print(lua_State* L) {
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
        enif_fprintf(stderr, luaL_checkstring(L, i));
        enif_fprintf(stderr, " ");
    }
    enif_fprintf(stderr, "\r\n");
    return 0;
}

static int luex_receive(lua_State* L) {
  void* user_data_ptr;
  resource_data_t* rd;
  priv_data_t* priv;
  ErlNifEnv* env;

  env = enif_alloc_env();
  lua_getglobal(L, "__RESOURCE_DATA__");
  user_data_ptr = lua_touserdata(L, -1);
  if(user_data_ptr == NULL) {
    luaL_error(L, "Could not find global resource object!\r\n");
  }
  rd = (resource_data_t*)user_data_ptr;

  lua_getglobal(L, "__PRIVATE_DATA__");
  user_data_ptr = lua_touserdata(L, -1);
  if(user_data_ptr == NULL) {
    luaL_error(L, "Could not find global private data object!\r\n");
  }

  priv = (priv_data_t*)user_data_ptr;

  enif_fprintf(stderr, "%p\r\n", rd);
  sem_wait(rd->mailbox_ready);
  sem_init(rd->mailbox_ready, 1, 0);

  ERL_NIF_TERM const *data = rd->mailbox;
  ERL_NIF_TERM const *array;
  int arity;
  if(!enif_get_tuple(env, *data, &arity, &array)) {
    luaL_error(L, "Error decoding mailbox data.\r\n");
  }

  for(int i = 0; i<arity; i++) {
    if(enif_compare(array[i], priv->atom_true) == 0) {
      lua_pushboolean(L, 1);
    } else if(enif_compare(array[i], priv->atom_false) == 0) {
      lua_pushboolean(L, 0);
    } else if(enif_compare(array[i], priv->atom_nil) == 0) {
      lua_pushnil(L);
    } else if(enif_is_number(env, array[i])) {
      double data;
      enif_get_double(env, array[i], &data);
      lua_pushnumber(L, data);
    }
    else {
      luaL_error(L, "Unknown type for term.\r\n");
    }
  }

  return arity;
}

static int luex_send(lua_State* L) {
    int nargs = lua_gettop(L);
    ERL_NIF_TERM send_data, tag;
    void* user_data_ptr;
    ErlNifEnv* env;
    priv_data_t* priv;
    resource_data_t* rd;
    env = enif_alloc_env();

    lua_getglobal(L, "__RESOURCE_DATA__");
    user_data_ptr = lua_touserdata(L, -1);
    if(user_data_ptr == NULL) {
      luaL_error(L, "Could not find global resource object!\r\n");
    }
    rd = (resource_data_t*)user_data_ptr;

    lua_getglobal(L, "__PRIVATE_DATA__");
    user_data_ptr = lua_touserdata(L, -1);
    if(user_data_ptr == NULL) {
      luaL_error(L, "Could not find global private data object!\r\n");
    }

    priv = (priv_data_t*)user_data_ptr;

    lua_getglobal(L, "__TAG__");
    if(lua_isnil(L, -1)) {
      tag = priv->atom_nil;
    } else if(lua_isstring(L, -1)) {
      ErlNifBinary output;
      const char* boop = lua_tostring(rd->L, -1);
      enif_alloc_binary(strlen(boop), &output);
      strcpy(output.data, boop);
      tag = enif_make_binary(env, &output);
    }
    lua_pop(L, 3);
    send_data = enif_make_tuple2(env, tag, lua_return_to_tuple(env, priv, L, nargs));
    enif_send(env, &rd->self, NULL, send_data);
    return 0;
}

static int luex_wrap(lua_State* L) {
  // int nargs = lua_gettop(L);
  lua_Debug ar;
  if(!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ar.name = "?"; /* Some default ? */
  else {
    lua_getinfo(L, "n", &ar);
    if (ar.name == NULL)
      ar.name = "?"; /* <-- PUT DEFAULT HERE */
  }
  // enif_fprintf(stderr, "hello??? %s\r\n", ar.name);

  lua_pushstring(L, ar.name);
  lua_setglobal(L, "__TAG__");
  luex_send(L);
  return luex_receive(L);
}

static const struct luaL_Reg LUEX_NIF_LUA_LIB [] = {
    {"print",    luex_print},
    {"send",     luex_send},
    {"receive",  luex_receive},
    {NULL, NULL}
};

static int map_put(ErlNifEnv *env, ERL_NIF_TERM map_in, ERL_NIF_TERM* map_out, ERL_NIF_TERM key, ERL_NIF_TERM value) {
    return enif_make_map_put(env, map_in, key, value, map_out);
}

static void rt_dtor(ErlNifEnv *env, void *obj) {
    resource_data_t* rd = (resource_data_t *)obj;
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
    data->atom_true = enif_make_atom(env, "true");
    data->atom_false = enif_make_atom(env, "false");
    data->atom_unknown_type = enif_make_atom(env, "unknown_type");
    data->atom_k_struct = enif_make_atom(env, "__struct__");
    data->atom_k_name = enif_make_atom(env, "name");
    data->atom_k_type = enif_make_atom(env, "type");
    data->atom_k_ptr = enif_make_atom(env, "ptr");
    data->atom_struct_name = enif_make_atom(env, "Elixir.Luex.OpaqueData");

    *priv = (void*)data;

    resource_type = enif_open_resource_type(env, "Elixir.Luex", "luex_nif", &rt_dtor, ERL_NIF_RT_CREATE, NULL);
    return !resource_type;
}

static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    return 0;
}

static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
    return load(env, priv, info);
}

static void unload(ErlNifEnv* env, void* priv) {
    enif_free(priv);
}

static ERL_NIF_TERM luex_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t* rd;
    ErlNifPid self;
    enif_self(env, &self);

    ERL_NIF_TERM res;

    rd = enif_alloc_resource(resource_type, sizeof(resource_data_t));

    lua_state_t *L;
    L = luaL_newstate();

    lua_pushlightuserdata(L, (void*)rd);
    lua_setglobal(L, "__RESOURCE_DATA__");

    lua_pushlightuserdata(L, (void*)priv);
    lua_setglobal(L, "__PRIVATE_DATA__");

    // Open the lib, get the global variable _G
    luaL_openlibs(L);
    lua_getglobal(L, "_G");

    // Inject functions and pop _G
    luaL_setfuncs(L, LUEX_NIF_LUA_LIB, 0);
    lua_pop(L, 1);

    rd->L = L;
    rd->self = self;
    rd->mailbox_ready = malloc(sizeof(sem_t));
    sem_init(rd->mailbox_ready, 1, 0);
    rd->mailbox = &priv->atom_nil;

    res = enif_make_resource(env, rd);
    enif_release_resource(rd);

    return enif_make_tuple2(env, priv->atom_ok, res);
}

// register_function(l, :add, 2)
static ERL_NIF_TERM luex_register_function(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  priv_data_t* priv = enif_priv_data(env);
  (void)priv; // not used.

  resource_data_t* rd;
  ERL_NIF_TERM function;
  char* function_char;
  unsigned int size;
  int arity;

  if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
      return enif_make_badarg(env);

  if(!enif_is_atom(env, argv[1]))
      return enif_make_badarg(env);
  function = argv[1];

  if(!enif_get_int(env, argv[2], &arity))
      return enif_make_badarg(env);


  enif_get_atom_length(env, function, &size, ERL_NIF_LATIN1);
  size = size * sizeof(char) + 1;
  function_char = malloc(size);
  enif_get_atom(env, function, function_char, size, ERL_NIF_LATIN1);
  enif_fprintf(stderr, "registering function: %s %d\r\n", function_char, arity);
  lua_pushcfunction(rd->L, luex_wrap);
  lua_setglobal(rd->L, function_char);

  free(function_char);
  return enif_make_resource(env, rd);
}

static ERL_NIF_TERM luex_into_mailbox(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  priv_data_t* priv = enif_priv_data(env);

  resource_data_t* rd;
  if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
      return enif_make_badarg(env);

  if(!enif_is_tuple(env, argv[1]))
    return enif_make_badarg(env);

  sem_post(rd->mailbox_ready);
  rd->mailbox = &argv[1];
  enif_fprintf(stderr, "%p\r\n", rd);
  return priv->atom_ok;
}

static ERL_NIF_TERM luex_dostring(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t* rd;
    ErlNifBinary input;

    if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
        return enif_make_badarg(env);

    if(!enif_inspect_binary(env, argv[1], &input))
        return enif_make_badarg(env);

    int top = lua_gettop(rd->L);

    // Make a copy of the data to ensure there is a null character at the end of it.
    char* data = calloc(input.size + 1, sizeof(char));
    memcpy(data, input.data, input.size);

    if(luaL_dostring(rd->L, data) != LUA_OK) {
        ErlNifBinary output;
        const char* boop = lua_tostring(rd->L, -1);
        enif_alloc_binary(strlen(boop), &output);
        strcpy(output.data, boop);
        return enif_make_tuple2(env, priv->atom_error, enif_make_binary(env, &output));
    }
    free(data);
    enif_release_binary(&input);
    int nresults = lua_gettop(rd->L) - top;
    return enif_make_tuple2(env, priv->atom_ok, lua_return_to_tuple(env, priv, rd->L, nresults));
}

static ERL_NIF_TERM luex_dofile(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t* rd;
    ErlNifBinary input;

    if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
        return enif_make_badarg(env);

    if(!enif_inspect_binary(env, argv[1], &input))
        return enif_make_badarg(env);

    int top = lua_gettop(rd->L);

    // Make a copy of the data to ensure there is a null character at the end of it.
    char* data = calloc(input.size + 1, sizeof(char));
    memcpy(data, input.data, input.size);

    if(luaL_dofile(rd->L, data) != LUA_OK) {
        ErlNifBinary output;
        const char* boop = lua_tostring(rd->L, -1);
        enif_alloc_binary(strlen(boop), &output);
        strcpy(output.data, boop);
        return enif_make_tuple2(env, priv->atom_error, enif_make_binary(env, &output));
    }

    free(data);
    enif_release_binary(&input);
    int nresults = lua_gettop(rd->L) - top;
    return enif_make_tuple2(env, priv->atom_ok, lua_return_to_tuple(env, priv, rd->L, nresults));
}

static ERL_NIF_TERM convert_ptr_to_opaque_map(ErlNifEnv *env, priv_data_t* priv, const void* ptr, int type, const char* name) {
    ErlNifBinary output;
    ERL_NIF_TERM map;
    enif_alloc_binary(strlen(name), &output);
    strcpy(output.data, name);

    map = enif_make_new_map(env);
    map_put(env, map, &map, priv->atom_k_struct, priv->atom_struct_name);
    map_put(env, map, &map, priv->atom_k_name, enif_make_binary(env, &output));
    map_put(env, map, &map, priv->atom_k_type, enif_make_int(env, type));
    map_put(env, map, &map, priv->atom_k_ptr, enif_make_int64(env, (size_t)ptr));

    return map;
}

static ERL_NIF_TERM convert_to_term(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int stack_index) {
    switch(lua_type(L, stack_index)) {
    case LUA_TNIL:
        return priv->atom_nil;
    case LUA_TNUMBER:
        return enif_make_double(env, lua_tonumber(L, stack_index));
    case LUA_TBOOLEAN:
        return lua_toboolean(L, stack_index) ? priv->atom_true : priv->atom_false;
    case LUA_TSTRING: {
        const char* ret_str = lua_tostring(L, stack_index);
        ErlNifBinary output;
        enif_alloc_binary(strlen(ret_str), &output);
        strcpy(output.data, ret_str);
        return enif_make_binary(env, &output);
    }
    case LUA_TTABLE:
        return convert_ptr_to_opaque_map(env, priv, lua_topointer(L, stack_index), LUA_TTABLE, "Ltable");
    case LUA_TFUNCTION:
        return convert_ptr_to_opaque_map(env, priv, lua_topointer(L, stack_index), LUA_TFUNCTION, "Lfunction");
    case LUA_TUSERDATA:
        return convert_ptr_to_opaque_map(env, priv, lua_topointer(L, stack_index), LUA_TUSERDATA, "Luserdata");
    case LUA_TTHREAD:
        return convert_ptr_to_opaque_map(env, priv, lua_topointer(L, stack_index), LUA_TTHREAD, "Lthread");
    case LUA_TLIGHTUSERDATA:
        return convert_ptr_to_opaque_map(env, priv, lua_topointer(L, stack_index), LUA_TLIGHTUSERDATA, "Llightuserdata");
    }
}

static ERL_NIF_TERM lua_return_to_tuple(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int nresults) {
    ERL_NIF_TERM ret[nresults];

    for(int i = 0; i < nresults; i++) {
        int stack_index = -1 * (nresults - i);
        ret[i] = convert_to_term(env, priv, L, stack_index);
    }

    return enif_make_tuple_from_array(env, ret, nresults);
}
