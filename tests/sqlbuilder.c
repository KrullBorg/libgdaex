/*
 * Copyright (C) 2015 Andrea Zagli <azagli@libero.it>
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
	GdaExSqlBuilder *sqlb;

	gtk_init (&argc, &argv);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_SELECT);
	gdaex_sql_builder_from (sqlb, "pippo", "pluto");
	gdaex_sql_builder_fields (sqlb,
							  "pippo", "id", "",
							  "pippo", "name", "the_name",
							  NULL);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb));
	
	return 0;
}
