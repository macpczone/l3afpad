/*
 * L3afpad - GTK+ based simple text editor
 * Copyright (C) 2004-2005 Tarot Osuji
 * Copyright (C)      2026 Nube
 */

#include "l3afpad.h"
#include <string.h>

static GList *recent_files = NULL;

void recent_add_file(const gchar *filename)
{
	GList *l;

	if (!filename || filename[0] == '\0')
		return;

	for (l = recent_files; l; l = l->next) {
		if (strcmp((gchar *)l->data, filename) == 0) {
			g_free(l->data);
			recent_files = g_list_delete_link(recent_files, l);
			break;
		}
	}

	recent_files = g_list_prepend(recent_files, g_strdup(filename));

	if (g_list_length(recent_files) > MAX_RECENT_FILES) {
		GList *last = g_list_last(recent_files);
		if (last) {
			g_free(last->data);
			recent_files = g_list_delete_link(recent_files, last);
		}
	}
}

GList *recent_get_list(void)
{
	return recent_files;
}

void recent_clear(void)
{
	g_list_free_full(recent_files, g_free);
	recent_files = NULL;
}