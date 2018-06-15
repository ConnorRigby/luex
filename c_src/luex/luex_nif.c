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

static int l_send(lua_State* L) {
    int nargs = lua_gettop(L);
    ERL_NIF_TERM send_data;
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
    lua_pop(L, 2);

    send_data = enif_make_tuple2(env, priv->atom_ok, lua_return_to_tuple(env, priv, L, nargs));
    enif_send(env, &rd->self, NULL, send_data);
    return 0;
}

static const struct luaL_Reg LUEX_NIF_LUA_LIB [] = {
    {"print", l_luex_print},
    {"send",  l_send},
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
    resource_data_t *rd;
    ErlNifPid self;
    enif_self(env, &self);

    ERL_NIF_TERM res;

    rd = enif_alloc_resource(resource_type, sizeof(resource_data_t));

    lua_state_t *L;
    L = luaL_newstate();

    lua_pushlightuserdata(L, (void*)env);
    lua_setglobal(L, "__ENV__");

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
