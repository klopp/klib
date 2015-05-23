/*
 * config.h
 *
 *  Created on: 04.05.2015
 *      Author: klopp
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// _MSC_VER
#ifndef __WINDOWS__
# if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#  define __WINDOWS__
# endif
#endif

#ifndef PATH_MAX
# if defined(MAX_PATH)
#  define PATH_MAX MAX_PATH
# else
#  define PATH_MAX 256
# endif
#endif

#if defined(_MSC_VER)
# define pid_t          int
# define strcasecmp     _stricmp
# define snprintf       _snprintf
# define _CRT_SECURE_NO_WARNINGS
# pragma warning(disable : 4996)
#else
# include <unistd.h>
#endif

#ifndef __func__
# if defined(__FUNCTION__)
#  define __func__ __FUNCTION__
# elif defined(__FUNC__)
#  define __func__ __FUNC__
# endif
#endif

#endif /* CONFIG_H_ */
