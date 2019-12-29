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

#include "tinymm.h"

#include <errno.h>
#if WITH_THREADS
#include <pthread.h>
#endif
#include <string.h>

struct tinymm_page {
	struct tinymm_page *next, *prev;
	unsigned long alloc_cnt;
	void *base, *ptr;
};

struct tinymm {
	void * (*alloc)(size_t);
	void (*free)(void *);
	size_t page_size;
	struct tinymm_page *page_list;
#if WITH_THREADS
	pthread_mutex_t mutex;
#endif
};

static void tinymm_lock(struct tinymm *mm)
{
#if WITH_THREADS
	pthread_mutex_lock(&mm->mutex);
#endif
}

static void tinymm_unlock(struct tinymm *mm)
{
#if WITH_THREADS
	pthread_mutex_unlock(&mm->mutex);
#endif
}

static void tinymm_free_page(struct tinymm *mm, struct tinymm_page *page)
{
	if (page->prev)
		page->prev->next = page->next;
	else
		mm->page_list = page->next;

	if (page->next)
		page->next->prev = page->prev;

	(*mm->free)(page);
}

struct tinymm *tinymm_init(void * (*alloc)(size_t), void (*free)(void *),
			   size_t page_size)
{
	struct tinymm *mm;
	int ret;

	if (!alloc || !free) {
		errno = EINVAL;
		return NULL;
	}

	mm = (*alloc)(sizeof(*mm));
	if (!mm) {
		errno = ENOMEM;
		return NULL;
	}

#if WITH_THREADS
	ret = pthread_mutex_init(&mm->mutex, NULL);
	if (ret) {
		(*free)(mm);
		errno = -ret;
		return NULL;
	}
#endif

	mm->alloc = alloc;
	mm->free = free;
	mm->page_list = NULL;
	mm->page_size = page_size;

	return mm;
}

void tinymm_shutdown(struct tinymm *mm)
{
	struct tinymm_page *page, *next;

	for (page = mm->page_list; page; page = next) {
		next = page->next;
		tinymm_free_page(mm, page);
	}

#if WITH_THREADS
	pthread_mutex_destroy(&mm->mutex);
#endif

	(*mm->free)(mm);
}

void * tinymm_malloc(struct tinymm *mm, size_t len)
{
	struct tinymm_page *page, *prev;
	void *ptr;

	/* Align to pointer size */
	if (len & (sizeof(void *) - 1))
		len = (len & ~(sizeof(void *) - 1)) + sizeof(void *);

	tinymm_lock(mm);

	for (prev = NULL, page = mm->page_list; page;
	     prev = page, page = page->next)
		if (page->ptr + len < page->base + mm->page_size)
			break;

	if (!page) {
		page = (*mm->alloc)(sizeof(*page) + mm->page_size);
		if (!page) {
			tinymm_unlock(mm);
			errno = ENOMEM;
			return NULL;
		}

		page->next = NULL;
		page->prev = prev;
		if (prev)
			prev->next = page;
		else
			mm->page_list = page;

		page->ptr = page->base = (void *)page + sizeof(*page);
		page->alloc_cnt = 0;
	}

	ptr = page->ptr;
	page->ptr += len;
	page->alloc_cnt++;

	tinymm_unlock(mm);

	return ptr;
}

void * tinymm_zalloc(struct tinymm *mm, size_t len)
{
	void *ptr;

	ptr = tinymm_malloc(mm, len);
	if (ptr)
		memset(ptr, 0, len);

	return ptr;
}

void tinymm_free(struct tinymm *mm, void *ptr)
{
	struct tinymm_page *page;

	tinymm_lock(mm);

	for (page = mm->page_list; page; page = page->next)
		if (ptr >= page->base && ptr < page->base + mm->page_size)
			break;

	if (page && !--page->alloc_cnt)
		tinymm_free_page(mm, page);

	tinymm_unlock(mm);
}
