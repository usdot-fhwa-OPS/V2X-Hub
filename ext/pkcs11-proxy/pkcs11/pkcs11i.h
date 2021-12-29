/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* pkcs11g.h - GNOME internal definitions to PKCS#11

   Copyright (C) 2008, Stef Walter

   The Gnome Keyring Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Keyring Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Stef Walter <stef@memberwebs.com>
*/

#ifndef PKCS11I_H
#define PKCS11I_H

#include "pkcs11.h"
#include "pkcs11g.h"

/* Signifies that nobody is logged in */
#define CKU_NONE G_MAXULONG

/* ----------------------------------------------------------------------
 * APARTMENT SLOTS
 *
 * The lower 10 bits of the CK_SLOT_ID are used as the actual slot identifier,
 * and the remainder are used as application identifiers.
 *
 * This enables a single loaded module to serve multiple applications
 * concurrently. The caller of a module should check the
 * CKF_GNOME_VIRTUAL_SLOTS flag before using this functionality.
 */

/* Flag for CK_INFO when virtual slots are supported */
#define CKF_GNOME_APPARTMENTS                       0x40000000

/* Get an actual slot id from a virtual slot */
#define CK_GNOME_APPARTMENT_SLOT(virt)              ((virt) & 0x000003FF)

/* Get an app id from a virtual slot */
#define CK_GNOME_APPARTMENT_APP(virt)               ((virt) >> 10)

/* Is the app id valid for use in a virtual slot? */
#define CK_GNOME_APPARTMENT_IS_APP(app)             ((app) < (((CK_ULONG)-1) >> 10))

/* Build a virtual slot from an actual slot id, and an app id */
#define CK_GNOME_MAKE_APPARTMENT(slot, app)         (((slot) & 0x000003FF) | ((app) << 10))

/* -------------------------------------------------------------------
 * LIMITED HANDLES
 *
 * The upper 10 bits of a CK_SESSION_HANDLE and CK_OBJECT_HANDLE are
 * never used by Gnome Keyring PKCS#11 modules. These bits are used
 * for tracking purposes when combining modules into a single module.
 */

#define CK_GNOME_MAX_SLOT                           (0x000003FF)
#define CK_GNOME_MAX_APP                            (((CK_ULONG)-1) >> 10)
#define CK_GNOME_MAX_HANDLE                         (((CK_ULONG)-1) >> 10)

/* -------------------------------------------------------------------
 * OBJECT HASH
 */

#define CKA_GNOME_INTERNAL_SHA1                      (CKA_GNOME + 1000)

#endif /* PKCS11I_H */
