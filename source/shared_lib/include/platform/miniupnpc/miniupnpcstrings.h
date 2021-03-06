/* $Id: miniupnpcstrings.h.in,v 1.2 2009/10/30 09:18:18 nanard Exp $ */
/* Project: miniupnp
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * Author: Thomas Bernard
 * Copyright (c) 2005-2009 Thomas Bernard
 * This software is subjects to the conditions detailed
 * in the LICENCE file provided within this distribution */
#ifndef __MINIUPNPCSTRINGS_H__
#define __MINIUPNPCSTRINGS_H__

 //#include "lib/framework/wzglobal.h"

#if defined(WIN32)
#define OS_STRING "Windows"
//#elif defined (WZ_OS_MAC)
//#define OS_STRING "Mac OS"
#elif defined(__linux__)
#define OS_STRING "Linux"
#else
#define OS_STRING "Other/unknown"
#endif

#define MINIUPNPC_VERSION_STRING "1.9"
#define UPNP_VERSION_STRING "UPnP/1.1"
#define MINIUPNPC_API_VERSION  15

#endif

