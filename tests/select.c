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

	gchar *sql;
	GdaDataModel *dm;

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=grid.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Unable to connect to the db.");
		}

	sql = g_strdup_printf ("SELECT * FROM clientssss");
	dm = gdaex_query (gdaex, sql);
	g_free (sql);
	g_object_unref (dm);

	return 0;
}
