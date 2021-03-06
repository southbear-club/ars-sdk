/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file ssl.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/net/ssl.hpp"
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

namespace ars {

namespace sdk {

static ssl_ctx_t s_ssl_ctx = NULL;

const char* ssl_backend() {
#ifdef WITH_OPENSSL
    return "openssl";
#elif defined(WITH_MBEDTLS)
    return "mbedtls";
#else
    return "null";
#endif
}

ssl_ctx_t ssl_ctx_instance() {
    if (s_ssl_ctx == NULL) {
        s_ssl_ctx = ssl_ctx_init(NULL);
    }
    return s_ssl_ctx;
}

#ifdef WITH_OPENSSL

#include <openssl/ssl.h>
#include <openssl/err.h>

ssl_ctx_t ssl_ctx_init(ssl_ctx_init_param_t* param) {
    static int s_initialized = 0;
    if (s_initialized == 0) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSL_library_init();
        SSL_load_error_strings();
#else
        OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
#endif
        s_initialized = 1;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
#else
    SSL_CTX* ctx = SSL_CTX_new(TLS_method());
#endif
    if (ctx == NULL) return NULL;
    int mode = SSL_VERIFY_NONE;
    if (param) {
        if (param->ca_file && *param->ca_file) {
            if (!SSL_CTX_load_verify_locations(ctx, param->ca_file, NULL)) {
                fprintf(stderr, "ssl ca_file verify failed!\n");
                goto error;
            }
        }
        if (param->crt_file && *param->crt_file) {
            if (!SSL_CTX_use_certificate_file(ctx, param->crt_file, SSL_FILETYPE_PEM)) {
                fprintf(stderr, "ssl crt_file error!\n");
                goto error;
            }
        }
        if (param->key_file && *param->key_file) {
            if (!SSL_CTX_use_PrivateKey_file(ctx, param->key_file, SSL_FILETYPE_PEM)) {
                fprintf(stderr, "ssl key_file error!\n");
                goto error;
            }
            if (!SSL_CTX_check_private_key(ctx)) {
                fprintf(stderr, "ssl key_file check failed!\n");
                goto error;
            }

        }
        if (param->verify_peer) {
            mode = SSL_VERIFY_PEER;
        }
    }
    SSL_CTX_set_verify(ctx, mode, NULL);
    s_ssl_ctx = ctx;
    return ctx;
error:
    SSL_CTX_free(ctx);
    return NULL;
}

void ssl_ctx_cleanup(ssl_ctx_t ssl_ctx) {
    if (ssl_ctx) {
        if (ssl_ctx == s_ssl_ctx) {
            s_ssl_ctx = NULL;
        }
        SSL_CTX_free((SSL_CTX*)ssl_ctx);
        ssl_ctx = NULL;
    }
}

ssl_t ssl_new(ssl_ctx_t ssl_ctx, int fd) {
    SSL* ssl = SSL_new((SSL_CTX*)ssl_ctx);
    if (ssl == NULL) return NULL;
    SSL_set_fd(ssl, fd);
    return ssl;
}

void ssl_free(ssl_t ssl) {
    if (ssl) {
        SSL_free((SSL*)ssl);
        ssl = NULL;
    }
}

int ssl_accept(ssl_t ssl) {
    int ret = SSL_accept((SSL*)ssl);
    if (ret == 1) return 0;

    int err = SSL_get_error((SSL*)ssl, ret);
    if (err == SSL_ERROR_WANT_READ) {
        return SSL_WANT_READ;
    }
    else if (err == SSL_ERROR_WANT_WRITE) {
        return SSL_WANT_WRITE;
    }
    return err;
}

int ssl_connect(ssl_t ssl) {
    int ret = SSL_connect((SSL*)ssl);
    if (ret == 1) return 0;

    int err = SSL_get_error((SSL*)ssl, ret);
    if (err == SSL_ERROR_WANT_READ) {
        return SSL_WANT_READ;
    }
    else if (err == SSL_ERROR_WANT_WRITE) {
        return SSL_WANT_WRITE;
    }
    return err;
}

int ssl_read(ssl_t ssl, void* buf, int len) {
    return SSL_read((SSL*)ssl, buf, len);
}

int ssl_write(ssl_t ssl, const void* buf, int len) {
    return SSL_write((SSL*)ssl, buf, len);
}

int ssl_close(ssl_t ssl) {
    SSL_shutdown((SSL*)ssl);
    return 0;
}

#elif defined(WITH_MBEDTLS)


#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

struct mbedtls_ctx {
    mbedtls_entropy_context     entropy;
    mbedtls_ctr_drbg_context    ctr_drbg;
    mbedtls_ssl_config          conf;
    mbedtls_x509_crt            cert;
    mbedtls_pk_context          pkey;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context   cache;
#endif
};

ssl_ctx_t ssl_ctx_init(ssl_ctx_init_param_t* param) {
    struct mbedtls_ctx* ctx = (struct mbedtls_ctx*)malloc(sizeof(struct mbedtls_ctx));
    if (ctx == NULL) return NULL;

    mbedtls_ssl_config_init(&ctx->conf);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init(&ctx->cache);
#endif
    mbedtls_x509_crt_init(&ctx->cert);
    mbedtls_pk_init(&ctx->pkey);
    mbedtls_entropy_init(&ctx->entropy);
    mbedtls_ctr_drbg_init(&ctx->ctr_drbg);

    int mode = MBEDTLS_SSL_VERIFY_NONE;
    int endpoint = MBEDTLS_SSL_IS_CLIENT;
    bool check = false;
    if (param) {
        if (param->crt_file && *param->crt_file) {
            if (mbedtls_x509_crt_parse_file(&ctx->cert, param->crt_file) != 0) {
                fprintf(stderr, "ssl crt_file error!\n");
                goto error;
            }
        }
        if (param->key_file && *param->key_file) {
            if (mbedtls_pk_parse_keyfile(&ctx->pkey, param->key_file, NULL) != 0) {
                fprintf(stderr, "ssl key_file error!\n");
                goto error;
            }
            check = true;
        }
        if (param->verify_peer) {
            mode = MBEDTLS_SSL_VERIFY_REQUIRED;
        }
        if (param->endpoint == 0) {
            endpoint = MBEDTLS_SSL_IS_SERVER;
        }
    }
    mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy, NULL, 0);
    if (mbedtls_ssl_config_defaults(&ctx->conf, endpoint,
        MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        fprintf(stderr, "ssl config error!\n");
        goto error;
    }
    mbedtls_ssl_conf_authmode(&ctx->conf, mode);
    mbedtls_ssl_conf_rng(&ctx->conf, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&ctx->conf, &ctx->cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

    if (check) {
        mbedtls_ssl_conf_ca_chain(&ctx->conf, ctx->cert.next, NULL);
        if (mbedtls_ssl_conf_own_cert(&ctx->conf, &ctx->cert, &ctx->pkey) != 0) {
            fprintf(stderr, "ssl key_file check failed!\n");
            goto error;
        }
    }

    s_ssl_ctx = ctx;
    return ctx;
error:
    free(ctx);
    return NULL;
}

void ssl_ctx_cleanup(ssl_ctx_t ssl_ctx) {
    if (!ssl_ctx) return;
    if (ssl_ctx == s_ssl_ctx) {
        s_ssl_ctx = NULL;
    }
    struct mbedtls_ctx *mctx = (struct mbedtls_ctx *)ssl_ctx;
    mbedtls_x509_crt_free(&mctx->cert);
    mbedtls_pk_free(&mctx->pkey);
    mbedtls_ssl_config_free(&mctx->conf);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&mctx->cache);
#endif
    mbedtls_ctr_drbg_free(&mctx->ctr_drbg);
    mbedtls_entropy_free(&mctx->entropy);
    free(mctx);
}

static int __mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len) {
    int fd = (intptr_t)ctx;
    int n = write(fd, buf, len);
    if (n >= 0) return n;
    return ((errno == EAGAIN || errno == EINPROGRESS) ? MBEDTLS_ERR_SSL_WANT_WRITE : -1);
}

static int __mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len) {
    int fd = (intptr_t)ctx;
    int n = read(fd, buf, len);
    if (n >= 0) return n;
    return ((errno == EAGAIN || errno == EINPROGRESS) ? MBEDTLS_ERR_SSL_WANT_READ : -1);
}

ssl_t ssl_new(ssl_ctx_t ssl_ctx, int fd) {
    struct mbedtls_ctx* mctx = (struct mbedtls_ctx*)ssl_ctx;
    mbedtls_ssl_context* ssl = (mbedtls_ssl_context*)malloc(sizeof(mbedtls_ssl_context));
    if (ssl == NULL) return NULL;
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_setup(ssl, &mctx->conf);
    mbedtls_ssl_set_bio(ssl, (void*)(intptr_t)fd, __mbedtls_net_send, __mbedtls_net_recv, NULL);
    return ssl;
}

void ssl_free(ssl_t ssl) {
    if (ssl) {
        mbedtls_ssl_free(ssl);
        ssl = NULL;
    }
}

static int hssl_handshake(ssl_t ssl) {
    int ret = mbedtls_ssl_handshake(ssl);
    if (ret != 0) {
        if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
            return SSL_WANT_READ;
        }
        else if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            return SSL_WANT_WRITE;
        }
    }
    return ret;
}

int ssl_accept(ssl_t ssl) {
    return hssl_handshake(ssl);
}

int ssl_connect(ssl_t ssl) {
    return hssl_handshake(ssl);
}

int ssl_read(ssl_t ssl, void* buf, int len) {
    return mbedtls_ssl_read(ssl, buf, len);
}

int ssl_write(ssl_t ssl, const void* buf, int len) {
    return mbedtls_ssl_write(ssl, buf, len);
}

int ssl_close(ssl_t ssl) {
    return 0;
}

#else

ssl_ctx_t ssl_ctx_init(ssl_ctx_init_param_t* param) {
    fprintf(stderr, "Please recompile WITH_SSL.\n");
    return NULL;
}

void ssl_ctx_cleanup(ssl_ctx_t ssl_ctx) {
}

ssl_t ssl_new(ssl_ctx_t ssl_ctx, int fd) {
    return (void*)(intptr_t)fd;
}

void ssl_free(ssl_t ssl) {
}

int ssl_accept(ssl_t ssl) {
    return 0;
}

int ssl_connect(ssl_t ssl) {
    return 0;
}

int ssl_read(ssl_t ssl, void* buf, int len) {
    int fd = (intptr_t)ssl;
    return read(fd, buf, len);
}

int ssl_write(ssl_t ssl, const void* buf, int len) {
    int fd = (intptr_t)ssl;
    return write(fd, buf, len);
}

int ssl_close(ssl_t ssl) {
    return 0;
}
#endif

} // namespace sdk

} // namespace ars
