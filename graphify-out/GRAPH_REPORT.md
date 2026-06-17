# Graph Report - .  (2026-06-16)

## Corpus Check
- Corpus is ~27,165 words - fits in a single context window. You may not need a graph.

## Summary
- 387 nodes · 794 edges · 38 communities (37 shown, 1 thin omitted)
- Extraction: 80% EXTRACTED · 20% INFERRED · 0% AMBIGUOUS · INFERRED: 156 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Edit Callbacks|Edit Callbacks]]
- [[_COMMUNITY_Dialog System|Dialog System]]
- [[_COMMUNITY_Menu System|Menu System]]
- [[_COMMUNITY_Emacs & Indent|Emacs & Indent]]
- [[_COMMUNITY_Auto-Save|Auto-Save]]
- [[_COMMUNITY_Project Documentation|Project Documentation]]
- [[_COMMUNITY_Undo System|Undo System]]
- [[_COMMUNITY_Encoding|Encoding]]
- [[_COMMUNITY_Font Selector|Font Selector]]
- [[_COMMUNITY_Text Iterators|Text Iterators]]
- [[_COMMUNITY_File IO|File I/O]]
- [[_COMMUNITY_Printing|Printing]]
- [[_COMMUNITY_Line Numbers|Line Numbers]]
- [[_COMMUNITY_Font Management|Font Management]]
- [[_COMMUNITY_Drag & Drop|Drag & Drop]]
- [[_COMMUNITY_Application Icons|Application Icons]]
- [[_COMMUNITY_Build System|Build System]]

## God Nodes (most connected - your core abstractions)
1. `main()` - 23 edges
2. `file_open_real()` - 16 edges
3. `document_search_real()` - 14 edges
4. `GtkTextBuffer` - 14 edges
5. `L3afpad — Agent Guide` - 13 edges
6. `document_replace_real()` - 12 edges
7. `dnd_drag_data_received_handler()` - 11 edges
8. `detect_charset()` - 11 edges
9. `file_save_real()` - 11 edges
10. `undo_create_undo_info()` - 11 edges

## Surprising Connections (you probably didn't know these)
- `on_search_find_next()` --calls--> `document_search_real()`  [INFERRED]
  src/callback.c → src/search.c
- `on_search_find_previous()` --calls--> `document_search_real()`  [INFERRED]
  src/callback.c → src/search.c
- `on_search_jump_to()` --calls--> `run_dialog_jump_to()`  [INFERRED]
  src/callback.c → src/search.c
- `on_option_auto_save()` --calls--> `autosave_set_state()`  [INFERRED]
  src/callback.c → src/autosave.c
- `main()` --calls--> `autosave_set_state()`  [INFERRED]
  src/main.c → src/autosave.c

## Import Cycles
- None detected.

## Communities (38 total, 1 thin omitted)

### Community 0 - "Edit Callbacks"
Cohesion: 0.07
Nodes (33): activate_quick_find(), gint, gpointer, GtkTextBuffer, on_edit_redo(), on_edit_select_all(), on_edit_undo(), on_file_close() (+25 more)

### Community 1 - "Dialog System"
Cohesion: 0.10
Nodes (34): GtkMessageType, gchar, gint, GtkWidget, create_dialog_message_question(), run_dialog_message(), run_dialog_message_question(), gboolean (+26 more)

### Community 2 - "Menu System"
Cohesion: 0.10
Nodes (32): GdkEventButton, GdkEventFocus, gdouble, MainWin, gboolean, gchar, gpointer, GtkUIManager (+24 more)

### Community 3 - "Emacs & Indent"
Cohesion: 0.11
Nodes (31): Conf, GtkWindow, on_option_auto_indent(), gboolean, GtkUIManager, check_emacs_key_theme(), gboolean, gchar (+23 more)

### Community 4 - "Auto-Save"
Cohesion: 0.13
Nodes (28): AutoSaveData, autosave_cb_buffer_changed(), autosave_cb_file_saved(), autosave_delete_file(), autosave_discard_temp_file(), autosave_ensure_parent_exists(), autosave_generate_filename(), autosave_get_immediate_changes() (+20 more)

### Community 5 - "Project Documentation"
Cohesion: 0.08
Nodes (24): Adding a new configure option, Adding a new menu item, Adding a new source file, Architecture, Build Steps, Build System, Coding Conventions, Common Tasks (+16 more)

### Community 6 - "Undo System"
Cohesion: 0.20
Nodes (24): gboolean, gchar, gint, GList, GtkTextBuffer, GtkTextIter, GtkWidget, cb_begin_user_action() (+16 more)

### Community 7 - "Encoding"
Cohesion: 0.26
Nodes (18): EncArray, gboolean, gchar, gint, guint, convert_line_ending(), convert_line_ending_to_lf(), detect_charset() (+10 more)

### Community 8 - "Font Selector"
Cohesion: 0.24
Nodes (17): CharsetTable, GtkComboBox, FileInfo, gboolean, gchar, gint, GtkWidget, cb_select_charset() (+9 more)

### Community 9 - "Text Iterators"
Cohesion: 0.36
Nodes (16): gssize, GtkSourceSearchFlags, backward_lines_match(), gboolean, gchar, gint, GtkTextIter, forward_chars_with_skipping() (+8 more)

### Community 10 - "File I/O"
Cohesion: 0.24
Nodes (16): on_file_print(), on_file_print_preview(), on_file_stats(), FileInfo, gboolean, gchar, gint, GtkWidget (+8 more)

### Community 11 - "Printing"
Cohesion: 0.29
Nodes (15): GtkPrintContext, GtkPrintOperation, PangoTabArray, gchar, gint, gpointer, GtkTextView, cb_begin_print() (+7 more)

### Community 12 - "Line Numbers"
Cohesion: 0.25
Nodes (14): cairo_t, GArray, PangoAttribute, on_option_line_numbers(), gboolean, gint, GtkTextView, GtkWidget (+6 more)

### Community 13 - "Font Management"
Cohesion: 0.20
Nodes (9): on_option_font(), gchar, GtkWidget, change_text_font_by_selector(), get_font_name_by_selector(), set_text_font_by_name(), GList, recent_get_list() (+1 more)

### Community 14 - "Drag & Drop"
Cohesion: 0.24
Nodes (11): GdkDragContext, GtkSelectionData, gboolean, gint, GtkWidget, guint, dnd_drag_data_received_handler(), dnd_drag_motion_handler() (+3 more)

### Community 15 - "Application Icons"
Cohesion: 0.50
Nodes (4): L3afpad Alternative Icons, L3afpad Sized Icons (16x16-256x256), L3afpad Application Icon, L3afpad SVG Source Icon

## Knowledge Gaps
- **66 isolated node(s):** `autogen.sh script`, `AutoSaveData`, `GtkTextBuffer`, `gpointer`, `GtkMessageType` (+61 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `main()` connect `Emacs & Indent` to `Dialog System`, `Menu System`, `Auto-Save`, `Undo System`, `Encoding`, `Font Management`, `Drag & Drop`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Why does `file_open_real()` connect `Encoding` to `Edit Callbacks`, `Dialog System`, `Menu System`, `Emacs & Indent`, `File I/O`?**
  _High betweenness centrality (0.068) - this node is a cross-community bridge._
- **Why does `scroll_to_cursor()` connect `Menu System` to `Dialog System`, `Emacs & Indent`, `Undo System`?**
  _High betweenness centrality (0.037) - this node is a cross-community bridge._
- **Are the 18 inferred relationships involving `main()` (e.g. with `autosave_set_immediate_changes()` and `autosave_set_same_dir()`) actually correct?**
  _`main()` has 18 INFERRED edges - model-reasoned connections that need verification._
- **Are the 12 inferred relationships involving `file_open_real()` (e.g. with `on_file_open()` and `on_file_open_recent()`) actually correct?**
  _`file_open_real()` has 12 INFERRED edges - model-reasoned connections that need verification._
- **Are the 7 inferred relationships involving `document_search_real()` (e.g. with `on_search_find_next()` and `on_search_find_previous()`) actually correct?**
  _`document_search_real()` has 7 INFERRED edges - model-reasoned connections that need verification._
- **What connects `autogen.sh script`, `AutoSaveData`, `GtkTextBuffer` to the rest of the system?**
  _66 weakly-connected nodes found - possible documentation gaps or missing edges._