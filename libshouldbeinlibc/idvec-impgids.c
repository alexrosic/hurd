/* Add gids implied by a user

   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdlib.h>
#include <errno.h>
#include <idvec.h>
#include <pwd.h>
#include <grp.h>

#define NUM_STATIC_GIDS 100	/* Initial size of static gid array.  */

/* The set of gids implied by a uid.  */
struct uid_implies
{
  uid_t uid;			/* this uid... */
  struct idvec *implies;	/* implies these gids.  */
  struct uid_implies *next;
};

/* Cache of previously calculated results for add_implied_gids.  */
static struct uid_implies *uid_implies_cache = 0;

/* Add to IMPLIED_GIDS those group ids implied by the user UID.  */
static error_t
_merge_implied_gids (struct idvec *implied_gids, uid_t uid)
{
  struct uid_implies *ui;

  for (ui = uid_implies_cache; ui; ui = ui->next)
    if (ui->uid == uid)
      return idvec_merge (implied_gids, ui->implies);

  {
    error_t err = 0;
    struct passwd *pw = getpwuid (uid);

    if (! pw)
      err = EINVAL;
    else
      {
	struct idvec *cache = make_idvec ();
#if 0  /* Glibc doesn't have getgrouplist yet.  */
	gid_t _gids[NUM_STATIC_GIDS], *gids = _gids;
	size_t maxgids = NUM_STATIC_GIDS;
	size_t ngids =
	  getgrouplist (pw->pw_name, pw->pw_gid, gids, &maxgids);

	if (ngids == -1)
	  {
	    gids = malloc (maxgids * sizeof (gid_t));
	    if (! gids)
	      err = ENOMEM;
	    else
	      ngids = getgrouplist (pw->pw_name, pw->pw_gid, gids, &maxgids);
	  }

	if (! cache)
	  err = ENOMEM;

	if (! err)
	  {
	    err = idvec_merge_ids (cache, gids, ngids);
	    if (gids != _gids)
	      free (gids);
	  }
#else
	if (! cache)
	  err = ENOMEM;
	else
	  err = idvec_add_new (cache, pw->pw_gid);
#endif

	if (! err)
	  {
	    idvec_merge (implied_gids, cache);
	    ui = malloc (sizeof (struct uid_implies));
	    if (ui)
	      {
		ui->uid = uid;
		ui->implies = cache;
		ui->next = uid_implies_cache;
		uid_implies_cache = ui;
	      }
	    else
	      idvec_free (cache);
	  }
      }

    return err;
  }
}

/* Add to GIDS those group ids implied by the users in UIDS.  */
error_t
idvec_merge_implied_gids (struct idvec *gids, const struct idvec *uids)
{
  int i;
  error_t err = 0;
  for (i = 0; i < uids->num && !err; i++)
    err = _merge_implied_gids (gids, uids->ids[i]);
  return err;
}