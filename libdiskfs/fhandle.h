/* File handle type (for nfs server support)

   Copyright (C) 1997 Free Software Foundation

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

#ifndef __FHANDLE_H__
#define __FHANDLE_H__

/* Must be exactly 28 bytes long */
struct diskfs_fhandle
{
  int filler1;
  int cache_id;
  long gen;
  int filler2[28 - sizeof (int) + sizeof (int) + sizeof (long)];
};

#endif /* __FHANDLE_H__ */