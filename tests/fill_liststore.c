/*
 * Copyright (C) 2011 Andrea Zagli <azagli@libero.it>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <libgdaex.h>

int
main (int argc, char **argv)
{
	GtkWidget *w;
	GtkWidget *scrolledw;
	GtkListStore *lstore;
	GtkWidget *tview;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	GdaEx *gdaex;

	gchar *sql;

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=grid.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Unable to connect to the db.");
		}

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 550, 350);

	scrolledw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (w), scrolledw);

	lstore = gtk_list_store_new (7,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING,
	                             G_TYPE_STRING);

	tview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (lstore));

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("ID",
	                                                   renderer,
	                                                   "text", 0,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Name",
	                                                   renderer,
	                                                   "text", 1,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Surname",
	                                                   renderer,
	                                                   "text", 2,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Age",
	                                                   renderer,
	                                                   "text", 3,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Birthday",
	                                                   renderer,
	                                                   "text", 4,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Incoming",
	                                                   renderer,
	                                                   "text", 5,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Married",
	                                                   renderer,
	                                                   "text", 6,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tview), column);

	sql = g_strdup_printf ("SELECT * FROM clients");
	gdaex_fill_liststore_from_sql (gdaex, lstore, sql, NULL, NULL);
	g_free (sql);

	gtk_container_add (GTK_CONTAINER (scrolledw), tview);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
