/*
 * Copyright (C) 2011 Andrea Zagli <azagli@libero.it>
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

GtkWidget *w;

static void
on_btn_clean_clicked (GtkButton *button,
                      gpointer user_data)
{
	gdaex_query_editor_clean_choices ((GdaExQueryEditor *)user_data);
}

static void
on_btn_save_xml_clicked (GtkButton *button,
                      gpointer user_data)
{
	xmlDoc *doc;
	xmlNode *node;
	GtkWidget *dialog;

	doc = xmlNewDoc ("1.0");
	node = gdaex_query_editor_get_sql_as_xml ((GdaExQueryEditor *)user_data);
	xmlDocSetRootElement (doc, node);

	dialog = gtk_file_chooser_dialog_new ("Save xml to...",
	                                      GTK_WINDOW (w),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
	                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
							gdaex_query_editor_load_choices_from_xml ((GdaExQueryEditor *)user_data, node, TRUE);
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
	gint size;

	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                            GTK_DIALOG_DESTROY_WITH_PARENT,
	                                            GTK_MESSAGE_INFO,
	                                            GTK_BUTTONS_OK,
	                                            gdaex_query_editor_get_sql ((GdaExQueryEditor *)user_data));
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);;

	doc = xmlNewDoc ("1.0");
	node = gdaex_query_editor_get_sql_as_xml ((GdaExQueryEditor *)user_data);
	xmlDocSetRootElement (doc, node);
	xmlDocDumpMemory (doc, &buf, &size);

	dialog = gtk_message_dialog_new (GTK_WINDOW (w),
	                                            GTK_DIALOG_DESTROY_WITH_PARENT,
	                                            GTK_MESSAGE_INFO,
	                                            GTK_BUTTONS_OK,
	                                            buf);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);;
}

int
main (int argc, char *argv[])
{
	GError *error;

	GOptionContext *context;

	GdaEx *gdaex;
	GdaExQueryEditor *qe;

	GdaExQueryEditorField *field;

	GtkWidget *vbox;
	GtkWidget *widget_qe;
	GtkWidget *hbtnbox;
	GtkWidget *btn_clean;
	GtkWidget *btn_get_sql;
	GtkWidget *btn_save_xml;
	GtkWidget *btn_load_xml;
	GtkWidget *btn_ok;

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=test_prefix.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Error on GdaEx initialization.");
			return 0;
		}

	error = NULL;
	context = g_option_context_new ("tests");
	g_option_context_add_group (context, gdaex_get_option_group (gdaex));
	g_option_context_parse (context, &argc, &argv, &error);
	if (error != NULL)
		{
			g_warning ("Error on command line parsing: %s", error->message);
		}

	qe = gdaex_query_editor_new (gdaex);

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
	field->available_where_type = GDAEX_QE_WHERE_TYPE_ILIKE
	                              | GDAEX_QE_WHERE_TYPE_ILIKE;
	gdaex_query_editor_table_add_field (qe, "clients", *field);
	g_free (field);

	field = g_new0 (GdaExQueryEditorField, 1);
	field->name = g_strdup ("surname");
	field->name_visible = g_strdup ("Surname");
	field->description = g_strdup ("The client's surname");
	field->type = GDAEX_QE_FIELD_TYPE_TEXT;
	field->for_show = TRUE;
	field->for_where = TRUE;
	field->available_where_type = GDAEX_QE_WHERE_TYPE_LIKE
	                              | GDAEX_QE_WHERE_TYPE_ILIKE;
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
	field->available_where_type = GDAEX_QE_WHERE_TYPE_EQUAL
	                              | GDAEX_QE_WHERE_TYPE_GREAT
	                              | GDAEX_QE_WHERE_TYPE_GREAT_EQUAL
	                              | GDAEX_QE_WHERE_TYPE_LESS
	                              | GDAEX_QE_WHERE_TYPE_LESS_EQUAL
	                              | GDAEX_QE_WHERE_TYPE_BETWEEN;
	gdaex_query_editor_table_add_field (qe, "clients", *field);
	g_free (field);

	field = g_new0 (GdaExQueryEditorField, 1);
	field->name = g_strdup ("id_cities");
	field->name_visible = g_strdup ("City");
	field->description = g_strdup ("The client's city");
	field->type = GDAEX_QE_FIELD_TYPE_INTEGER;
	field->for_show = TRUE;
	field->for_where = TRUE;
	field->decode_table2 = g_strdup ("cities");
	/*field->decode_fields1 = g_slist_append (field->decode_fields1, "id_cities");
	field->decode_fields2 = g_slist_append (field->decode_fields2, "id");*/
	field->decode_field2 = g_strdup ("id");
	field->decode_field_to_show = g_strdup ("name");
	field->decode_field_alias = g_strdup ("city_name");
	field->available_where_type = GDAEX_QE_WHERE_TYPE_EQUAL;
	gdaex_query_editor_table_add_field (qe, "clients", *field);
	g_free (field);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 610, 400);
	gtk_window_set_modal (GTK_WINDOW (w), TRUE);
	gtk_window_set_position (GTK_WINDOW (w), GTK_WIN_POS_CENTER);
	g_signal_connect (G_OBJECT (w), "delete-event",
	                  gtk_main_quit, NULL);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (w), vbox);

	widget_qe = gdaex_query_editor_get_widget (qe);
	gtk_box_pack_start (GTK_BOX (vbox), widget_qe, TRUE, TRUE, 5);

	hbtnbox = gtk_hbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbtnbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbtnbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox), hbtnbox, FALSE, FALSE, 5);

	btn_clean = gtk_button_new_from_stock ("gtk-clear");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_clean, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_clean), "clicked",
	                  G_CALLBACK (on_btn_clean_clicked), qe);

	btn_get_sql = gtk_button_new_with_mnemonic ("Get _Sql");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_get_sql, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_get_sql), "clicked",
	                  G_CALLBACK (on_btn_get_sql_clicked), qe);

	btn_save_xml = gtk_button_new_from_stock ("gtk-save");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_save_xml, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_save_xml), "clicked",
	                  G_CALLBACK (on_btn_save_xml_clicked), qe);

	btn_load_xml = gtk_button_new_from_stock ("gtk-open");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_load_xml, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_load_xml), "clicked",
	                  G_CALLBACK (on_btn_load_xml_clicked), qe);

	btn_ok = gtk_button_new_from_stock ("gtk-close");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn_ok, TRUE, TRUE, 5);
	g_signal_connect (G_OBJECT (btn_ok), "clicked",
	                  gtk_main_quit, NULL);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
