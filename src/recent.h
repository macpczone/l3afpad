/*
 * L3afpad - GTK+ based simple text editor
 * Copyright (C) 2004-2005 Tarot Osuji
 * Copyright (C)      2026 Nube
 */

#ifndef _RECENT_H
#define _RECENT_H

#include <glib.h>

#define MAX_RECENT_FILES 10

void recent_add_file(const gchar *filename);
GList *recent_get_list(void);
void recent_clear(void);

#endif /* _RECENT_H */