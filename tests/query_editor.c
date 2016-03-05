/*
 * Copyright (C) 2011-2015 Andrea Zagli <azagli@libero.it>
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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <gdaex.h>
#include <queryeditor.h>

GdaExQueryEditor *qe;

gchar *xmlfile;

GdaEx *gdaex;

GtkWidget *w;

GtkWidget *widget_qe;
GtkWidget *btn_destroy;
GtkWidget *btn_clean;
GtkWidget *btn_get_sql;
GtkWidget *btn_save_xml;
GtkWidget *btn_load_xml;
GtkWidget *btn_ok;

GtkWidget *vbox;

static void
do_query_editor ()
{
	GdaExQueryEditorField *field;

	qe = gdaex_query_editor_new (gdaex);

	if (xmlfile != NULL)
		{
			gdaex_query_editor_load_tables_from_file (qe, xmlfile, TRUE);
		}
	else
		{
			gdaex_query_editor_add_table (qe, "clients", "Clients");

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("id");
			field->name_visible = g_strdup ("ID");
			field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
			field->for_show = TRUE;
			field->always_showed = TRUE;
			field->for_where = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_EQUAL;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("name");
			field->name_visible = g_strdup ("Name");
			field->description = g_strdup ("The client's name");
			field->type = GDAEX_QE_FIELD_TYPE_TEXT;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_STARTS
			                              | GDAEX_QE_WHERE_TYPE_CONTAINS
			                              | GDAEX_QE_WHERE_TYPE_ENDS
			                              | GDAEX_QE_WHERE_TYPE_ISTARTS
			                              | GDAEX_QE_WHERE_TYPE_ICONTAINS
			                              | GDAEX_QE_WHERE_TYPE_IENDS;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("surname");
			field->name_visible = g_strdup ("Surname");
			field->description = g_strdup ("The client's surname");
			field->type = GDAEX_QE_FIELD_TYPE_TEXT;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_STRING
			                              | GDAEX_QE_WHERE_TYPE_IS_NULL;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("brithday");
			field->name_visible = g_strdup ("Birthday");
			field->description = g_strdup ("The client's birthday");
			field->type = GDAEX_QE_FIELD_TYPE_DATE;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_DATETIME
			                              | GDAEX_QE_WHERE_TYPE_IS_NULL;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("age");
			field->name_visible = g_strdup ("Age");
			field->description = g_strdup ("The client's age");
			field->alias = g_strdup ("client_age");
			field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_NUMBER;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("datetime");
			field->name_visible = g_strdup ("DateTime");
			field->description = g_strdup ("???");
			field->type = GDAEX_QE_FIELD_TYPE_DATETIME;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_DATETIME
	                              | GDAEX_QE_WHERE_TYPE_IS_NULL;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("id_cities");
			field->name_visible = g_strdup ("City");
			field->description = g_strdup ("The client's city");
			field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->decode_table2 = g_strdup ("cities");
			field->decode_join_type = GDAEX_QE_JOIN_TYPE_LEFT;
			/*field->decode_fields1 = g_slist_append (field->decode_fields1, "id_cities");
			field->decode_fields2 = g_slist_append (field->decode_fields2, "id");*/
			field->decode_field2 = g_strdup ("id");
			field->decode_field_to_show = g_strdup ("name");
			field->decode_field_alias = g_strdup ("city_name");
			field->available_where_type = GDAEX_QE_WHERE_TYPE_EQUAL;
			gdaex_query_editor_table_add_field (qe, "clients", *field);
			g_free (field);

			gdaex_query_editor_add_table (qe, "orders", "Orders");

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("id");
			field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
			gdaex_query_editor_table_add_field (qe, "orders", *field);
			g_free (field);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("id_clients");
			field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
			gdaex_query_editor_table_add_field (qe, "orders", *field);
			g_free (field);

			gdaex_query_editor_add_relation (qe,
			                                 "clients", "orders",
			                                 GDAEX_QE_JOIN_TYPE_LEFT,
			                                 "id", "id_clients",
			                                 NULL);

			field = g_new0 (GdaExQueryEditorField, 1);
			field->name = g_strdup ("amount");
			field->name_visible = g_strdup ("Amount");
			field->type = GDAEX_QE_FIELD_TYPE_DOUBLE;
			field->for_show = TRUE;
			field->for_where = TRUE;
			field->for_order = TRUE;
			field->available_where_type = GDAEX_QE_WHERE_TYPE_NUMBER;
			gdaex_query_editor_table_add_field (qe, "orders", *field);
			g_free (field);
		}

	widget_qe = gdaex_query_editor_get_widget (qe);
	gtk_box_pack_start (GTK_BOX (vbox), widget_qe, TRUE, TRUE, 5);
}

static void
on_btn_destroy_clicked (GtkButton *button,
                      gpointer user_data)
{
	if (widget_qe != NULL)
		{
			g_object_unref (G_OBJECT (qe));
			widget_qe = NULL;
		}
	else
		{
			do_query_editor ();
		}
}

static void
on_btn_clean_clicked (GtkButton *button,
                      gpointer user_data)
{
	gdaex_query_editor_clean_choices (qe);
}

static void
on_btn_save_xml_clicked (GtkButton *button,
                      gpointer user_data)
{
	xmlDoc *doc;
	xmlNode *node;
	GtkWidget *dialog;

	doc = xmlNewDoc ("1.0");
	node = gdaex_query_editor_get_sql_as_xml (qe);
	xmlDocSetRootElement (doc, node);

	dialog = gtk_file_chooser_dialog_new ("Save xml to...",
	                                      GTK_WINDOW (w),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      "gtk-cancel", GTK_RESPONSE_CANCEL,
	                                      "gtk-save", GTK_RESPONSE_ACCEPT,
	                                      NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			gchar *filename;
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			xmlSaveFormatFile (filename, doc, 2);
			g_free (filename);
		}
	gtk_widget_destroy (dialog);
}

static void
on_btn_load_xml_clicked (GtkButton *button,
                      gpointer user_data)
{
	GtkWidget *dialog;
	xmlDoc *doc;
	xmlNode *node;

	dialog = gtk_file_chooser_dialog_new ("Load xml from...",
	                                      GTK_WINDOW (w),
	                                      GTK_FILE_CHOOSER_ACTION_OPEN,
	                                      "gtk-cancel", GTK_RESPONSE_CANCEL,
	                                      "gtk-open", GTK_RESPONSE_ACCEPT,
	                                      NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			gchar *filename;
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			doc = xmlParseFile (filename);
			if (doc != NULL)
				{
					node = xmlDocGetRootElement (doc);
					if (node != NULL)
						{
							gdaex_query_editor_load_choices_from_xml (qe, node, TRUE);
						}
				}
			g_free (filename);
		}
	gtk_widget_destroy (dialog);
}

static void
on_btn_get_sql_clicked (GtkButton *button,
                      gpointer user_data)
{
	xmlDoc *doc;
	xmlNode *node;
	xmlChar *buf;

	gchar *sql;

	gint size;

	/* SQL */
	sql = (gchar *)gdaex_query_editor_get_sql (qe);
	sql = g_strjoinv ("%%", g_strsplit (sql, "%", -1));
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                            GTK_DIALOG_DESTROY_WITH_PARENT,
	                                            GTK_MESSAGE_INFO,
	                                            GTK_BUTTONS_OK,
	                                            sql);
	gtk_window_set_title (GTK_WINDOW (dialog), "Sql");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	/* SQL SELECT */
	sql = (gchar *)gdaex_query_editor_get_sql_select (qe);
	sql = g_strjoinv ("%%", g_strsplit (sql, "%", -1));
	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 sql);
	gtk_window_set_title (GTK_WINDOW (dialog), "Sql Select");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (sql);

	/* SQL FROM */
	sql = (gchar *)gdaex_query_editor_get_sql_from (qe);
	sql = g_strjoinv ("%%", g_strsplit (sql, "%", -1));
	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 sql);
	gtk_window_set_title (GTK_WINDOW (dialog), "Sql From");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (sql);

	/* SQL WHERE */
	sql = (gchar *)gdaex_query_editor_get_sql_where (qe);
	sql = g_strjoinv ("%%", g_strsplit (sql, "%", -1));
	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 sql);
	gtk_window_set_title (GTK_WINDOW (dialog), "Sql Where");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (sql);

	/* SQL ORDER */
	sql = (gchar *)gdaex_query_editor_get_sql_order (qe);
	sql = g_strjoinv ("%%", g_strsplit (sql, "%", -1));
	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 sql);
	gtk_window_set_title (GTK_WINDOW (dialog), "Sql Order By");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (sql);

	/* XML */
	doc = xmlNewDoc ("1.0");
	node = gdaex_query_editor_get_sql_as_xml (qe);
	xmlDocSetRootElement (doc, node);
	xmlDocDumpMemory (doc, &buf, &size);
	buf = g_strjoinv ("%%", g_strsplit (buf, "%", -1));

	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 buf);
	gtk_window_set_title (GTK_WINDOW (dialog), "Xml");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

int
main (int argc, char *argv[])
{
	GError *error;

	GOptionContext *context;

	GtkWidget *hbtnbox;

	xmlfile = NULL;

	GOptionEntry entries[] =
		{
			{ "xml-file", 'x', 0, G_OPTION_ARG_FILENAME, &xmlfile, "Xml file", NULL },
			{ NULL }
		};

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=query_editor.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Error on GdaEx initialization.");
			return 0;
		}

	error = NULL;
	context = g_option_context_new ("tests");
	g_option_context_add_main_entries (context, entries, "tests");
	g_option_context_add_group (context, gdaex_get_option_group (gdaex));
	g_option_context_parse (context, &argc, &argv, &error);
	if (error != NULL)
		{
			g_warning ("Error on command line parsing: %s", error->message);
		}

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 610, 400);
	gtk_window_set_modal (GTK_WINDOW (w), TRUE);
	gtk_window_set_position (GTK_WINDOW (w), GTK_WIN_POS_CENTER);
	g_signal_connect (G_OBJECT (w), "delete-event",
	                  gtk_main_quit, NULL);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add (GTK_CONTAINER (w), vbox);

	hbtnbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbtnbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbtnbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox), hbtnbox, FALSE, FALSE, 5);

	btn_destroy = gtk_button_new_with_mnemonic ("_Destroy");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_destroy, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_destroy), "clicked",
	                  G_CALLBACK (on_btn_destroy_clicked), NULL);

	btn_clean = gtk_button_new_with_mnemonic ("_Clear");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_clean, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_clean), "clicked",
	                  G_CALLBACK (on_btn_clean_clicked), NULL);

	btn_get_sql = gtk_button_new_with_mnemonic ("Get _Sql");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_get_sql, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_get_sql), "clicked",
	                  G_CALLBACK (on_btn_get_sql_clicked), NULL);

	btn_save_xml = gtk_button_new_with_mnemonic ("_Save");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_save_xml, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_save_xml), "clicked",
	                  G_CALLBACK (on_btn_save_xml_clicked), NULL);

	btn_load_xml = gtk_button_new_with_mnemonic ("_Open");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_load_xml, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_load_xml), "clicked",
	                  G_CALLBACK (on_btn_load_xml_clicked), NULL);

	btn_ok = gtk_button_new_with_mnemonic ("_Close");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_ok, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_ok), "clicked",
	                  gtk_main_quit, NULL);

	do_query_editor ();

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
