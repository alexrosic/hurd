/*
   Copyright (C) 1993, 1994 Free Software Foundation

This file is part of the GNU Hurd.

The GNU Hurd is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU Hurd is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU Hurd; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Written by Michael I. Bushnell.  */

#include "priv.h"
#include "io_S.h"
#include <assert.h>

kern_return_t
trivfs_S_io_reauthenticate (struct trivfs_protid *cred,
			    int rendint)
{
  struct trivfs_protid *newcred;
  uid_t gubuf[20], ggbuf[20], aubuf[20], agbuf[20];
  uid_t *gen_uids, *gen_gids, *aux_uids, *aux_gids;
  u_int genuidlen, gengidlen, auxuidlen, auxgidlen;
  error_t err;
  int i;
  auth_t auth;

  if (cred == 0)
    return EOPNOTSUPP;

  genuidlen = gengidlen = auxuidlen = auxgidlen = 20;
  gen_uids = gubuf;
  gen_gids = ggbuf;
  aux_uids = aubuf;
  aux_gids = agbuf;

  newcred = ports_allocate_port (sizeof (struct trivfs_protid), cred->pi.type);
  auth = getauth ();
  err = auth_server_authenticate (auth, 
				  ports_get_right (cred),
				  MACH_MSG_TYPE_MAKE_SEND,
				  rendint,
				  ports_get_right (newcred),
				  MACH_MSG_TYPE_MAKE_SEND,
				  &gen_uids, &genuidlen, 
				  &aux_uids, &auxuidlen,
				  &gen_gids, &gengidlen,
				  &aux_gids, &auxgidlen);
  assert (!err);		/* XXX */
  mach_port_deallocate (mach_task_self (), auth);

  newcred->isroot = 0;
  for (i = 0; i < genuidlen; i++)
    if (gen_uids[i] == 0)
      newcred->isroot = 1;

  newcred->uids = malloc (genuidlen * sizeof (uid_t));
  newcred->gids = malloc (gengidlen * sizeof (uid_t));
  bcopy (gen_uids, newcred->uids, genuidlen * sizeof (uid_t));
  bcopy (gen_gids, newcred->gids, gengidlen * sizeof (uid_t));
  newcred->nuids = genuidlen;
  newcred->ngids = gengidlen;
  
  newcred->po = cred->po;
  newcred->po->refcnt++;
  
  err = io_restrict_auth (newcred->po->cntl->underlying, &newcred->realnode,
			  gen_uids, genuidlen, gen_gids, gengidlen);
  if (err)
    newcred->realnode = MACH_PORT_NULL;

  if (trivfs_protid_create_hook)
    (*trivfs_protid_create_hook) (newcred);

  if (gubuf != gen_uids)
    vm_deallocate (mach_task_self (), (u_int) gen_uids,
		   genuidlen * sizeof (uid_t));
  if (ggbuf != gen_gids)
    vm_deallocate (mach_task_self (), (u_int) gen_gids,
		   gengidlen * sizeof (uid_t));
  if (aubuf != aux_uids)
    vm_deallocate (mach_task_self (), (u_int) aux_uids,
		   auxuidlen * sizeof (uid_t));
  if (agbuf != aux_gids)
    vm_deallocate (mach_task_self (), (u_int) aux_gids,
		   auxgidlen * sizeof (uid_t));
  
  return 0;
}
