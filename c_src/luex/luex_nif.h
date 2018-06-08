#ifndef LUEX_NIF_H
#define LUEX_NIF_H

typedef struct ResourceData {
    void* data;
} resource_data_t;

typedef struct PrivData {
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_undefined;
    ERL_NIF_TERM atom_error;
    ERL_NIF_TERM atom_nil;
    ERL_NIF_TERM atom_number;
} priv_data_t;

static ERL_NIF_TERM luex_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static void rt_dtor(ErlNifEnv *env, void *obj);
static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info);
static void unload(ErlNifEnv* env, void* priv);

static ErlNifResourceType *resource_type;

static ErlNifFunc nif_funcs[] = {
    {"init", 0, luex_init}
};

ERL_NIF_INIT(Elixir.Luex, nif_funcs, &load, &reload, &upgrade, &unload)

#endif
