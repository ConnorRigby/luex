#ifndef LUEX_NIF_H
#define LUEX_NIF_H

#include <stdbool.h>

typedef lua_State lua_state_t;

#define HERETXT(...) #__VA_ARGS__

typedef struct ResourceData {
    lua_state_t *L;
    ErlNifPid self;
    sem_t *mailbox_ready;
    ERL_NIF_TERM const *mailbox;
} resource_data_t;

typedef struct PrivData {
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_undefined;
    ERL_NIF_TERM atom_error;
    ERL_NIF_TERM atom_nil;
    ERL_NIF_TERM atom_number;
    ERL_NIF_TERM atom_true;
    ERL_NIF_TERM atom_false;
    ERL_NIF_TERM atom_unknown_type;
    ERL_NIF_TERM atom_k_struct;
    ERL_NIF_TERM atom_k_name;
    ERL_NIF_TERM atom_k_type;
    ERL_NIF_TERM atom_k_ptr;
    ERL_NIF_TERM atom_struct_name;
} priv_data_t;

static ERL_NIF_TERM luex_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM luex_register_function(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM luex_into_mailbox(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM luex_dostring(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM luex_dofile(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM convert_to_term(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int stack_index);
static ERL_NIF_TERM convert_ptr_to_opaque_map(ErlNifEnv *env, priv_data_t* priv, const void* ptr, int type, const char* name);
static ERL_NIF_TERM lua_return_to_tuple(ErlNifEnv *env, priv_data_t* priv, lua_state_t* L, int nresults);

static void rt_dtor(ErlNifEnv *env, void *obj);
static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info);
static void unload(ErlNifEnv* env, void* priv);

static ErlNifResourceType *resource_type;

static ErlNifFunc nif_funcs[] = {
    {"new", 0, luex_init},
    {"register_function", 3, luex_register_function},
    {"into_mailbox", 2, luex_into_mailbox},
    {"dostring", 2, luex_dostring, ERL_NIF_DIRTY_JOB_CPU_BOUND},
    {"dofile", 2, luex_dofile, ERL_NIF_DIRTY_JOB_CPU_BOUND}
};

ERL_NIF_INIT(Elixir.Luex, nif_funcs, &load, &reload, &upgrade, &unload)

#endif
