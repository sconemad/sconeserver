/* SconeServer (http://www.sconemad.com)

OpenSSL compatibility utils

Copyright (c) 2000-2018 Andrew Wedgbury <wedge@sconemad.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see the file COPYING); if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA */

#include <openssl/opensslv.h>

// Pre openssl-1.1 compatibility
#if OPENSSL_VERSION_NUMBER < 0x10100000L

#include <openssl/crypto.h>
#include <openssl/ssl.h>

static pthread_mutex_t lock_cs[CRYPTO_NUM_LOCKS];
static long lock_count[CRYPTO_NUM_LOCKS];

unsigned long pthreads_thread_id()
{ return (unsigned long)pthread_self(); }

void pthreads_locking_callback(int mode, int type, const char* file, int line)
{
  if (mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&(lock_cs[type]));
    lock_count[type]++;
  } else {
    pthread_mutex_unlock(&(lock_cs[type]));
  }
}

void init_openssl_threading(void)
{
  int i;
  for (i=0; i<CRYPTO_NUM_LOCKS; i++) {
    lock_count[i]=0;
    pthread_mutex_init(&(lock_cs[i]),NULL);
  }

  CRYPTO_set_id_callback((unsigned long (*)())pthreads_thread_id);
  CRYPTO_set_locking_callback(pthreads_locking_callback);
}

void cleanup_openssl_threading(void)
{
  int i;
  CRYPTO_set_locking_callback(NULL);
  for (i=0; i<CRYPTO_NUM_LOCKS; i++) {
    pthread_mutex_destroy(&(lock_cs[i]));
  }
}

static void *OPENSSL_zalloc(size_t num)
{ 
  void *ret = OPENSSL_malloc(num);
  if (ret != NULL) memset(ret, 0, num);
  return ret;
}

EVP_MD_CTX *EVP_MD_CTX_new(void) 
{ 
  return (EVP_MD_CTX*)OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
  EVP_MD_CTX_cleanup(ctx);
  OPENSSL_free(ctx);
}

BIO_METHOD *BIO_meth_new(int type, const char *name)
{
  BIO_METHOD *biom = OPENSSL_zalloc(sizeof(BIO_METHOD));
  if (biom == NULL
      || (biom->name = OPENSSL_strdup(name)) == NULL) {
    OPENSSL_free(biom);
    return NULL;
  }
  biom->type = type;
  return biom;
}

int BIO_meth_set_write(BIO_METHOD *biom,
                       int (*write) (BIO *, const char *, int))
{ biom->bwrite = write; }

int BIO_meth_set_read(BIO_METHOD *biom,
                      int (*read) (BIO *, char *, int))
{ biom->bread = read; }

int BIO_meth_set_puts(BIO_METHOD *biom,
                      int (*puts) (BIO *, const char *))
{ biom->bputs = puts; }

int BIO_meth_set_gets(BIO_METHOD *biom,
                      int (*gets) (BIO *, char *, int))
{ biom->bgets = gets; }

int BIO_meth_set_ctrl(BIO_METHOD *biom,
                      long (*ctrl) (BIO *, int, long, void *))
{ biom->ctrl = ctrl; }

int BIO_meth_set_create(BIO_METHOD *biom, int (*create) (BIO *))
{ biom->create = create; }

int BIO_meth_set_destroy(BIO_METHOD *biom, int (*destroy) (BIO *))
{ biom->destroy = destroy; }

void BIO_set_data(BIO *a, void *ptr)
{ a->ptr = ptr; }

void *BIO_get_data(BIO *a)
{ return a->ptr; }

void BIO_set_init(BIO *a, int init)
{ a->init = init; }

int BIO_get_init(BIO *a)
{ return a->init; }

void BIO_set_shutdown(BIO *a, int shut)
{ a->shutdown = shut; }

int BIO_get_shutdown(BIO *a)
{ return a->shutdown; }

#else

// OpenSSL>=1.1 handles threading itself
void init_openssl_threading(void) {}
void cleanup_openssl_threading(void) {}

#endif

