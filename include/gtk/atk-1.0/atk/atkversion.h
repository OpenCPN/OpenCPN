/* ATK -  Accessibility Toolkit
 *
 * Copyright (C) 2012 Igalia, S.L.
 *
 * Author: Alejandro Piñeiro Iglesias <apinheiro@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if defined(ATK_DISABLE_SINGLE_INCLUDES) && !defined (__ATK_H_INSIDE__) && !defined (ATK_COMPILATION)
#error "Only <atk/atk.h> can be included directly."
#endif

#ifndef __ATK_VERSION_H__
#define __ATK_VERSION_H__

/**
 * ATK_MAJOR_VERSION:
 *
 * Like atk_get_major_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.7.4
 */
#define ATK_MAJOR_VERSION (2)

/**
 * ATK_MINOR_VERSION:
 *
 * Like atk_get_minor_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.7.4
 */
#define ATK_MINOR_VERSION (10)

/**
 * ATK_MICRO_VERSION:
 *
 * Like atk_get_micro_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.7.4
 */
#define ATK_MICRO_VERSION (0)

/**
 * ATK_BINARY_AGE:
 *
 * Like atk_get_binary_age(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.7.4
 */
#define ATK_BINARY_AGE    (21010)

/**
 * ATK_INTERFACE_AGE:
 *
 * Like atk_get_interface_age(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.7.4
 */
#define ATK_INTERFACE_AGE (1)

/**
 * ATK_CHECK_VERSION:
 * @major: major version (e.g. 1 for version 1.2.5)
 * @minor: minor version (e.g. 2 for version 1.2.5)
 * @micro: micro version (e.g. 5 for version 1.2.5)
 *
 * Returns %TRUE if the version of the ATK header files is the same as
 * or newer than the passed-in version.
 *
 * Since: 2.7.4
 */
#define ATK_CHECK_VERSION(major,minor,micro)                          \
    (ATK_MAJOR_VERSION > (major) ||                                   \
     (ATK_MAJOR_VERSION == (major) && ATK_MINOR_VERSION > (minor)) || \
     (ATK_MAJOR_VERSION == (major) && ATK_MINOR_VERSION == (minor) && \
      ATK_MICRO_VERSION >= (micro)))


guint atk_get_major_version (void) G_GNUC_CONST;
guint atk_get_minor_version (void) G_GNUC_CONST;
guint atk_get_micro_version (void) G_GNUC_CONST;
guint atk_get_binary_age    (void) G_GNUC_CONST;
guint atk_get_interface_age (void) G_GNUC_CONST;

#define atk_major_version atk_get_major_version ()
#define atk_minor_version atk_get_minor_version ()
#define atk_micro_version atk_get_micro_version ()
#define atk_binary_age atk_get_binary_age ()
#define atk_interface_age atk_get_interface_age ()

#endif /* __ATK_VERSION_H__ */
