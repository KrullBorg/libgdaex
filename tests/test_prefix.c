/*
 * Copyright (C) 2010 Andrea Zagli <azagli@libero.it>
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

#include "libgdaex.h"

int
main (int argc, char **argv)
{
	GdaEx *gdaex;
	GError *error;
	GOptionContext *context;

	g_type_init ();

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=test_prefix.db", TESTSDIR));

	error = NULL;
	context = g_option_context_new ("tests");
	g_option_context_add_group (context, gdaex_get_option_group (gdaex));
	g_option_context_parse (context, &argc, &argv, &error);
	if (error != NULL)
		{
			g_warning ("Error on command line parsing: %s", error->message);
		}

	gdaex_set_tables_name_prefix (gdaex, "paperinik_");

	gdaex_execute (gdaex, "INSERT INTO table1 (id) VALUES (1)");

	gdaex_execute (gdaex, "UPDATE table1 SET id = 1 WHERE id = 1");

	gdaex_execute (gdaex, "DELETE FROM table1 WHERE id = 1");

	gdaex_query (gdaex, "SELECT * FROM table1 WHERE id = 1");

	gdaex_query (gdaex, "SELECT * FROM paperinik_table1 WHERE id = 1");

	gdaex_query (gdaex, "SELECT * FROM table_sel1 INNER JOIN table_sel2 ON table_sel1.id = table_sel2.id2");

	gdaex_query (gdaex, "SELECT * FROM table_sel1 AS t1 INNER JOIN table_sel2 ON t1.id = table_sel2.id2");

	gdaex_query (gdaex, "SELECT * FROM table_sel1 AS t1");

	gdaex_free (gdaex);

	return 0;
}
