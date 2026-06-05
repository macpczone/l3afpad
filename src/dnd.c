#include "l3afpad.h"
#include <string.h>

#define DV(x)

enum {
	TARGET_UTF8_STRING,
	TARGET_COMPOUND_TEXT,
	TARGET_PLAIN,
	TARGET_URI_LIST,
};

static GtkTargetEntry drag_types[] =
{
	{ "UTF8_STRING", 0, TARGET_UTF8_STRING },
	{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
	{ "text/plain", 0, TARGET_PLAIN },
	{ "text/uri-list", 0, TARGET_URI_LIST }
};

static gint n_drag_types = G_N_ELEMENTS(drag_types);

static void dnd_open_first_file(gchar *filename)
{
	FileInfo *fi;

	if (check_text_modification())
		return;

	fi = g_malloc(sizeof(FileInfo));
	fi->filename = g_strdup(filename);
	fi->charset = pub->fi->charset_flag ? g_strdup(pub->fi->charset) : NULL;
	fi->charset_flag = pub->fi->charset_flag;
	fi->lineend = LF;

	if (file_open_real(pub->mw->view, fi)) {
		g_free(fi);
	} else {
		g_free(pub->fi);
		pub->fi = fi;
		undo_clear_all(pub->mw->buffer);
		set_main_window_title();
	}
}

static void dnd_drag_data_received_handler(GtkWidget *widget,
	GdkDragContext *context, gint x, gint y,
	GtkSelectionData *selection_data, guint info, guint time)
{
	static guint last_time = 0;
	gchar **files;
	gchar *filename;
	gchar *comline;
	gint i = 0, j = 0;
	
#ifdef ENABLE_CSDI
	j = 1;
#endif

	if (time == last_time) {
		last_time = 0;
		g_signal_stop_emission_by_name(widget, "drag_data_received");
		return;
	}
	last_time = time;

	if (gtk_selection_data_get_length(selection_data) > 0 && info == TARGET_URI_LIST) {
		files = g_uri_list_extract_uris((const gchar *)gtk_selection_data_get_data(selection_data));
		
		while (files[i]) {
			filename = g_filename_from_uri(files[i], NULL, NULL);
			if (filename) {
				if (i + j == 0) {
					dnd_open_first_file(filename);
				} else {
					if (i + j == 1)
						save_config_file();
					
					comline = g_strdup_printf("%s \"%s\"", PACKAGE, filename);
					g_spawn_command_line_async(comline, NULL);
					g_free(comline);
				}
				g_free(filename);
			}
			i++;
		}
		g_strfreev(files);
		gtk_drag_finish(context, TRUE, FALSE, time);
	} else {
		clear_current_keyval();
		undo_set_sequency(FALSE);
		if (info == TARGET_UTF8_STRING) {
			undo_set_sequency_reserve();
			gdk_drag_status(context, GDK_ACTION_MOVE, time);
		}
	}
}

static gboolean dnd_drag_motion_handler(GtkWidget *widget,
	GdkDragContext *context, gint x, gint y, guint time)
{
	GList *targets = gdk_drag_context_list_targets(context);
	gboolean flag = FALSE;

	while (targets) {
		gchar *name = gdk_atom_name(GDK_POINTER_TO_ATOM(targets->data));
		if (g_ascii_strcasecmp(name, "text/uri-list") == 0)
			flag = TRUE;
		g_free(name);
		targets = targets->next;
	}

	if (flag)
		gdk_drag_status(context, GDK_ACTION_COPY, time);

	return flag;
}

void dnd_init(GtkWidget *widget)
{
	gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL,
		drag_types, n_drag_types, GDK_ACTION_COPY);
	
	g_signal_connect(G_OBJECT(widget), "drag_data_received",
		G_CALLBACK(dnd_drag_data_received_handler), NULL);
	g_signal_connect(G_OBJECT(widget), "drag_motion",
		G_CALLBACK(dnd_drag_motion_handler), NULL);
}