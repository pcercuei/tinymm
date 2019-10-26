/*
 * Copyright (C) 2019 Paul Cercueil <paul@crapouillou.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __TINYMM_H__
#define __TINYMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#ifdef _WIN32
#   ifdef tinymm_EXPORTS
#	define __api __declspec(dllexport)
#   elif !defined(TINYMM_STATIC)
#	define __api __declspec(dllimport)
#   else
#	define __api
#   endif
#elif __GNUC__ >= 4
#   define __api __attribute__((visibility ("default")))
#else
#   define __api
#endif

struct tinymm;

__api struct tinymm * tinymm_init(void * (*alloc)(size_t), void (*free)(void *),
				  size_t page_size);
__api void tinymm_shutdown(struct tinymm *mm);

__api void * tinymm_malloc(struct tinymm *mm, size_t len);
__api void * tinymm_zalloc(struct tinymm *mm, size_t len);

__api void tinymm_free(struct tinymm *mm, void *ptr);

#ifdef __cplusplus
};
#endif

#endif /* __TINYMM_H__ */
