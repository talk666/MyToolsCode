#ifndef _CAPABILITES_H_
#define _CAPABILITES_H_

#include <stdint.h>

typedef struct cipher_bits_s{
    uint32_t ecb        :1;
    uint32_t cbc        :1;
    uint32_t gcm        :1;
    uint32_t cfb        :1;
    uint32_t ctr        :1;
    uint32_t ofb        :1;
    uint32_t xts        :1;
    uint32_t reserve1   :1;

    uint32_t kek        :1;
    uint32_t key_index  :1;
    uint32_t gcm_update :1;
    uint32_t gcm_aad    :1;
    uint32_t reserve3   :4;

    uint32_t reserve2   :8;

    //chain default CBC-chain
    uint32_t sha1       :1;
    uint32_t sha256     :1;
    uint32_t sha384     :1;
    uint32_t sha512     :1;
    uint32_t sm3        :1;
    uint32_t reserve4   :3;

}cipher_bits_t;

struct sym_cap_s
{
    struct
    {
        union{
            uint32_t data;
            cipher_bits_t bits;
        }u;
    }aes128;

    struct
    {
        union{
            uint32_t data;
            cipher_bits_t bits;
        }u;
    }aes256;

    struct
    {
        union{
            uint32_t data;
            cipher_bits_t bits;
        }u;
    }sm4;

    struct
    {
        union{
            uint32_t data;
            cipher_bits_t bits;
        }u;
    }sm1;
};

typedef struct ecc_bits_s{
    uint32_t sign        :1;
    uint32_t verify      :1;
    uint32_t kg          :1;
    uint32_t kp          :1;
    uint32_t enc         :1;
    uint32_t dec         :1;
    uint32_t reserve1    :2;
    uint32_t kek         :1;
    uint32_t key_index   :1;
    uint32_t reserve2    :22;
}ecc_bits_t;

typedef struct rsa_bits_s{
    uint32_t pub_modexp  :1;
    uint32_t priv_modexp :1;
    uint32_t pub_crt     :1;
    uint32_t priv_crt    :1;
    uint32_t reserve1    :4;
    uint32_t kek         :1;
    uint32_t key_index   :1;
    uint32_t reserve2    :22;
}rsa_bits_t;

struct asym_cap_s
{
    struct
    {
        union{
            uint32_t data;
            rsa_bits_t bits;
        }bits_1024;
        union{
            uint32_t data;
            rsa_bits_t bits;
        }bits_2048;
    }rsa;
    struct
    {
        union{
            uint32_t data;
            ecc_bits_t bits;
        }curve_256r1;
        union{
            uint32_t data;
            ecc_bits_t bits;
        }curve_25519;
        union{
            uint32_t data;
            ecc_bits_t bits;
        }curve_sm2;
    }ecc;
};

typedef struct digest_bits_s{
    uint32_t sm3          :1;
    uint32_t sha1         :1;
    uint32_t sha224       :1;
    uint32_t sha256       :1;
    uint32_t sha384       :1;
    uint32_t sha512       :1;
    uint32_t md5          :1;
    uint32_t reserve1     :1;
    uint32_t kek          :1;
    uint32_t key_index    :1;
    uint32_t reserve2     :22;
}digest_bits_t;

struct digest_cap_s{
    struct
    {
        union{
            uint32_t data;
            digest_bits_t bits;
        }u;
    }hash;

    struct
    {
        union{
            uint32_t data;
            digest_bits_t bits;
        }u;
    }hmac;

    struct
    {
        union{
            uint32_t data;
            digest_bits_t bits;
        }u;
    }prf;
};

typedef struct random_bits_s{
    uint32_t random       :1;
    uint32_t reserve1     :7;
    uint32_t kek          :1;
    uint32_t key_index    :1;
    uint32_t reserve2     :22;
}random_bits_t;

struct random_cap_s
{
    union{
        uint32_t data;
        random_bits_t bits;
    }u;
};

struct capabilities_s
{
    struct sym_cap_s    sym;
    struct asym_cap_s   asym;
    struct digest_cap_s digest;
    struct random_cap_s random;
    uint32_t model;
};

#endif//_CAPABILITES_H_
