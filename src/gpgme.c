/*
 * Copyright (C) 2003-2007 Jason Woodward <woodwardj at jaos dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "main.h"

FILE *slapt_get_pkg_source_checksums_signature (const slapt_rc_config *global_config,
                                                const char *url,
                                                unsigned int *compressed)
{
  FILE *tmp_checksum_f = NULL;
  char *checksum_head = NULL;
  SLAPT_BOOL_T interactive  = global_config->progress_cb == NULL && global_config->dl_stats == SLAPT_FALSE
                            ? SLAPT_TRUE
                            : SLAPT_FALSE;
  char *location_uncompressed = SLAPT_CHECKSUM_ASC_FILE;
  char *location_compressed = SLAPT_CHECKSUM_ASC_FILE_GZ;
  char *filename = NULL;
  char *local_head = NULL;
  char *location;
  int checksums_compressed = *compressed;

  if (checksums_compressed == 1) {
    location = location_compressed;
    *compressed = 1;
  } else {
    location = location_uncompressed;
    *compressed = 0;
  }

  filename = slapt_gen_filename_from_url(url,location);
  local_head = slapt_read_head_cache(filename);
  checksum_head = slapt_head_mirror_data(url,location);

  if (checksum_head == NULL)
  {
      if (interactive == SLAPT_TRUE)
        printf(gettext("Not Found\n"));
      free(filename);
      free(local_head);
      if (checksum_head != NULL)
        free(checksum_head);
      return NULL;
  }

  if (checksum_head != NULL && local_head != NULL &&
      strcmp(checksum_head,local_head) == 0) {
    if ((tmp_checksum_f = slapt_open_file(filename,"r")) == NULL)
      exit(EXIT_FAILURE);

    if (global_config->progress_cb == NULL)
      printf(gettext("Cached\n"));

  } else {
    const char *err = NULL;

    if ((tmp_checksum_f = slapt_open_file(filename,"w+b")) == NULL)
      exit(EXIT_FAILURE);

    err = slapt_get_mirror_data_from_source(tmp_checksum_f,
                                            global_config,url,
                                            location);
    if (!err) {

      if (interactive == SLAPT_TRUE)
        printf(gettext("Done\n"));

    } else {
      fprintf(stderr,gettext("Failed to download: %s\n"),err);
      slapt_clear_head_cache(filename);
      fclose(tmp_checksum_f);
      free(filename);
      free(local_head);
      if (checksum_head != NULL)
        free(checksum_head);
      return NULL;
    }
    /* make sure we are back at the front of the file */
    rewind(tmp_checksum_f);

    /* if all is good, write it */
    if (checksum_head != NULL)
      slapt_write_head_cache(checksum_head,filename);

  }

  free(filename);
  free(local_head);

  if (checksum_head != NULL)
    free(checksum_head);

  return tmp_checksum_f;
}

FILE *slapt_get_pkg_source_gpg_key(const slapt_rc_config *global_config,
                                   const char *url,
                                   unsigned int *compressed)
{
  FILE *tmp_key_f = NULL;
  char *key_head = NULL;
  char *filename = slapt_gen_filename_from_url(url,SLAPT_GPG_KEY);
  char *local_head = slapt_read_head_cache(filename);
  SLAPT_BOOL_T interactive  = global_config->progress_cb == NULL && global_config->dl_stats == SLAPT_FALSE
                            ? SLAPT_TRUE
                            : SLAPT_FALSE;

  *compressed = 0;
  key_head = slapt_head_mirror_data(url,SLAPT_GPG_KEY);

  if (key_head == NULL) {
      if (interactive == SLAPT_TRUE)
        printf(gettext("Not Found\n"));
      free(filename);
      free(local_head);
      if (key_head != NULL)
        free(key_head);
      return NULL;
  }

  if (key_head != NULL && local_head != NULL &&
      strcmp(key_head,local_head) == 0) {

    if ((tmp_key_f = slapt_open_file(filename,"r")) == NULL)
      exit(EXIT_FAILURE);

    if (global_config->progress_cb == NULL)
      printf(gettext("Cached\n"));

  } else {
    const char *err = NULL;

    if ((tmp_key_f = slapt_open_file(filename,"w+b")) == NULL)
      exit(EXIT_FAILURE);

    err = slapt_get_mirror_data_from_source(tmp_key_f,
                                            global_config, url,
                                            SLAPT_GPG_KEY);

    if (!err) {
      if (interactive == SLAPT_TRUE)
        printf(gettext("Done\n"));
    } else{
      fprintf(stderr,gettext("Failed to download: %s\n"),err);
      slapt_clear_head_cache(filename);
      fclose(tmp_key_f);
      free(filename);
      free(local_head);
      if (key_head != NULL)
        free(key_head);
      return NULL;
    }

    rewind(tmp_key_f);

    /* if all is good, write it */
    if (key_head != NULL)
      slapt_write_head_cache(key_head, filename);

  }

  free(filename);
  free(local_head);

  if (key_head != NULL)
    free(key_head);

  return tmp_key_f;
}

slapt_code_t slapt_add_pkg_source_gpg_key (FILE *key)
{
  gpgme_error_t e;
  gpgme_ctx_t ctx;
  gpgme_import_result_t import_result;
  gpgme_data_t key_data;
  slapt_code_t imported = SLAPT_GPG_KEY_NOT_IMPORTED;

  e = gpgme_new          (&ctx);
  assert                 (e == GPG_ERR_NO_ERROR);
  e = gpgme_set_protocol (ctx, GPGME_PROTOCOL_OpenPGP);
  assert                 (e == GPG_ERR_NO_ERROR);
  gpgme_set_armor (ctx, 1);
  
  e = gpgme_data_new_from_stream (&key_data, key);
  assert (e == GPG_ERR_NO_ERROR);

  e = gpgme_op_import(ctx, key_data);
  assert (e == GPG_ERR_NO_ERROR);

  import_result = gpgme_op_import_result(ctx);
  if (import_result != NULL)
  {
    if (import_result->unchanged > 0)
      imported = SLAPT_GPG_KEY_UNCHANGED;
    else if (import_result->imported > 0)
      imported = SLAPT_GPG_KEY_IMPORTED;
  }

  gpgme_data_release  (key_data);
  gpgme_release       (ctx);
  return imported;
}

slapt_code_t slapt_gpg_verify_checksums(FILE *checksums,
                                        FILE *signature)
{
  gpgme_error_t e;
  gpgme_ctx_t ctx;
  gpgme_data_t chk_data, asc_data;
  slapt_code_t verified = SLAPT_CHECKSUMS_NOT_VERIFIED;

  e = gpgme_new          (&ctx);
  assert                 (e == GPG_ERR_NO_ERROR);
  e = gpgme_set_protocol (ctx, GPGME_PROTOCOL_OpenPGP);
  assert                 (e == GPG_ERR_NO_ERROR);
  gpgme_set_armor (ctx, 1);

  e = gpgme_data_new_from_stream (&chk_data, checksums);
  assert                         (e == GPG_ERR_NO_ERROR);
  e = gpgme_data_new_from_stream (&asc_data, signature);
  assert                         (e == GPG_ERR_NO_ERROR);

  e = gpgme_op_verify (ctx, asc_data, chk_data, NULL);
  /* assert              (e == GPG_ERR_NO_ERROR); */
  
  if (e == GPG_ERR_NO_ERROR)
  {
    gpgme_verify_result_t verify_result = gpgme_op_verify_result (ctx);
    if (verify_result != NULL)
    {
      gpgme_signature_t sig = verify_result->signatures;
      gpgme_sigsum_t    sum = sig->summary;
      gpgme_error_t  status = sig->status;

      if (sum & GPGME_SIGSUM_VALID || status == GPG_ERR_NO_ERROR) {
        verified = SLAPT_CHECKSUMS_VERIFIED;
      } else if (sum & GPGME_SIGSUM_KEY_MISSING) {
        verified = SLAPT_CHECKSUMS_MISSING_KEY;
      }

    }
  }

  gpgme_data_release  (chk_data);
  gpgme_data_release  (asc_data);
  gpgme_release       (ctx);

  return verified;
}
