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

#include <libgdaex.h>

int
main (int argc, char **argv)
{
	GdaEx *gdaex;

	GtkWidget *w;

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

	grid = gdaex_grid_new ();

	gcol = gdaex_grid_column_new ("ID", "id", G_TYPE_INT, TRUE, FALSE, FALSE, FALSE, 0);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Name", "name", G_TYPE_STRING, TRUE, TRUE, TRUE, TRUE, 0);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Surname", "surname", G_TYPE_STRING, TRUE, TRUE, TRUE, TRUE, 0);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Age", "age", G_TYPE_INT, TRUE, TRUE, TRUE, TRUE, 0);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Birthday", "birthday", G_TYPE_DATE, TRUE, TRUE, TRUE, TRUE, 0);
	gdaex_grid_add_column (grid, gcol);

	wgrid = gdaex_grid_get_widget (grid);
	gtk_container_add (GTK_CONTAINER (w), wgrid);

	gdaex_grid_fill_from_sql (grid, gdaex, "SELECT * FROM clients", NULL);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
