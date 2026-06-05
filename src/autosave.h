/*
 *  L3afpad - GTK+ based simple text editor
 *  Copyright (C) 2017 Tarot Osuji
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _AUTOSAVE_H
#define _AUTOSAVE_H

/**
 * Returns the value of the config setting "AutoSave".
 * This setting determines whether auto-save functionality is enabled.
 */
gboolean autosave_get_state(void);
void autosave_set_state(gboolean state);

/**
 * Returns the value of the config setting "AutoSaveTimer".
 * This setting determines the time in milliseconds to wait
 * before saving buffer contents to a temporary file
 * after the last (unsaved) change.
 */
guint autosave_get_timer(void);
void autosave_set_timer(guint milliseconds);

/**
 * Returns the value of the config setting "AutoSaveSameDir".
 * This setting determines whether to save temp-files in the same dir
 * as the real file, or in the system-wide l3afpad auto-save dir.
 * This setting only has an effect if a filename is already chosen,
 * as (Unnamed) buffers will always be auto-saved in the system wide
 * auto-save dir.
 */
gboolean autosave_get_same_dir(void);
void autosave_set_same_dir(gboolean state);

/**
 * Returns the value of the config setting "AutoSaveImmediateChanges".
 * This setting determines the ammount of changes to a buffer
 * after which auto-saveing is performed immediatly.
 */
guint autosave_get_immediate_changes(void);
void autosave_set_immediate_changes(guint num_changes);

/** Called when the text in the internal buffer changed */
void autosave_cb_buffer_changed(GtkTextBuffer *buffer, GtkWidget *view);
/**
 * Called when the buffer contents were save in a file,
 * which might be the propper one, chosen by the user,
 * or the auto-save file.
 */
void autosave_cb_file_saved(gchar* filename);
/**
 * Discards/deletes the (temporary) auto-save file.
 */
void autosave_discard_temp_file();

#endif /* _AUTOSAVE_H */

