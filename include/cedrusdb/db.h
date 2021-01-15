#ifndef _CEDRUS_DB_H
#define _CEDRUS_DB_H

#include <stdint.h>
#include <stddef.h>

typedef struct CedrusIter CedrusIter;
typedef struct CedrusConfig CedrusConfig;
typedef enum HashTrieKeyMode {
    HASHED_KEY = 0x0,
    USER_KEY = 0x1
} HashTrieKeyMode;

typedef struct Cedrus Cedrus;
typedef struct CedrusValueRef CedrusValueRef;
typedef struct CedrusValueMut CedrusValueMut;

typedef struct CedrusValueInfo {
    const uint8_t *base;
    size_t size;
} CedrusValueInfo;

typedef struct CedrusKeyValue {
    const uint8_t *key;
    size_t key_size;
    const uint8_t *val;
    size_t val_size;
    HashTrieKeyMode mode;
} CedrusKeyValue;

typedef struct CedrusWriteBatch CedrusWriteBatch;

typedef struct CedrusConfig {
    uint64_t space_file_nbit;
    uint64_t space_regn_nbit;
    uint64_t freed_file_nbit;
    uint64_t freed_regn_nbit;
    uint8_t wal_block_nbit;
    uint8_t wal_file_nbit;
    uint64_t max_regn_id;
    size_t node_space_max_cached_regn;
    size_t node_freed_max_cached_regn;
    int node_swap_on;
    size_t data_blk_space_max_cached_regn;
    size_t data_blk_freed_max_cached_regn;
    int data_blk_swap_on;
    size_t data_comp_space_max_cached_regn;
    size_t data_comp_freed_max_cached_regn;
    int data_comp_swap_on;
    uint64_t data_comp_max_walk;
    uint8_t sluggishness;
    size_t max_buffered;
    size_t max_staging;
    size_t max_sealed;
    int32_t max_aio_requests;
    uint32_t max_aio_submit;
    uint32_t max_aio_responses;
    int32_t max_wal_aio_requests;
    size_t max_wal_queued;
    size_t max_wal_growth;
    size_t emulated_failure_point; /** only useful when emulated-failure feature is enabled */
} CedrusConfig;

/** NOTE: for a more detailed documentation, please refer to the `binding`
 * section of the Rust doc where most functions are documented. */

/** Create a config object to be used in `cedrus_new`. */
CedrusConfig cedrus_config_default();

/** Create/open a Cedrus database, config pointer will be freed after this call. */
Cedrus *cedrus_new(const char *db_name, const CedrusConfig *config, int truncate);
void cedrus_free(Cedrus *cedrus);

/* The following functions implements the basic functionalities for a key-value
 * store. Both key and value could be arbitrary sized binary data. */
int cedrus_put(const Cedrus *cedrus, const uint8_t *key, size_t key_size,
                                const uint8_t *val, size_t val_size);
int cedrus_get(const Cedrus *cedrus, const uint8_t *key, size_t key_size, CedrusValueRef **vr);
int cedrus_get_mut(const Cedrus *cedrus, const uint8_t *key, size_t key_size, CedrusValueMut **vm);
int cedrus_delete(const Cedrus *cedrus, const uint8_t *key, size_t key_size);
/* */

/* The following functions directly use key to index the item, bypassing further
 * hashing. This assumes the provided key points to a 32-byte memory space. The
 * statistical property of the key affects the performance and ideally it should
 * be pseudo-randomized. */
int cedrus_put_by_hash(const Cedrus *cedrus, const uint8_t *key, const uint8_t *val, size_t val_size);
int cedrus_get_by_hash(const Cedrus *cedrus, const uint8_t *key, CedrusValueRef **vr);
int cedrus_get_by_hash_mut(const Cedrus *cedrus, const uint8_t *key, CedrusValueMut **vm);
int cedrus_delete_by_hash(const Cedrus *cedrus, const uint8_t *key);
/* */

/* Helper functions */
int cedrus_modify(const Cedrus *db, CedrusValueMut *vm, void (*f)(uint8_t *base, size_t size, void *arg), void *f_arg);
int cedrus_update(const Cedrus *db, CedrusValueMut *vm, const uint8_t *val, size_t val_size);
CedrusValueInfo cedrus_vr_info(const CedrusValueRef *vr);
CedrusValueInfo cedrus_vm_info(const CedrusValueMut *vr);
void cedrus_vr_free(CedrusValueRef *vr);
void cedrus_vm_free(CedrusValueMut *vm);
void cedrus_kvh_free(CedrusKeyValue *kvh);

/* Write batch related */
CedrusWriteBatch *cedrus_writebatch_new(const Cedrus *cedrus);
int cedrus_writebatch_put_by_hash(CedrusWriteBatch *batch, const uint8_t *key,
                                                        const uint8_t *val, size_t val_size);
int cedrus_writebatch_delete_by_hash(CedrusWriteBatch *batch, const uint8_t *key);
int cedrus_writebatch_put(CedrusWriteBatch *batch, const uint8_t *key, size_t key_size,
                                                    const uint8_t *val, size_t val_size);
int cedrus_writebatch_delete(CedrusWriteBatch *batch, const uint8_t *key, size_t key_size);
int cedrus_writebatch_write(CedrusWriteBatch *batch);
void cedrus_writebatch_drop(CedrusWriteBatch *batch);
/* */

void cedrus_update_profile(const Cedrus *cedrus);
void cedrus_reset_profile(const Cedrus *cedrus);
void cedrus_print_profile(const Cedrus *cedrus);

/* NOTE: the functions below this line are not NOT thread-safe */

int cedrus_dump(const Cedrus *cedrus);
CedrusIter *cedrus_new_iter(const Cedrus *cedrus);
void cedrus_iter_free(CedrusIter *iter);
int cedrus_iter_next(CedrusIter *iter, CedrusKeyValue **kvh);
int cedrus_check_integrity(Cedrus *cedrus);

#endif
