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

#define MAX_FILE_PATH_SIZE 256

static gboolean auto_save = FALSE;
static guint auto_save_timer = 10000;
static gboolean auto_save_same_dir = TRUE;
static guint auto_save_immediate_changes = 150;
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

static AutoSaveData auto_save_data = { 0, 0, "" };

static void AutoSaveData_init(AutoSaveData *data)
{
	data->timer_id = 0;
	data->changes = 0;
	data->filename[0] = '\0';
}

gboolean autosave_get_state(void)
{
	return auto_save;
}

void autosave_set_state(gboolean state)
{
	auto_save = state;
}

guint autosave_get_timer(void)
{
	return auto_save_timer;
}

void autosave_set_timer(guint milliseconds)
{
	auto_save_timer = milliseconds;
}

gboolean autosave_get_same_dir(void)
{
	return auto_save_same_dir;
}

void autosave_set_same_dir(gboolean state)
{
	auto_save_same_dir = state;
}

guint autosave_get_immediate_changes(void)
{
	return auto_save_immediate_changes;
}

void autosave_set_immediate_changes(guint num_changes)
{
	auto_save_immediate_changes = num_changes;
}

static void autosave_reset_num_changes(void)
{
	auto_save_data.changes = 0;
}

/** @see https://stackoverflow.com/questions/11871245/knuth-multiplicative-hash */
static inline uint32_t hash(const uint32_t value)
{
	return value * UINT32_C(2654435761);
}

/**
 * Generates a new auto-save filename, and stores it in auto_save_data.filename.
 */
static gboolean autosave_generate_filename(GtkTextBuffer *buffer) {

	gboolean uses_cache_dir = TRUE;
	// Create a hash from our buffers pointer value,
	// in order to circumvent the case of multiple opened files
	// with the same name overwriting each others buffer.
	//
	// NOTE In case of a 64bit system, we discard the upper 32bits here,
	//   but as the addresses are very likley to be different in the lower 32bits,
	//   there should generally be no problem.
	const uint32_t pointer_hash = hash((uint32_t) buffer);
	//auto_save_data.filename[MAX_FILE_PATH_SIZE];
	if (pub->fi->filename == NULL) {
		// We are editing a text buffer without having chosen a filename yet
		char *home;
		home = getenv("HOME");
		sprintf(auto_save_data.filename, "%s/%s/%u.txt", home, AUTOSAVE_DIR_PATH, pointer_hash);
	} else {
		// Known/chosen filename
		gchar real_filename_base_buf[MAX_FILE_PATH_SIZE];
		strcpy(real_filename_base_buf, pub->fi->filename);
		char* real_filename_base = basename(real_filename_base_buf);
		if (auto_save_same_dir) {
			gchar real_filename_dir_buf[MAX_FILE_PATH_SIZE];
			strcpy(real_filename_dir_buf, pub->fi->filename);
			char* real_filename_dir = dirname(real_filename_dir_buf);
			sprintf(auto_save_data.filename, "%s/.%u_%s", real_filename_dir, pointer_hash, real_filename_base);
			uses_cache_dir = FALSE;
		} else {
			char *home;
			home = getenv("HOME");
			sprintf(auto_save_data.filename, "%s/%s/.%u_%s", home, AUTOSAVE_DIR_PATH, pointer_hash, real_filename_base);
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

/**
 * Ensure the parent directory of the autosave file exists.
 */
static void autosave_ensure_parent_exists() {

	gchar parent_buf[MAX_FILE_PATH_SIZE];
	strcpy(parent_buf, auto_save_data.filename);
	const gchar* parent = dirname(parent_buf);
	const gint res = g_mkdir_with_parents(parent, 0700);
	if (res != 0) {
		perror("Error: Failed to create auto-save dir");
	}
}

static void autosave_try_save(GtkWidget *view) {

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	if (gtk_text_buffer_get_modified(buffer)) {
		// store old/current auto-save filename
		gchar old_filename[MAX_FILE_PATH_SIZE];
		strcpy(old_filename, auto_save_data.filename);

		const gboolean uses_cache_dir = autosave_generate_filename(buffer);
		if (uses_cache_dir) {
			autosave_ensure_parent_exists();
		}

		// perform the auto-save
		FileInfo auto_save_file_info = { auto_save_data.filename, pub->fi->charset, pub->fi->charset_flag, pub->fi->lineend };
		/*const gint save_err = */file_save_real(view, &auto_save_file_info);
		gtk_text_buffer_set_modified(buffer, TRUE);
		autosave_reset_num_changes();
		//g_print("l3afpad: auto-saved to file: '%s'\n", auto_save_data.filename);

		if (strcmp(old_filename, auto_save_data.filename) != 0) {
			autosave_delete_file(old_filename);
		}
	}
}

static gboolean idle_handler(GtkWidget *view) {

	// mark this timer as done
	auto_save_data.timer_id = 0;

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
	auto_save_data.changes++;
	if (auto_save) {
		if (auto_save_data.timer_id > 0) {
			g_source_remove(auto_save_data.timer_id);
			auto_save_data.timer_id = 0;
		}
		if (auto_save_data.changes >= auto_save_immediate_changes) {
			autosave_try_save(view);
		} else {
			auto_save_data.timer_id = g_timeout_add(auto_save_timer, (GSourceFunc) time_handler, view);
		}
	}
}

static void autosave_kill_timer()
{
	if (auto_save_data.timer_id > 0) {
		g_source_remove(auto_save_data.timer_id);
		auto_save_data.timer_id = 0;
	}
}

void autosave_cb_file_saved(gchar *filename)
{
	autosave_kill_timer();
	if (strcmp(filename, auto_save_data.filename) != 0) {
		// not saving to our temporary/auto-save file
		autosave_discard_temp_file();
	}
}

void autosave_discard_temp_file()
{
	autosave_kill_timer();
	autosave_delete_file(auto_save_data.filename);
	AutoSaveData_init(&auto_save_data);
}

