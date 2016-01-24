/*
 * Copyright (C) 2016 Andrea Zagli <azagli@libero.it>
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

void
read_xml_file (const gchar *xmlfilename, GdaEx *gdaex)
{
	xmlDoc *xdoc;
	xmlNode *xnode;

	gchar *filename;
	gchar *sql;
	GdaDataModel *dm;

	GError *error;

	xdoc = xmlParseFile (xmlfilename);
	if (xdoc == NULL)
		{
			g_error ("Unable to parse xml file «%s».", xmlfilename);
		}
	else
		{
			xnode = xmlDocGetRootElement (xdoc);
			if (xnode != NULL)
				{
					if (xmlStrcmp (xnode->name, "gdaex2gettext") != 0)
						{
							g_error ("Xml file not valid: «%s».", xmlfilename);
						}
					else
						{
							xnode = xnode->children;
							while (xnode)
								{
									if (xmlStrcmp (xnode->name, "file") == 0)
										{
											filename = g_strdup (xmlGetProp (xnode, "filename"));
											if (filename != NULL
												&& g_strcmp0 (g_strstrip (filename), "") != 0)
												{
													sql = g_strdup (xmlNodeGetContent (xnode));

													dm = gdaex_query (gdaex, g_strstrip (sql));
													xmlFree (sql);
													if (dm != NULL)
														{
															error = NULL;
															if (!gda_data_model_export_to_file (dm,
																								GDA_DATA_MODEL_IO_DATA_ARRAY_XML,
																								filename,
																								NULL, 0,
																								NULL, 0,
																								NULL,
																								&error)
																|| error != NULL)
																{
																	g_warning ("Error on data model export: %s.",
																			   error != NULL && error->message != NULL ? error->message : "no details");
																}
															g_object_unref (dm);
														}
												}
											if (filename != NULL)
												{
													g_free (filename);
												}
										}

									xnode = xnode->next;
								}
						}
				}
			else
				{
					g_error ("Xml file not valid: «%s».", xmlfilename);
				}
	}

	return;
}

int
main (int argc, char **argv)
{
	gchar *cncstring;
	gchar *xmlfile;

	GError *error;
	GOptionContext *context;

	GdaEx *gdaex;

	GOptionEntry entries[] =
		{
			{ "cnc-string", 'c', 0, G_OPTION_ARG_STRING, &cncstring, "Connection string", NULL },
			{ "xml-file", 'x', 0, G_OPTION_ARG_FILENAME, &xmlfile, "Xml file", NULL },
			{ NULL }
		};

	cncstring = NULL;
	xmlfile = NULL;

	gtk_init (&argc, &argv);

	error = NULL;
	context = g_option_context_new ("gdaex2gettext");
	g_option_context_add_main_entries (context, entries, "gdaex2gettext");
	g_option_context_parse (context, &argc, &argv, &error);
	if (error != NULL)
		{
			g_error ("Error on command line parsing: %s", error->message);
			return 0;
		}

	if (cncstring == NULL)
		{
			g_error ("Connection string is mandatory.");
			return 0;
		}
	if (xmlfile == NULL)
		{
			g_error ("Xml file is mandatory.");
			return 0;
		}

	gdaex = gdaex_new_from_string (cncstring);
	if (gdaex == NULL)
		{
			g_error ("Unable to create database connection «%s».", cncstring);
			return 0;
		}

	read_xml_file (xmlfile, gdaex);

	return 0;
}
