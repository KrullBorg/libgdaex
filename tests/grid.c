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
	GtkWidget *w;

	GdaExGrid *grid;
	GdaExGridColumn *gcol;
	GtkWidget *wgrid;

	gtk_init (&argc, &argv);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	grid = gdaex_grid_new ();

	gcol = gdaex_grid_column_new ("ID", "id", G_TYPE_INT, TRUE, FALSE, FALSE, FALSE, 0);
	gdaex_grid_add_column (grid, gcol);

	gcol = gdaex_grid_column_new ("Name", "name", G_TYPE_STRING, TRUE, TRUE, TRUE, TRUE, 0);
	gdaex_grid_add_column (grid, gcol);

	wgrid = gdaex_grid_get_widget (grid);
	gtk_container_add (GTK_CONTAINER (w), wgrid);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
