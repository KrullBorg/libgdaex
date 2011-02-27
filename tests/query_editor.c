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

int
main (int argc, char *argv[])
{
	GError *error;

	GOptionContext *context;

	GdaEx *gdaex;
	GdaExQueryEditor *qe;

	gtk_init (&argc, &argv);

	gdaex = 	gdaex = gdaex_new_from_string (g_strdup_printf ("SQLite://DB_DIR=%s;DB_NAME=test_prefix.db", TESTSDIR));
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

	gtk_dialog_run (GTK_DIALOG (gdaex_query_editor_get_dialog (qe)));

	gtk_main ();

	return 0;
}
