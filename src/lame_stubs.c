/*
 * OCaml bindings for liblame
 *
 * Copyright 2005-2006 Savonet team
 *
 * This file is part of ocaml-lame.
 *
 * ocaml-lame is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ocaml-lame is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ocaml-lame; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id$ */

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/misc.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <lame/lame.h>

#if !defined(LITTLE_ENDIAN) && defined(__LITTLE_ENDIAN)
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#if !defined(BIG_ENDIAN) && defined(__BIG_ENDIAN)
#define BIG_ENDIAN __BIG_ENDIAN
#endif

#define Lame_val(v) (*(lame_global_flags**)Data_custom_val(v))

static inline int16_t bswap_16 (int16_t x) { return ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)); }

static void finalize_lame_t(value l)
{
  lame_global_flags *lgf = Lame_val(l);
  lame_close(lgf);
}

static struct custom_operations lame_ops =
{
  "ocaml_lame_t",
  finalize_lame_t,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value ocaml_lame_init(value unit)
{
  CAMLparam1(unit);
  CAMLlocal1(l);

  lame_global_flags *lgf;
  lgf = lame_init();
#ifdef DEBUG
  printf("New encoder: %p.\n", lgf);
#endif
  l = caml_alloc_custom(&lame_ops, sizeof(lame_global_flags*), 0, 1);
  Lame_val(l) = lgf;

  CAMLreturn(l);
}

/***** Parameters *****/

#define BIND_SET_INT_PARAM(p) \
CAMLprim value ocaml_lame_set_##p(value l, value v) \
{ \
    CAMLparam2(l, v); \
    lame_set_##p(Lame_val(l), Int_val(v)); \
    CAMLreturn(Val_unit); \
}

#define BIND_GET_INT_PARAM(p) \
CAMLprim value ocaml_lame_get_##p(value l) \
{ \
    CAMLparam1(l); \
    CAMLreturn(Val_int(lame_get_##p(Lame_val(l)))); \
}

#define BIND_INT_PARAM(p) BIND_GET_INT_PARAM(p) BIND_SET_INT_PARAM(p)

BIND_INT_PARAM(num_samples);
BIND_INT_PARAM(in_samplerate);
BIND_INT_PARAM(num_channels);
BIND_INT_PARAM(out_samplerate);
BIND_INT_PARAM(quality);
BIND_INT_PARAM(mode);
BIND_INT_PARAM(brate);

#define BIND_FLOAT_PARAM(p) \
CAMLprim value ocaml_lame_set_##p(value l, value v) \
{ \
    CAMLparam2(l, v); \
    lame_set_##p(Lame_val(l), Double_val(v)); \
    CAMLreturn(Val_unit); \
} \
CAMLprim value ocaml_lame_get_##p(value l) \
{ \
    CAMLparam1(l); \
    CAMLreturn(caml_copy_double(lame_get_##p(Lame_val(l)))); \
}

BIND_FLOAT_PARAM(compression_ratio);

CAMLprim value ocaml_lame_init_params(value l)
{
  CAMLparam1(l);
  int ans = lame_init_params(Lame_val(l));
  if (ans < 0)
  {
    fprintf(stderr, "init_params error: %d\n", ans);
    caml_raise_constant(*caml_named_value("lame_exn_init_params_failed"));
  }
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_lame_init_bitstream(value l)
{
  CAMLparam1(l);
  lame_init_bitstream(Lame_val(l));
  CAMLreturn(Val_unit);
}

static vbr_mode vbr_mode_val[] = {vbr_off, vbr_rh, vbr_abr, vbr_mtrh, vbr_max_indicator};

CAMLprim value ocaml_lame_set_vbr(value l, value m)
{
  CAMLparam2(l, m);
  lame_set_VBR(Lame_val(l), vbr_mode_val[Int_val(m)]);
  CAMLreturn(Val_unit);
}

BIND_INT_PARAM(VBR_q);
BIND_INT_PARAM(VBR_mean_bitrate_kbps);
BIND_INT_PARAM(VBR_min_bitrate_kbps);
BIND_INT_PARAM(VBR_max_bitrate_kbps);
BIND_INT_PARAM(VBR_hard_min);

/****** Encoding ******/

static void raise_enc_err(int n)
{
  switch(n)
  {
    case -3:
      caml_raise_constant(*caml_named_value("lame_exn_init_params_not_called"));

    case -4:
      caml_raise_constant(*caml_named_value("lame_exn_psychoacoustic_problem"));

    default:
      /* TODO: be more precise. */
      caml_raise_with_arg(*caml_named_value("lame_exn_unknown_error"), Val_int(n));
  }
}

CAMLprim value ocaml_lame_encode_buffer_interleaved(value l, value _buf, value _ofs, value _samples)
{
  CAMLparam4(l, _buf, _ofs, _samples);
  CAMLlocal1(ret);
  lame_global_flags *lgf = Lame_val(l);
  int samples = Int_val(_samples);
  int inbuf_len = caml_string_length(_buf);
  int outbuf_len = 1.25 * samples + 7200;
  short int *inbuf = malloc(inbuf_len);
  unsigned char *outbuf = malloc(outbuf_len);
  int ans;

  memcpy(inbuf, String_val(_buf), inbuf_len);

  caml_enter_blocking_section();
#if BYTE_ORDER == LITTLE_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
  int i;
  for (i=0;i<inbuf_len/2;i++) \
    inbuf[i] = bswap_16(inbuf[i]);
#else
#error "Indians shall be either little or big, no other choice here."
#endif
  ans = lame_encode_buffer_interleaved(lgf, inbuf, samples, outbuf, outbuf_len);
  caml_leave_blocking_section();

  free(inbuf);

  if (ans < 0)
  {
    free(outbuf);
    raise_enc_err(ans);
  }
  ret = caml_alloc_string(ans);
  memcpy(String_val(ret), outbuf, ans);
  free(outbuf);

  CAMLreturn(ret);
}

/* Input: float arrays, floats in [-1..1]
 * Output: MP3 frames.
 * Lame wants floats in +/- SHRTMAX, we renormalize when copying to C heap.
 * TODO Figure out how to proceed when encoding mono data. */
CAMLprim value
ocaml_lame_encode_buffer_float(value l, value _bufl, value _bufr,
                               value _ofs, value _samples)
{
  CAMLparam5(l, _bufl, _bufr, _ofs, _samples);
  CAMLlocal1(ret);
  lame_global_flags *lgf = Lame_val(l);
  int ofs = Int_val(_ofs);
  int samples = Int_val(_samples);
  float *inbufl = malloc(sizeof(float)*samples);
  float *inbufr = malloc(sizeof(float)*samples);
  int outbuf_len = 1.25 * samples + 7200;
  unsigned char *outbuf = malloc(outbuf_len);
  int i,ans;

  for (i=0 ; i<samples ; i++) {
    inbufl[i] = 32768. * Double_field(_bufl,ofs+i) ;
    inbufr[i] = 32768. * Double_field(_bufr,ofs+i) ;
  }

  caml_enter_blocking_section();
  ans = lame_encode_buffer_float(lgf, inbufl, inbufr,
                                 samples, outbuf, outbuf_len);
  caml_leave_blocking_section();

  free(inbufl);
  free(inbufr);

  if (ans < 0)
  {
    free(outbuf);
    raise_enc_err(ans);
  }
  ret = caml_alloc_string(ans);
  memcpy(String_val(ret), outbuf, ans);
  free(outbuf);

  CAMLreturn(ret);
}

CAMLprim value ocaml_lame_encode_flush(value l)
{
  CAMLparam1(l);
  CAMLlocal1(ret);
  lame_global_flags *lgf = Lame_val(l);
  int ans;
  int outbuf_len = lame_get_size_mp3buffer(lgf) + lame_get_encoder_padding(lgf) + 7200;
  unsigned char *outbuf = malloc(outbuf_len);

  caml_enter_blocking_section();
  ans = lame_encode_flush(lgf, outbuf, outbuf_len);
  caml_leave_blocking_section();

  if (ans < 0)
  {
    free(outbuf);
    raise_enc_err(ans);
  }
  ret = caml_alloc_string(ans);
  memcpy(String_val(ret), outbuf, ans);
  free(outbuf);

  CAMLreturn(ret);
}

CAMLprim value ocaml_lame_encode_flush_nogap(value l)
{
  CAMLparam1(l);
  CAMLlocal1(ret);
  lame_global_flags *lgf = Lame_val(l);
  int ans;
  int outbuf_len = lame_get_size_mp3buffer(lgf) + 7200;
  unsigned char *outbuf = malloc(outbuf_len);

  caml_enter_blocking_section();
  ans = lame_encode_flush_nogap(lgf, outbuf, outbuf_len);
  caml_leave_blocking_section();

  if (ans < 0)
  {
    free(outbuf);
    raise_enc_err(ans);
  }
  ret = caml_alloc_string(ans);
  memcpy(String_val(ret), outbuf, ans);
  free(outbuf);

  CAMLreturn(ret);
}

/***** Id3 tags *****/

CAMLprim value ocaml_lame_id3tag_init(value l)
{
  CAMLparam1(l);

  id3tag_init(Lame_val(l));

  CAMLreturn(Val_unit);
}

#define BIND_TAG_PARAM(p) \
CAMLprim value ocaml_lame_id3tag_set_##p(value l, value v) \
{ \
    CAMLparam2(l, v); \
    id3tag_set_##p(Lame_val(l), String_val(v)); \
    CAMLreturn(Val_unit); \
}

BIND_TAG_PARAM(title);
BIND_TAG_PARAM(artist);
BIND_TAG_PARAM(album);
BIND_TAG_PARAM(year);
BIND_TAG_PARAM(comment);
BIND_TAG_PARAM(track);
/* TODO: check for errors. */
BIND_TAG_PARAM(genre);

/***** Informations *****/

BIND_GET_INT_PARAM(version);
BIND_GET_INT_PARAM(encoder_delay);
BIND_GET_INT_PARAM(framesize);
BIND_GET_INT_PARAM(mf_samples_to_encode);
BIND_GET_INT_PARAM(frameNum);
BIND_GET_INT_PARAM(totalframes);

#define BIND_GET_STRING_PARAM(p) \
CAMLprim value ocaml_lame_get_##p(value unit) \
{ \
    CAMLparam1(unit); \
    CAMLreturn(caml_copy_string(get_##p())); \
}

BIND_GET_STRING_PARAM(lame_version);
BIND_GET_STRING_PARAM(lame_short_version);
BIND_GET_STRING_PARAM(lame_very_short_version);
BIND_GET_STRING_PARAM(psy_version);
BIND_GET_STRING_PARAM(lame_url);
