/*
 * Copyright (C) 2004 Jason Woodward <woodwardj at jaos dot org>
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

struct head_request_t {
	char *data;
	size_t size;
};
                                                                                                                             
/* this is the main download routine */
int download_data(FILE *fh,const char *url,size_t bytes,int use_curl_dl_stats);

/* this performs a head request */
int head_request(const char *filename,const char *url);

/*
	this fills FILE with data from url, used for PACKAGES.TXT and CHECKSUMS
*/
int get_mirror_data_from_source(FILE *fh,int use_curl_dl_stats,const char *base_url,const char *filename);

/* download pkg, cals download_data */
int download_pkg(const rc_config *global_config,pkg_info_t *pkg);
void create_dir_structure(const char *dir_name);

/* generate an md5sum of filehandle */
void gen_md5_sum_of_file(FILE *f,char *result_sum);
