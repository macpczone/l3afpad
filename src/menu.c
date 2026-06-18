/*
 * L3afpad - GTK+ based simple text editor
 * Copyright (C) 2004-2005 Tarot Osuji
 * Copyright (C)      2011 Wen-Yen Chuang <caleb AT calno DOT com>
 * Copyright (C)      2025 Nube <nubesu AT tuta DOT io>
 */

#include "l3afpad.h"
#include <gdk/gdkkeysyms.h>

#define accel_group gtk_ui_manager_get_accel_group(ifactory)

static GtkWidget *menu_item_save;
static GtkWidget *menu_item_cut;
static GtkWidget *menu_item_copy;
static GtkWidget *menu_item_paste;
static GtkWidget *menu_item_delete;

static GtkActionEntry menu_items[] =
{
	{ "File", NULL, N_("_File"), NULL, NULL, NULL },
	{ "Edit", NULL, N_("_Edit"), NULL, NULL, NULL },
	{ "Search", NULL, N_("_Search"), NULL, NULL, NULL },
	{ "Options", NULL, N_("_Options"), NULL, NULL, NULL },
	{ "Help", NULL, N_("_Help"), NULL, NULL, NULL },
	{ "New", "document-new", N_("_New"), "<control>N", NULL, G_CALLBACK(on_file_new) },
	{ "Open", "document-open", N_("_Open..."), "<control>O", NULL, G_CALLBACK(on_file_open) },
	{ "RecentFiles", NULL, N_("Recent _Files"), NULL, NULL, NULL },
	{ "Save", "document-save", N_("_Save"), "<control>S", NULL, G_CALLBACK(on_file_save) },
	{ "SaveAs", "document-save-as", N_("Save _As..."), "<shift><control>S", NULL, G_CALLBACK(on_file_save_as) },
#if ENABLE_STATISTICS
	{ "Statistics", "document-properties", N_("Sta_tistics..."), NULL, NULL, G_CALLBACK(on_file_stats) },
#endif
#if ENABLE_PRINT
	{ "PrintPreview", "document-print-preview", N_("Print Pre_view"), "<shift><control>P", NULL, G_CALLBACK(on_file_print_preview) },
	{ "Print", "document-print", N_("_Print..."), "<control>P", NULL, G_CALLBACK(on_file_print) },
#endif
	{ "Quit", "application-exit", N_("_Quit"), "<control>Q", NULL, G_CALLBACK(on_file_quit) },
	{ "Undo", "edit-undo", N_("_Undo"), "<control>Z", NULL, G_CALLBACK(on_edit_undo) },
	{ "Redo", "edit-redo", N_("_Redo"), "<shift><control>Z", NULL, G_CALLBACK(on_edit_redo) },
	{ "Cut", "edit-cut", N_("Cu_t"), "<control>X", NULL, G_CALLBACK(on_edit_cut) },
	{ "Copy", "edit-copy", N_("_Copy"), "<control>C", NULL, G_CALLBACK(on_edit_copy) },
	{ "Paste", "edit-paste", N_("_Paste"), "<control>V", NULL, G_CALLBACK(on_edit_paste) },
	{ "Delete", "edit-delete", N_("_Delete"), NULL, NULL, G_CALLBACK(on_edit_delete) },
	{ "SelectAll", NULL, N_("Select _All"), "<control>A", NULL, G_CALLBACK(on_edit_select_all) },
	{ "Find", "edit-find", N_("_Find..."), "<control>F", NULL, G_CALLBACK(on_search_find) },
	{ "FindNext", NULL, N_("Find _Next"), "<control>G", NULL, G_CALLBACK(on_search_find_next) },
	{ "FindPrevious", NULL, N_("Find _Previous"), "<shift><control>G", NULL, G_CALLBACK(on_search_find_previous) },
	{ "Replace", "edit-find-replace", N_("_Replace..."), "<control>H", NULL, G_CALLBACK(on_search_replace) },
	{ "JumpTo", "go-jump", N_("_Jump To..."), "<control>J", NULL, G_CALLBACK(on_search_jump_to) },
	{ "Font", "font-select", N_("_Font..."), NULL, NULL, G_CALLBACK(on_option_font) },
	{ "About", "help-about", N_("_About"), NULL, NULL, G_CALLBACK(on_help_about) },
};

static guint nmenu_items = G_N_ELEMENTS (menu_items);

static GtkToggleActionEntry toggle_entries[] =
{
	{ "WordWrap", NULL, N_("_Word Wrap"), NULL, NULL, G_CALLBACK (on_option_word_wrap), FALSE },
	{ "LineNumbers", NULL, N_("_Line Numbers"), NULL, NULL, G_CALLBACK (on_option_line_numbers), FALSE },
	{ "AutoIndent", NULL, N_("_Auto Indent"), NULL, NULL, G_CALLBACK (on_option_auto_indent), FALSE },
	{ "AutoSave", NULL, N_("Auto Save"), NULL, NULL, G_CALLBACK (on_option_auto_save), FALSE },
	{ "AutoSaveSameDir", NULL, N_("Auto Save Same Dir"), NULL, NULL, G_CALLBACK (on_option_auto_save_same_dir), FALSE },
	{ "SaveOnFocus", NULL, N_("Auto _save on lost focus"), NULL, NULL, G_CALLBACK (on_option_focus_save), FALSE },
};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);

static const gchar *ui_info =
"<ui>"
"  <menubar name='M'>"
"    <menu action='File'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menu action='RecentFiles'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <separator/>"
#if ENABLE_STATISTICS
"      <menuitem action='Statistics'/>"
#endif
#if ENABLE_PRINT
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"      <separator/>"
#endif
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='Edit'>"
"      <menuitem action='Undo'/>"
"      <menuitem action='Redo'/>"
"      <separator/>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Paste'/>"
"      <menuitem action='Delete'/>"
"      <separator/>"
"      <menuitem action='SelectAll'/>"
"    </menu>"
"    <menu action='Search'>"
"      <menuitem action='Find'/>"
"      <menuitem action='FindNext'/>"
"      <menuitem action='FindPrevious'/>"
"      <menuitem action='Replace'/>"
"      <separator/>"
"      <menuitem action='JumpTo'/>"
"    </menu>"
"    <menu action='Options'>"
"      <menuitem action='Font'/>"
"      <menuitem action='WordWrap'/>"
"      <menuitem action='LineNumbers'/>"
"      <separator/>"
"      <menuitem action='AutoIndent'/>"
"      <separator/>"
"      <menuitem action='AutoSave'/>"
"      <menuitem action='AutoSaveSameDir'/>"
"      <menuitem action='SaveOnFocus'/>"
"    </menu>"
"    <menu action='Help'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static gchar *menu_translate(const gchar *path, gpointer data)
{
	return _(path);
}

void menu_sensitivity_from_modified_flag(gboolean is_text_modified)
{
	gtk_widget_set_sensitive(menu_item_save, is_text_modified);
}

void menu_sensitivity_from_selection_bound(gboolean is_bound_exist)
{
	gtk_widget_set_sensitive(menu_item_cut,    is_bound_exist);
	gtk_widget_set_sensitive(menu_item_copy,   is_bound_exist);
	gtk_widget_set_sensitive(menu_item_delete, is_bound_exist);
}

void menu_sensitivity_from_clipboard(void)
{
	gtk_widget_set_sensitive(menu_item_paste,
		gtk_clipboard_wait_is_text_available(
			gtk_clipboard_get(GDK_SELECTION_CLIPBOARD)));
}

GtkUIManager *create_menu_bar(GtkWidget *window)
{
	GtkUIManager *ifactory;

	ifactory = gtk_ui_manager_new();
	GtkActionGroup *actions = gtk_action_group_new("Actions");
	gtk_action_group_set_translate_func(actions, menu_translate, NULL, NULL);
	gtk_action_group_add_actions(actions, menu_items, nmenu_items, NULL);
	gtk_action_group_add_toggle_actions (actions, toggle_entries, n_toggle_entries, NULL);
	gtk_ui_manager_insert_action_group(ifactory, actions, 0);
	g_object_unref(actions);
	gtk_ui_manager_add_ui_from_string(ifactory, ui_info, -1, NULL);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	gtk_accel_group_connect(
		accel_group, GDK_W, GDK_CONTROL_MASK, 0,
		g_cclosure_new_swap(G_CALLBACK(on_file_close), NULL, NULL));
	gtk_accel_group_connect(
		accel_group, GDK_T, GDK_CONTROL_MASK, 0,
		g_cclosure_new_swap(G_CALLBACK(on_option_always_on_top), NULL, NULL));
	gtk_accel_group_connect(
		accel_group, GDK_M, GDK_CONTROL_MASK, 0,
		g_cclosure_new_swap(G_CALLBACK(on_option_toggle_menubar), NULL, NULL));

	gtk_widget_add_accelerator(
		gtk_ui_manager_get_widget(ifactory, "/M/Edit/Redo"),
		"activate", accel_group, GDK_Y, GDK_CONTROL_MASK, 0);
	gtk_widget_add_accelerator(
		gtk_ui_manager_get_widget(ifactory, "/M/Search/FindNext"),
		"activate", accel_group, GDK_F3, 0, 0);
	gtk_widget_add_accelerator(
		gtk_ui_manager_get_widget(ifactory, "/M/Search/FindPrevious"),
		"activate", accel_group, GDK_F3, GDK_SHIFT_MASK, 0);
	gtk_widget_add_accelerator(
		gtk_ui_manager_get_widget(ifactory, "/M/Search/Replace"),
		"activate", accel_group, GDK_R, GDK_CONTROL_MASK, 0);

	gtk_widget_set_sensitive(
		gtk_ui_manager_get_widget(ifactory, "/M/Search/FindNext"),
		FALSE);
	gtk_widget_set_sensitive(
		gtk_ui_manager_get_widget(ifactory, "/M/Search/FindPrevious"),
		FALSE);

	menu_item_save   = gtk_ui_manager_get_widget(ifactory, "/M/File/Save");
	menu_item_cut    = gtk_ui_manager_get_widget(ifactory, "/M/Edit/Cut");
	menu_item_copy   = gtk_ui_manager_get_widget(ifactory, "/M/Edit/Copy");
	menu_item_paste  = gtk_ui_manager_get_widget(ifactory, "/M/Edit/Paste");
	menu_item_delete = gtk_ui_manager_get_widget(ifactory, "/M/Edit/Delete");
	menu_sensitivity_from_selection_bound(FALSE);

	GtkWidget *recent_item = gtk_ui_manager_get_widget(ifactory, "/M/File/RecentFiles");
	if (recent_item) {
		GtkWidget *recent_menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(recent_item), recent_menu);

		GList *recent_list = recent_get_list();
		gint i = 1;

		for (GList *l = recent_list; l && i <= MAX_RECENT_FILES; l = l->next, i++) {
			gchar *basename = g_path_get_basename((gchar *)l->data);
			gchar *label = g_strdup_printf("%d. %s", i, basename);
			GtkWidget *item = gtk_menu_item_new_with_label(label);
			
			g_signal_connect(G_OBJECT(item), "activate",
				G_CALLBACK(on_file_open_recent), l->data);
			
			gtk_menu_shell_append(GTK_MENU_SHELL(recent_menu), item);
			gtk_widget_show(item);
			
			g_free(label);
			g_free(basename);
		}
	}

	return ifactory;
}