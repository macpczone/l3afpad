/*
 *  L3afpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_selection_bound(GtkTextBuffer *buffer, gint start, gint end)
{
	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
	if (end < 0)
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
	else
		gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
	gtk_text_buffer_place_cursor(buffer, &end_iter);
	gtk_text_buffer_move_mark_by_name(buffer, "selection_bound", &start_iter);
}

void on_file_new(void)
{
	gchar *comline;
	gchar *option = "";

	save_config_file();
	if (pub->fi->charset_flag)
		option = g_strdup_printf(" --codeset=%s", pub->fi->charset);
	
	comline = g_strdup_printf("%s%s", PACKAGE, option);
	g_spawn_command_line_async(comline, NULL);
	
	if (pub->fi->charset_flag)
		g_free(option);
	g_free(comline);
}

void on_file_open(void)
{
#ifdef ENABLE_CSDI
	FileInfo *fi = get_fileinfo_from_selector(pub->fi, OPEN);
	if (fi) {
		gchar *option = g_strdup_printf("--codeset=%s ", fi->charset);
		gchar *comline = g_strdup_printf("%s %s%s", PACKAGE, fi->charset ? option : "", fi->filename);
		
		save_config_file();
		recent_add_file(fi->filename);
		g_spawn_command_line_async(comline, NULL);
		
		g_free(option);
		g_free(comline);
		g_free(fi);
	}
#else
	if (check_text_modification())
		return;

	FileInfo *fi = get_fileinfo_from_selector(pub->fi, OPEN);
	if (fi) {
		if (file_open_real(pub->mw->view, fi)) {
			g_free(fi);
		} else {
			g_free(pub->fi);
			pub->fi = fi;
			undo_clear_all(pub->mw->buffer);
			recent_add_file(fi->filename);
			force_call_cb_modified_changed(pub->mw->view);
		}
	}
#endif
}

void on_file_open_recent(gpointer data)
{
	gchar *filename = (gchar *)data;
	FileInfo *fi;

	if (!filename || strlen(filename) == 0)
		return;

	if (check_text_modification())
		return;

	fi = g_malloc0(sizeof(FileInfo));
	fi->filename = g_strdup(filename);
	fi->charset = pub->fi->charset_flag ? g_strdup(pub->fi->charset) : NULL;
	fi->charset_flag = pub->fi->charset_flag;
	fi->lineend = LF;

	if (file_open_real(pub->mw->view, fi)) {
		g_free(fi->filename);
		g_free(fi->charset);
		g_free(fi);
	} else {
		g_free(pub->fi);
		pub->fi = fi;
		undo_clear_all(pub->mw->buffer);
		recent_add_file(fi->filename);
		force_call_cb_modified_changed(pub->mw->view);
	}
}

gint on_file_save(void)
{
	if (pub->fi->filename == NULL || !check_file_writable(pub->fi->filename))
		return on_file_save_as();

	if (file_save_real(pub->mw->view, pub->fi))
		return -1;

	recent_add_file(pub->fi->filename);
	force_call_cb_modified_changed(pub->mw->view);
	return 0;
}

gint on_file_save_as(void)
{
	FileInfo *fi = get_fileinfo_from_selector(pub->fi, SAVE);
	if (!fi)
		return -1;

	if (file_save_real(pub->mw->view, fi)) {
		g_free(fi);
		return -1;
	}

	g_free(pub->fi);
	pub->fi = fi;
	undo_clear_all(pub->mw->buffer);
	recent_add_file(pub->fi->filename);
	force_call_cb_modified_changed(pub->mw->view);
	return 0;
}

#if ENABLE_STATISTICS
void on_file_stats(void)
{
	gchar *stats = file_stats(pub->mw->view, pub->fi);
	GtkWidget *msg = gtk_message_dialog_new_with_markup(
		GTK_WINDOW(pub->mw->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"<b>%s</b>", _("Statistics"));

	gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(msg), "<i>%s</i>", stats);
	gtk_window_set_title(GTK_WINDOW(msg), pub->fi->filename ? pub->fi->filename : _("Untitled"));
	
	gtk_dialog_run(GTK_DIALOG(msg));
	gtk_widget_destroy(msg);
	g_free(stats);
}
#endif

#if ENABLE_PRINT
void on_file_print_preview(void)
{
	create_gtkprint_preview_session(GTK_TEXT_VIEW(pub->mw->view),
		get_file_basename(pub->fi->filename, FALSE));
}

void on_file_print(void)
{
	create_gtkprint_session(GTK_TEXT_VIEW(pub->mw->view),
		get_file_basename(pub->fi->filename, FALSE));
}
#endif

void on_file_close(void)
{
	if (!check_text_modification()) {
		force_block_cb_modified_changed(pub->mw->view);
		gtk_text_buffer_set_text(pub->mw->buffer, "", 0);
		gtk_text_buffer_set_modified(pub->mw->buffer, FALSE);
		
		g_free(pub->fi->filename);
		pub->fi->filename = NULL;
		g_free(pub->fi->charset);
		pub->fi->charset = NULL;
		
		pub->fi->charset_flag = FALSE;
		pub->fi->lineend = LF;
		undo_clear_all(pub->mw->buffer);
		
		force_call_cb_modified_changed(pub->mw->view);
		force_unblock_cb_modified_changed(pub->mw->view);
	}
}

void on_file_quit(void)
{
	if (!check_text_modification()) {
		save_config_file();
		gtk_main_quit();
	}
}

void on_edit_undo(void) { undo_undo(pub->mw->buffer); }
void on_edit_redo(void) { undo_redo(pub->mw->buffer); }
void on_edit_cut(void) { g_signal_emit_by_name(pub->mw->view, "cut-clipboard"); }
void on_edit_copy(void) { g_signal_emit_by_name(pub->mw->view, "copy-clipboard"); }
void on_edit_paste(void) { g_signal_emit_by_name(pub->mw->view, "paste-clipboard"); }
void on_edit_delete(void) { gtk_text_buffer_delete_selection(pub->mw->buffer, TRUE, TRUE); }

void on_edit_select_all(void)
{
	set_selection_bound(pub->mw->buffer, 0, -1);
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static void activate_quick_find(void)
{
	static gboolean flag = FALSE;
	if (!flag) {
		gtk_widget_set_sensitive(gtk_ui_manager_get_widget(pub->mw->menubar, "/M/Search/FindNext"), TRUE);
		gtk_widget_set_sensitive(gtk_ui_manager_get_widget(pub->mw->menubar, "/M/Search/FindPrevious"), TRUE);
		flag = TRUE;
	}
}

void on_search_find(void)
{
	if (run_dialog_search(pub->mw->view, 0) == GTK_RESPONSE_OK)
		activate_quick_find();
}

void on_search_find_next(void) { document_search_real(pub->mw->view, 1); }
void on_search_find_previous(void) { document_search_real(pub->mw->view, -1); }

void on_search_replace(void)
{
	if (run_dialog_search(pub->mw->view, 1) == GTK_RESPONSE_OK)
		activate_quick_find();
}

void on_search_jump_to(void) { run_dialog_jump_to(pub->mw->view); }
void on_option_font(void) { change_text_font_by_selector(pub->mw->view); }

void on_option_word_wrap(void)
{
	gboolean state;
	state = gtk_toggle_action_get_active(
		GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/WordWrap")));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pub->mw->view), state ? GTK_WRAP_WORD_CHAR : GTK_WRAP_NONE);
}

void on_option_line_numbers(void)
{
	gboolean state;
	state = gtk_toggle_action_get_active(
		GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/LineNumbers")));
	show_line_numbers(pub->mw->view, state);
}

void on_option_always_on_top(void)
{
	static gboolean flag = FALSE;
	flag = !flag;
	gtk_window_set_keep_above(GTK_WINDOW(pub->mw->window), flag);
}

void on_option_auto_indent(void)
{
	gboolean state;
	state = gtk_toggle_action_get_active(
		GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/AutoIndent")));
	indent_set_state(state);
}

void on_option_auto_save(void)
{
	gboolean state;

	state = gtk_toggle_action_get_active(
		GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/AutoSave")));
	autosave_set_state(state);
}

void on_option_auto_save_same_dir(void)
{
	gboolean state;

	state = gtk_toggle_action_get_active(
		GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/AutoSaveSameDir")));
	autosave_set_same_dir(state);
}

void on_option_auto_save_chars(void)
{
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	GtkWidget *dialog;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkAdjustment *spinner_adj;
	guint current;

	current = autosave_get_immediate_changes();

	dialog = gtk_dialog_new_with_buttons(_("Auto Save Chars"),
		GTK_WINDOW(pub->mw->window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		"gtk-cancel", GTK_RESPONSE_CANCEL,
		NULL);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 4);
	button = gtk_button_new_with_label(_("_OK"));
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	table = gtk_table_new(1, 2, FALSE);
	 gtk_table_set_col_spacings(GTK_TABLE(table), 8);
	 gtk_container_set_border_width(GTK_CONTAINER(table), 8);
	 gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic(_("Changes to _trigger auto-save:"));
	spinner_adj = gtk_adjustment_new(current, 1, 10000, 1, 1, 0);
	spinner = gtk_spin_button_new(spinner_adj, 1, 0);
	 gtk_entry_set_width_chars(GTK_ENTRY(spinner), 8);
	 gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinner);
	 gtk_entry_set_activates_default(GTK_ENTRY(spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), spinner, 1, 2, 0, 1);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(dialog);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		guint value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner));
		autosave_set_immediate_changes(value);
	}
	gtk_widget_destroy(dialog);
G_GNUC_END_IGNORE_DEPRECATIONS
}

void on_option_auto_save_seconds(void)
{
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	GtkWidget *dialog;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkAdjustment *spinner_adj;
	guint current;

	current = autosave_get_timer() / 1000;

	dialog = gtk_dialog_new_with_buttons(_("Auto Save Seconds"),
		GTK_WINDOW(pub->mw->window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		"gtk-cancel", GTK_RESPONSE_CANCEL,
		NULL);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 4);
	button = gtk_button_new_with_label(_("_OK"));
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	table = gtk_table_new(1, 2, FALSE);
	 gtk_table_set_col_spacings(GTK_TABLE(table), 8);
	 gtk_container_set_border_width(GTK_CONTAINER(table), 8);
	 gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic(_("Seconds before _auto-save:"));
	spinner_adj = gtk_adjustment_new(current, 1, 300, 1, 1, 0);
	spinner = gtk_spin_button_new(spinner_adj, 1, 0);
	 gtk_entry_set_width_chars(GTK_ENTRY(spinner), 8);
	 gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinner);
	 gtk_entry_set_activates_default(GTK_ENTRY(spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), spinner, 1, 2, 0, 1);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(dialog);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		guint value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner));
		autosave_set_timer(value * 1000);
	}
	gtk_widget_destroy(dialog);
G_GNUC_END_IGNORE_DEPRECATIONS
}

void on_option_focus_save(void)
{
	// gboolean state;

	// state = gtk_toggle_action_get_active(
	// 	GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(pub->mw->menubar, "/M/Options/SaveOnFocus")));
	// indent_set_state(state);
}

void on_option_toggle_menubar(void)
{
	GtkWidget *menubar_widget;
	menubar_widget = gtk_ui_manager_get_widget(pub->mw->menubar, "/M");
	if (menubar_widget)
		gtk_widget_set_visible(menubar_widget, !gtk_widget_get_visible(menubar_widget));
G_GNUC_END_IGNORE_DEPRECATIONS
}

void on_help_about(void)
{
	const gchar *copyright = "Copyright \xc2\xa9 2004-2014 Tarot Osuji et al.\nCopyright \xc2\xa9 2025 Nube";
	const gchar *authors[] = {
		"Tarot Osuji <tarot@sdf.org>",
		"Wen-Yen Chuang <caleb@calno.com>",
		"Yoo, Taik-Yon <jaagar@gmail.com>",
		"Steven Honeyman <stevenhoneyman@gmail.com>",
		"Nube <nubesu@tuta.io>",
		"Michael Campbell <mikeai@bnm.one>",
		NULL
	};
	const gchar *artists[] = {
		"Lapo Calamandrei <calamandrei@gmail.com>",
		"Jack Gandy <scionicspectre@gmail.com>",
		NULL
	};

	gtk_show_about_dialog(GTK_WINDOW(pub->mw->window),
		"program-name", "Lɜafpad",
		"version", PACKAGE_VERSION,
		"copyright", copyright,
		"comments", _("GTK+ based simple text editor"),
		"authors", authors,
		"artists", artists,
		"logo-icon-name", PACKAGE,
		NULL);
}