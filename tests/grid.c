/*
 * Copyright (C) 2011-2013 Andrea Zagli <azagli@libero.it>
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

#include <libgdaex.h>

GdaExGridColumn *gcol_birthday;

static void
on_btn_birthday_clicked (GtkButton *button,
                         gpointer user_data)
{
	gdaex_grid_column_set_visible (gcol_birthday, !gdaex_grid_column_get_visible (gcol_birthday));
}

int
main (int argc, char **argv)
{
	GdaEx *gdaex;

	GtkWidget *w;
	GtkWidget *vbox;
	GtkWidget *scrolledw;
	GtkWidget *hbtnbox;
	GtkWidget *btn;

	GdaExGrid *grid;
	GdaExGridColumn *gcol;
	GtkWidget *wgrid;

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=grid.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Unable to connect to the db.");
		}

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 550, 350);
	g_signal_connect (w, "delete-event",
	                  G_CALLBACK (gtk_false), NULL);
	g_signal_connect (w, "destroy",
	                  G_CALLBACK (gtk_main_quit), NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (w), vbox);

	scrolledw = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), scrolledw, TRUE, TRUE, 0);

	grid = gdaex_grid_new ();
	gdaex_grid_set_title (grid, "The grid title");

#ifdef SOLIPA_FOUND
	gdaex_grid_set_solipa (grid, solipa_new ());
#endif

	gcol = gdaex_grid_column_new ("ID", "id", G_TYPE_INT, TRUE, FALSE, FALSE, FALSE, -1);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Name", "name", G_TYPE_STRING, TRUE, TRUE, TRUE, TRUE, -1);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new_defaults ("Surname", "surname", G_TYPE_STRING);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new_defaults ("Age", "age", G_TYPE_INT);
	gdaex_grid_add_column (grid, gcol);

	gcol_birthday = gdaex_grid_column_new ("Birthday", "birthday", G_TYPE_DATE, FALSE, TRUE, TRUE, TRUE, -1);
	gdaex_grid_add_column (grid, gcol_birthday);

	gcol = gdaex_grid_column_new_defaults ("Incoming", "incoming", G_TYPE_DOUBLE);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Married", "married", G_TYPE_BOOLEAN, TRUE, TRUE, TRUE, TRUE, -1);
	gdaex_grid_add_column (grid, gcol);

	wgrid = gdaex_grid_get_widget (grid);
	gtk_container_add (GTK_CONTAINER (scrolledw), wgrid);

	gdaex_grid_fill_from_sql (grid, gdaex, "SELECT * FROM clients", NULL);

	hbtnbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), hbtnbox, TRUE, TRUE, 0);

	btn = gtk_button_new_with_label ("Hide/Show Birthday");
	gtk_box_pack_start (GTK_BOX (hbtnbox), btn, TRUE, TRUE, 0);

	g_signal_connect (G_OBJECT (btn), "clicked",
	                  G_CALLBACK (on_btn_birthday_clicked), NULL);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
