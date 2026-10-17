/*
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
*/

#include "../../../config/config.h"

#ifdef USE_BZIP2
#include <bzlib.h>
#endif

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <string.h>

/* Bzip2 interface code */

#ifdef USE_BZIP2
/* Keep the native stream outside the moving OCaml heap. */
typedef struct {
  bz_stream *stream;
  int compress;
} bz_handle;

#define BZHandle_val(v) ((bz_handle *) Data_custom_val(v))

static bz_stream *bzstream_val(value v)
{
  bz_stream *stream = BZHandle_val(v)->stream;
  if (stream == NULL) caml_invalid_argument("Bzip2: stream is closed");
  return stream;
}

#define BZStream_val(v) bzstream_val(v)

static void finalize_bzstream(value v)
{
  bz_handle *handle = BZHandle_val(v);
  if (handle->stream != NULL) {
    if (handle->compress)
      BZ2_bzCompressEnd(handle->stream);
    else
      BZ2_bzDecompressEnd(handle->stream);
    caml_stat_free(handle->stream);
    handle->stream = NULL;
  }
}

static struct custom_operations bzstream_ops = {
  .identifier = "mldonkey.bzip2.stream",
  .finalize = finalize_bzstream,
  .compare = custom_compare_default,
  .hash = custom_hash_default,
  .serialize = custom_serialize_default,
  .deserialize = custom_deserialize_default,
  .compare_ext = custom_compare_ext_default,
  .fixed_length = custom_fixed_length_default
};
#endif

static const value * camlzip_bzerror_exn = NULL;

#ifdef USE_BZIP2
static void camlzip_bzerror(char * fn, int err)
{
  char * msg;
  value s1 = Val_unit, s2 = Val_unit, bucket = Val_unit;

  if (camlzip_bzerror_exn == NULL) {
    camlzip_bzerror_exn = caml_named_value("Bzlib.Error");
    if (camlzip_bzerror_exn == NULL)
      caml_invalid_argument("Exception Bzlib.Error not initialized");
  }
  Begin_roots3(s1, s2, bucket);
    s1 = caml_copy_string(fn);
    switch (err) {
    case BZ_CONFIG_ERROR:
      s2 = Val_int(0);
      break;
    case BZ_SEQUENCE_ERROR:
      s2 = Val_int(1);
      break;
    case BZ_PARAM_ERROR:
      s2 = Val_int(2);
      break;
    case BZ_MEM_ERROR:
      s2 = Val_int(3);
      break;
    case BZ_DATA_ERROR:
      s2 = Val_int(4);
      break;
    case BZ_DATA_ERROR_MAGIC:
      s2 = Val_int(5);
      break;
    default:
      s2 = Val_int(6);
    }
    bucket = caml_alloc_small(3, 0);
    Field(bucket, 0) = *camlzip_bzerror_exn;
    Field(bucket, 1) = s1;
    Field(bucket, 2) = s2;
  End_roots();
  caml_raise(bucket);
}

static value camlzip_new_bzstream(int compress)
{
  CAMLparam0();
  CAMLlocal1(res);
  res = caml_alloc_custom(&bzstream_ops, sizeof(bz_handle), sizeof(bz_stream), 1);
  BZHandle_val(res)->stream = NULL;
  BZHandle_val(res)->compress = compress;
  bz_stream *stream = caml_stat_alloc(sizeof(bz_stream));
  memset(stream, 0, sizeof(bz_stream));
  BZHandle_val(res)->stream = stream;
  CAMLreturn(res);
}

int camlzip_action_table[] = { BZ_RUN, BZ_FLUSH, BZ_FINISH };
#endif


value camlzip_bzCompressInit(value blockSize100k, value verbosity, value workFactor) {
#ifdef USE_BZIP2
  int err;
  CAMLparam3(blockSize100k, verbosity, workFactor);
  CAMLlocal1(vbzs);
  vbzs = camlzip_new_bzstream(1);
  if ((err = BZ2_bzCompressInit(BZStream_val(vbzs),
			 Int_val(blockSize100k),
			 Int_val(verbosity),
			 Int_val(workFactor))) != BZ_OK)
    camlzip_bzerror("Zlib.deflateInit", err);
  CAMLreturn(vbzs);
#else
  caml_failwith("Bzip2 compression not supported.");
#endif
}

value camlzip_bzCompress(value vzs, value srcbuf, value srcpos, value srclen,
                      value dstbuf, value dstpos, value dstlen,
                      value vflush)
{
#ifdef USE_BZIP2
  bz_stream * zs = BZStream_val(vzs);
  int retcode;
  long used_in, used_out;
  value res;

  zs->next_in = &Byte(srcbuf, Long_val(srcpos));
  zs->avail_in = Long_val(srclen);
  zs->next_out = &Byte(dstbuf, Long_val(dstpos));
  zs->avail_out = Long_val(dstlen);
  retcode = BZ2_bzCompress(zs, camlzip_action_table[Int_val(vflush)]);
  if (retcode < 0) camlzip_bzerror("Bzlib.compress", retcode);
  used_in = Long_val(srclen) - zs->avail_in;
  used_out = Long_val(dstlen) - zs->avail_out;
  zs->next_in = NULL;         /* not required, but cleaner */
  zs->next_out = NULL;        /* (avoid dangling pointers into Caml heap) */
  res = caml_alloc_small(3, 0);
  Field(res, 0) = Val_bool(retcode == BZ_STREAM_END);
  Field(res, 1) = Val_int(used_in);
  Field(res, 2) = Val_int(used_out);
  return res;
#else
  caml_failwith("Bzip2 compression not supported");
#endif
}

value camlzip_bzCompress_bytecode(value * arg, int nargs)
{
  return camlzip_bzCompress(arg[0], arg[1], arg[2], arg[3],
                         arg[4], arg[5], arg[6], arg[7]);
}

value camlzip_bzCompressEnd(value stream) {
#ifdef USE_BZIP2
  int err;
  if ((err = BZ2_bzCompressEnd(BZStream_val(stream))) != BZ_OK)
    camlzip_bzerror("Bzlib.compress_end", err);
  caml_stat_free(BZHandle_val(stream)->stream);
  BZHandle_val(stream)->stream = NULL;
#else
  caml_failwith("Bzip2 compression not supported");
#endif
  return Val_unit;
}

value camlzip_bzDecompressInit(value verbosity, value small)
{
#ifdef USE_BZIP2
  int err;
  CAMLparam2(verbosity, small);
  CAMLlocal1(vzs);
  vzs = camlzip_new_bzstream(0);
  if ((err = BZ2_bzDecompressInit(BZStream_val(vzs), Int_val(verbosity), Bool_val(small))) != BZ_OK)
    camlzip_bzerror("Bzlib.decompress_init", err);
  CAMLreturn(vzs);
#else
  caml_failwith("Bzip2 compression not supported");
#endif
}

value camlzip_bzDecompress(value vzs, value srcbuf, value srcpos, value srclen,
			   value dstbuf, value dstpos, value dstlen)
{
#ifdef USE_BZIP2
  bz_stream * zs = BZStream_val(vzs);
  int retcode;
  long used_in, used_out;
  value res;

  zs->next_in = &Byte(srcbuf, Long_val(srcpos));
  zs->avail_in = Long_val(srclen);
  zs->next_out = &Byte(dstbuf, Long_val(dstpos));
  zs->avail_out = Long_val(dstlen);
  retcode = BZ2_bzDecompress(zs);
  if (retcode < 0)
    camlzip_bzerror("Bzlib.decompress", retcode);
  used_in = Long_val(srclen) - zs->avail_in;
  used_out = Long_val(dstlen) - zs->avail_out;
  zs->next_in = NULL;           /* not required, but cleaner */
  zs->next_out = NULL;          /* (avoid dangling pointers into Caml heap) */
  res = caml_alloc_small(3, 0);
  Field(res, 0) = Val_bool(retcode == BZ_STREAM_END);
  Field(res, 1) = Val_int(used_in);
  Field(res, 2) = Val_int(used_out);
  return res;
#else
  caml_failwith("Bzip2 compression not supported");
#endif
}

value camlzip_bzDecompress_bytecode(value * arg, int nargs)
{
  return camlzip_bzDecompress(arg[0], arg[1], arg[2], arg[3],
                         arg[4], arg[5], arg[6]);
}

value camlzip_bzDecompressEnd(value stream) {
#ifdef USE_BZIP2
  int err;
  if ((err = BZ2_bzDecompressEnd(BZStream_val(stream))) != BZ_OK)
    camlzip_bzerror("Bzlib.decompressEnd", err);
  caml_stat_free(BZHandle_val(stream)->stream);
  BZHandle_val(stream)->stream = NULL;
#else
  caml_failwith("Bzip2 compression not supported");
#endif
  return Val_unit;
}

int camlzip_bzlibversion(void)
{
  CAMLparam0 ();
  CAMLlocal1 (v);
#ifdef HAVE_BZLIBVERSION
  v = caml_copy_string (BZ2_bzlibVersion());
  CAMLreturn (v);
#else
  caml_failwith("bzlibVersion not found");
#endif
}
