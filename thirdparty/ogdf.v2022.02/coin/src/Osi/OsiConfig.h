/* Copyright (C) 2011
 * All Rights Reserved.
 * This code is published under the Eclipse Public License.
 *
 * $Id: OsiConfig.h 1754 2011-06-19 16:07:57Z stefan $
 *
 * Include file for the configuration of Osi.
 *
 * On systems where the code is configured with the configure script
 * (i.e., compilation is always done with HAVE_CONFIG_H defined), this
 * header file includes the automatically generated header file.
 *
 * On systems that are compiled in other ways (e.g., with the
 * Developer Studio), a header files is included to define those
 * macros that depend on the operating system and the compiler.  The
 * macros that define the configuration of the particular user setting
 * (e.g., presence of other COIN-OR packages or third party code) are set
 * by the files config_*default.h. The project maintainer needs to remember
 * to update these file and choose reasonable defines.
 * A user can modify the default setting by editing the config_*default.h files.
 */

#ifndef __OSICONFIG_H__
#define __OSICONFIG_H__

#ifdef HAVE_CONFIG_H
#ifdef OSI_BUILD
#include "coin/config.h"
#else
#include "config_osi.h"
#endif

#else /* HAVE_CONFIG_H */

#ifdef OSI_BUILD
#include "coin/config_default.h"
#else
#include "config_osi_default.h"
#endif

#endif /* HAVE_CONFIG_H */

#endif /*__OSICONFIG_H__*/
