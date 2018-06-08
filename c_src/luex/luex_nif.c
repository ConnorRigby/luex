#include <erl_nif.h>
#include <stdio.h>

#include "luex_nif.h"

static void rt_dtor(ErlNifEnv *env, void *obj) {
    resource_data_t *rd = (resource_data_t *)obj;
    enif_fprintf(stderr, "rt_dtor called\r\n");
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
    return load(env, priv, info);
}
static void unload(ErlNifEnv* env, void* priv) {
    enif_free(priv);
}

static ERL_NIF_TERM luex_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    priv_data_t* priv = enif_priv_data(env);
    resource_data_t *rd;

    ERL_NIF_TERM res;

    rd = enif_alloc_resource(resource_type, sizeof(resource_data_t));
    rd->data = (void*)priv;

    res = enif_make_resource(env, rd);
    enif_release_resource(rd);

    return enif_make_tuple2(env, priv->atom_ok, res);
}
