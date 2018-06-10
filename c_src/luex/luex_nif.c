#include <erl_nif.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lualib.h>

#include "luex_nif.h"

static int l_luex_print(lua_State* L) {
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
        enif_fprintf(stderr, luaL_checkstring(L, i));
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
    data->atom_true = enif_make_atom(env, "true");
    data->atom_false = enif_make_atom(env, "false");
    data->atom_unknown_type = enif_make_atom(env, "unknown_type");

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
    ErlNifBinary input;

    if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
        return enif_make_badarg(env);

    if(!enif_inspect_binary(env, argv[1], &input))
        return enif_make_badarg(env);

    int top = lua_gettop(rd->L);

    // Make a copy of the data to ensure there is a null character at the end of it.
    char* data = calloc(input.size + 1, sizeof(char));
    memcpy(data, input.data, input.size);

    // if((lua_load(rd->L, enif_binary_reader, &input, "luex_dostring", "bt") || lua_pcall(rd->L, 0, LUA_MULTRET, 0)) != LUA_OK) {
    if(luaL_dostring(rd->L, data) != LUA_OK) {
        ErlNifBinary output;
        const char* boop = lua_tostring(rd->L, -1);
        enif_alloc_binary(strlen(boop), &output);
        strcpy(output.data, boop);
        return enif_make_tuple2(env, priv->atom_error, enif_make_binary(env, &output));
    }

    int nresults = lua_gettop(rd->L) - top;
    return enif_make_tuple2(env, priv->atom_ok, lua_result_to_erlang_term(env, priv, rd->L, nresults));
}

static ERL_NIF_TERM luex_dofile(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t *rd;
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

    int nresults = lua_gettop(rd->L) - top;
    return enif_make_tuple2(env, priv->atom_ok, lua_result_to_erlang_term(env, priv, rd->L, nresults));
}

static int map_put(ErlNifEnv *env, ERL_NIF_TERM map_in, ERL_NIF_TERM* map_out, ERL_NIF_TERM key, ERL_NIF_TERM value) {
    return enif_make_map_put(env, map_in, key, value, map_out);
}

static ERL_NIF_TERM convert_to_term(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int stack_index);
static ERL_NIF_TERM iterate_and_print(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, ERL_NIF_TERM map, int index);

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
    case LUA_TTABLE: {
        ERL_NIF_TERM map = enif_make_new_map(env);
        return iterate_and_print(env, priv, L, map, stack_index);
    }
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
    default:
        return priv->atom_unknown_type;
    }
}

// https://stackoverflow.com/questions/6137684/iterate-through-lua-table
static ERL_NIF_TERM iterate_and_print(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, ERL_NIF_TERM map, int index) {
    // Push another reference to the table on top of the stack (so we know
    // where it is, and this function can work for negative, positive and
    // pseudo indices
    lua_pushvalue(L, index);
    // stack now contains: -1 => table
    lua_pushnil(L);
    // stack now contains: -1 => nil; -2 => table
    while(lua_next(L, -2)) {
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(L, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
        const char *key = lua_tostring(L, -1);

        ErlNifBinary key_bin;
        enif_alloc_binary(strlen(key), &key_bin);
        strcpy(key_bin.data, key);
        ERL_NIF_TERM key_term = enif_make_binary(env, &key_bin);

        map_put(env, map, &map, key_term, convert_to_term(env, priv, L, -2));

        // const char *value = lua_tostring(L, -2);
        // printf("%s => %s\r\n", key, value);
        // pop value + copy of key, leaving original key
        lua_pop(L, 2);
        // stack now contains: -1 => key; -2 => table
    }
    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(L, 1);
    // Stack is now the same as it was on entry to this function
    return map;
}

static ERL_NIF_TERM lua_result_to_erlang_term(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int nresults) {
    ERL_NIF_TERM ret[nresults];

    for(int i = 0; i < nresults; i++) {
        int stack_index = -1 * (nresults - i);
        ret[i] = convert_to_term(env, priv, L, stack_index);
    }

    return enif_make_tuple_from_array(env, ret, nresults);
}
