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
	GValue *gval;

	gtk_init (&argc, &argv);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_SELECT);

	gdaex_sql_builder_from_v (sqlb,
							  "pippo", "pluto",
							  "topolino", "",
							  NULL);

	gdaex_sql_builder_fields (sqlb,
							  "pippo", "id", "", NULL,
							  "pippo", "name", "the_name", NULL,
							  NULL);

	gdaex_sql_builder_field (sqlb, "pippo", "income", NULL, NULL);

	gdaex_sql_builder_join (sqlb, GDA_SQL_SELECT_JOIN_INNER,
							"pippo", "id", "",
							GDA_SQL_OPERATOR_TYPE_EQ,
							"topolino", "id_pippo", "",
							NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 44);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_EQ,
							 gval,
							 NULL);
	g_value_unset (gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "ichichc");
	gdaex_sql_builder_where (sqlb, GDA_SQL_OPERATOR_TYPE_OR,
							 "pippo", "name", "",
							 GDA_SQL_OPERATOR_TYPE_EQ,
							 gval,
							 NULL);
	g_value_unset (gval);

	gdaex_sql_builder_order (sqlb,
							 "pippo", "name", "", TRUE,
							 "pippo", "id", "", FALSE,
							 NULL);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb, NULL, NULL));

	g_object_unref (sqlb);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_SELECT);

	gdaex_sql_builder_from (sqlb, "pippo", "");

	gdaex_sql_builder_fields (sqlb,
							  "pippo", "id", "", NULL,
							  "pippo", "name", "the_name", NULL,
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 44);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_ISNULL,
							 gval,
							 NULL);
	g_value_unset (gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 44);
	GValue *gval2 = g_new0 (GValue, 1);
	g_value_init (gval2, G_TYPE_INT);
	g_value_set_int (gval2, 8877);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_BETWEEN,
							 gval, gval2,
							 NULL);
	g_value_unset (gval);
	g_value_unset (gval2);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb, NULL, NULL));

	g_object_unref (sqlb);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_DELETE);

	gdaex_sql_builder_from_v (sqlb,
							  "pippo", "pluto",
							  "topolino", "",
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 1);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_EQ,
							 gval,
							 NULL);
	g_value_unset (gval);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb, NULL, NULL));

	g_object_unref (sqlb);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_UPDATE);

	gdaex_sql_builder_from_v (sqlb,
							  "pippo", "pluto",
							  "topolino", "",
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "il nome di pippo");
	gdaex_sql_builder_fields (sqlb,
							  "pippo", "name", "", gval,
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 1);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_EQ,
							 gval,
							 NULL);
	g_value_unset (gval);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb, NULL, NULL));

	g_object_unref (sqlb);

	sqlb = gdaex_sql_builder_new (GDA_SQL_STATEMENT_INSERT);

	gdaex_sql_builder_from_v (sqlb,
							  "pippo", "pluto",
							  "topolino", "",
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "il nome di pippo");
	gdaex_sql_builder_fields (sqlb,
							  "pippo", "name", "", gval,
							  NULL);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 1);
	gdaex_sql_builder_where (sqlb, 0,
							 "pippo", "id", "",
							 GDA_SQL_OPERATOR_TYPE_EQ,
							 gval,
							 NULL);
	g_value_unset (gval);

	g_message ("sql: %s", gdaex_sql_builder_get_sql (sqlb, NULL, NULL));

	g_object_unref (sqlb);

	return 0;
}
