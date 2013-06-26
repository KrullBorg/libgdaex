/*
 * Copyright (C) 2013 Andrea Zagli <azagli@libero.it>
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
	GdaEx *gdaex;

	GHashTable *ht_keys;
	GHashTable *ht_fields;

	GValue *gval;
	GDate *gdate;

	gtk_init (&argc, &argv);

	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=grid.db", TESTSDIR));
	if (gdaex == NULL)
		{
			g_error ("Unable to connect to the db.");
		}

	ht_keys = g_hash_table_new (g_str_hash, g_str_equal);
	ht_fields = g_hash_table_new (g_str_hash, g_str_equal);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "id");
	g_hash_table_insert (ht_fields, "id", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "name");
	g_hash_table_insert (ht_fields, "name", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "surname");
	g_hash_table_insert (ht_fields, "surname", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "birthday");
	g_hash_table_insert (ht_fields, "birthday", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "address");
	g_hash_table_insert (ht_fields, "address", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "incoming");
	g_hash_table_insert (ht_fields, "incoming", gval);

	g_message ("%s", gdaex_get_sql_from_hashtable (gdaex, GDAEX_SQL_SELECT, "clients", ht_keys, ht_fields));
	g_hash_table_destroy (ht_keys);
	g_hash_table_destroy (ht_fields);

	g_message ("%s", gdaex_get_sql_from_hashtable (gdaex, GDAEX_SQL_INSERT, "clients", ht_keys, ht_fields));

	ht_keys = g_hash_table_new (g_str_hash, g_str_equal);
	ht_fields = g_hash_table_new (g_str_hash, g_str_equal);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 44);
	g_hash_table_insert (ht_fields, "id", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "my name is my name");
	g_hash_table_insert (ht_fields, "name", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "and this is the sur'name with '");
	g_hash_table_insert (ht_fields, "surname", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_DATE);
	gdate = g_date_new_dmy (12, 5, 1966);
	g_value_set_boxed (gval, gdate);
	g_hash_table_insert (ht_fields, "birthday", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "5th avenue, 44");
	g_hash_table_insert (ht_fields, "address", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_DOUBLE);
	g_value_set_double (gval, 1451.22);
	g_hash_table_insert (ht_fields, "incoming", gval);

	g_message ("%s", gdaex_get_sql_from_hashtable (gdaex, GDAEX_SQL_INSERT, "clients", ht_keys, ht_fields));
	g_hash_table_destroy (ht_keys);
	g_hash_table_destroy (ht_fields);

	ht_keys = g_hash_table_new (g_str_hash, g_str_equal);
	ht_fields = g_hash_table_new (g_str_hash, g_str_equal);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "my new name");
	g_hash_table_insert (ht_fields, "name", gval);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_STRING);
	g_value_set_string (gval, "and this is the new sur'name with '");
	g_hash_table_insert (ht_fields, "surname", gval);

	ht_keys = g_hash_table_new (g_str_hash, g_str_equal);
	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 55);
	g_hash_table_insert (ht_keys, "id", gval);

	g_message ("%s", gdaex_get_sql_from_hashtable (gdaex, GDAEX_SQL_UPDATE, "clients", ht_keys, ht_fields));
	g_hash_table_destroy (ht_keys);
	g_hash_table_destroy (ht_fields);

	ht_keys = g_hash_table_new (g_str_hash, g_str_equal);

	gval = g_new0 (GValue, 1);
	g_value_init (gval, G_TYPE_INT);
	g_value_set_int (gval, 55);
	g_hash_table_insert (ht_keys, "id", gval);

	g_message ("%s", gdaex_get_sql_from_hashtable (gdaex, GDAEX_SQL_DELETE, "clients", ht_keys, ht_fields));
	g_hash_table_destroy (ht_keys);

	return 0;
}
