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

#include "l3afpad.h"
#include <stdint.h>
#include <libgen.h>
#include <stdlib.h>

#if ENABLE_DEBUG
#define AUTOSAVE_DEBUG(fmt, ...) g_print("[autosave] " fmt "\n", ##__VA_ARGS__)
#else
#define AUTOSAVE_DEBUG(fmt, ...) ((void)0)
#endif

#define MAX_FILE_PATH_SIZE 256

static gboolean autosave = FALSE;
static guint autosave_timer = 10000;
static gboolean autosave_same_dir = TRUE;
static guint autosave_immediate_changes = 100;
/**
 * The path (relative to "$HOME") to the directory
 * where autosave files are stored.
 */
static gchar* AUTOSAVE_DIR_PATH = ".cache/l3afpad/buffers";

typedef struct {
	/** GTK handler/signal ID of the auto-save timer or 0 if none. */
	guint timer_id;
	/** Number of changes to the text buffer since last (auto-)save. */
	guint changes;
	/** Name of the temporary auto-save file. Empty ([0] == '\0') if none. */
	gchar filename[256];
} AutoSaveData;

static AutoSaveData autosave_data = { 0, 0, "" };

static void AutoSaveData_init(AutoSaveData *data)
{
	data->timer_id = 0;
	data->changes = 0;
	data->filename[0] = '\0';
}

gboolean autosave_get_state(void)
{
	return autosave;
}

void autosave_set_state(gboolean state)
{
	autosave = state;
}

guint autosave_get_timer(void)
{
	return autosave_timer;
}

void autosave_set_timer(guint milliseconds)
{
	autosave_timer = milliseconds;
}

gboolean autosave_get_same_dir(void)
{
	return autosave_same_dir;
}

void autosave_set_same_dir(gboolean state)
{
	autosave_same_dir = state;
}

guint autosave_get_immediate_changes(void)
{
	return autosave_immediate_changes;
}

void autosave_set_immediate_changes(guint num_changes)
{
	autosave_immediate_changes = num_changes;
}

static void autosave_reset_num_changes(void)
{
	autosave_data.changes = 0;
}

/**
 * Generates a random 32-bit unsigned integer for use as an autosave file
 * identifier. Relies on srand() being called elsewhere (e.g. in main()).
 */
static inline uint32_t autosave_generate_random_id(void)
{
	return (uint32_t) rand();
}

/**
 * Generates a new auto-save filename, and stores it in autosave_data.filename.
 */
static gboolean autosave_generate_filename(GtkTextBuffer *buffer) {

	gboolean uses_cache_dir = TRUE;
	// Generate a random identifier for the autosave filename
	// to avoid collisions with other autosave files.
	const uint32_t random_id = autosave_generate_random_id();
	//autosave_data.filename[MAX_FILE_PATH_SIZE];
	if (pub->fi->filename == NULL) {
		// We are editing a text buffer without having chosen a filename yet
		char *home;
		home = getenv("HOME");
		sprintf(autosave_data.filename, "%s/%s/%u.text", home, AUTOSAVE_DIR_PATH, random_id);
	} else {
		// Known/chosen filename
		gchar real_filename_base_buf[MAX_FILE_PATH_SIZE];
		strcpy(real_filename_base_buf, pub->fi->filename);
		char* real_filename_base = basename(real_filename_base_buf);
		if (autosave_same_dir) {
			gchar real_filename_dir_buf[MAX_FILE_PATH_SIZE];
			strcpy(real_filename_dir_buf, pub->fi->filename);
			char* real_filename_dir = dirname(real_filename_dir_buf);
			sprintf(autosave_data.filename, "%s/.%u_%s", real_filename_dir, random_id, real_filename_base);
			uses_cache_dir = FALSE;
		} else {
			char *home;
			home = getenv("HOME");
			sprintf(autosave_data.filename, "%s/%s/.%u_%s", home, AUTOSAVE_DIR_PATH, random_id, real_filename_base);
		}
	}
	return uses_cache_dir;
}

static void autosave_delete_file(gchar *filename)
{
	if (filename[0] != '\0') {
		const int remove_status = remove(filename);
		if (remove_status != 0) {
			perror("Warning: Failed to delete auto-save file");
		}

		filename[0] = '\0';
	}
}

static void autosave_try_save(GtkWidget *view) {

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	if (gtk_text_buffer_get_modified(buffer)) {
		AUTOSAVE_DEBUG("autosave_try_save: writing to %s", autosave_data.filename);
		// perform the auto-save
		FileInfo autosave_file_info = { autosave_data.filename, pub->fi->charset, pub->fi->charset_flag, pub->fi->lineend };
		/*const gint save_err = */file_save_real(view, &autosave_file_info);
		gtk_text_buffer_set_modified(buffer, TRUE);
		autosave_reset_num_changes();
		AUTOSAVE_DEBUG("autosave complete, changes reset");
	}
}

static gboolean idle_handler(GtkWidget *view) {

	// mark this timer as done
	autosave_data.timer_id = 0;

	autosave_try_save(view);

	// stop this idle signal
	return FALSE;
}

static gboolean time_handler(GtkWidget *view) {

	/*guint idle_id = */g_idle_add((GSourceFunc) idle_handler, view);

	// stop this timer
	return FALSE;
}

void autosave_cb_buffer_changed(GtkTextBuffer *buffer, GtkWidget *view)
{
	autosave_data.changes++;
	AUTOSAVE_DEBUG("buffer changed: changes=%u", autosave_data.changes);
	if (autosave) {
		/* Generate the autosave filename once, on the first change. */
		if (autosave_data.filename[0] == '\0') {
			autosave_generate_filename(buffer);
			AUTOSAVE_DEBUG("generated autosave filename: %s", autosave_data.filename);
			if (autosave_data.filename[0] != '\0') {
				gchar parent_buf[MAX_FILE_PATH_SIZE];
				strcpy(parent_buf, autosave_data.filename);
				const gchar* parent = dirname(parent_buf);
				g_mkdir_with_parents(parent, 0700);
			}
		}
		if (autosave_data.timer_id > 0) {
			g_source_remove(autosave_data.timer_id);
			autosave_data.timer_id = 0;
		}
		if (autosave_data.changes >= autosave_immediate_changes) {
			AUTOSAVE_DEBUG("immediate autosave threshold reached");
			autosave_try_save(view);
		} else {
			autosave_data.timer_id = g_timeout_add(autosave_timer, (GSourceFunc) time_handler, view);
			AUTOSAVE_DEBUG("autosave timer set (%u ms)", autosave_timer);
		}
	}
}

static void autosave_kill_timer()
{
	if (autosave_data.timer_id > 0) {
		g_source_remove(autosave_data.timer_id);
		autosave_data.timer_id = 0;
	}
}

void autosave_cb_file_saved(gchar *filename)
{
	AUTOSAVE_DEBUG("file_saved: %s (autosave: %s)", filename, autosave_data.filename);
	autosave_kill_timer();
	if (strcmp(filename, autosave_data.filename) != 0) {
		AUTOSAVE_DEBUG("not autosave file, discarding temp");
		// not saving to our temporary/auto-save file
		autosave_discard_temp_file();
	}
}

void autosave_discard_temp_file()
{
	AUTOSAVE_DEBUG("discarding temp file: %s", autosave_data.filename);
	autosave_kill_timer();
	autosave_delete_file(autosave_data.filename);
	AutoSaveData_init(&autosave_data);
}

