/*
 *  gdaex.c
 *
 *  Copyright (C) 2005-2011 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex.
 *  
 *  libgdaex is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaex is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaex; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <stdarg.h>
#include <string.h>

#include <gio/gio.h>
#include <libgda/sql-parser/gda-sql-parser.h>

#include "gdaex.h"

static guint debug;
static gchar *log_file;

static GOptionEntry entries[] = 
{
	{ "gdaex-debug-level", 0, 0, G_OPTION_ARG_INT, &debug, "Sets the debug level", NULL },
	{ "gdaex-log-file", 0, 0, G_OPTION_ARG_FILENAME, &log_file, "Path to file where to write debug info (or stdout or stderr)", NULL },
	{ NULL }
};

static void gdaex_class_init (GdaExClass *klass);
static void gdaex_init (GdaEx *gdaex);

static void gdaex_create_connection_parser (GdaEx *gdaex);

static void gdaex_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

#define GDAEX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAEX, GdaExPrivate))

typedef struct _GdaExPrivate GdaExPrivate;
struct _GdaExPrivate
	{
		GdaConnection *gda_conn;
		GdaSqlParser *gda_parser;

		gchar *tables_name_prefix;

		guint debug;
		GFileOutputStream *log_file;
	};

G_DEFINE_TYPE (GdaEx, gdaex, G_TYPE_OBJECT)

static void
gdaex_class_init (GdaExClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExPrivate));

	object_class->set_property = gdaex_set_property;
	object_class->get_property = gdaex_get_property;

	/**
	 * GdaEx::before-execute:
	 * @gdaex:
	 * @gdastatement:
	 *
	 */
	klass->before_execute_signal_id = g_signal_new ("before-execute",
	                                               G_TYPE_FROM_CLASS (object_class),
	                                               G_SIGNAL_RUN_LAST,
	                                               0,
	                                               NULL,
	                                               NULL,
	                                               g_cclosure_marshal_VOID__POINTER,
	                                               G_TYPE_NONE,
	                                               1, G_TYPE_POINTER);

	/**
	 * GdaEx::after-execute:
	 * @gdaex:
	 * @gdastatement:
	 *
	 */
	klass->after_execute_signal_id = g_signal_new ("after-execute",
	                                               G_TYPE_FROM_CLASS (object_class),
	                                               G_SIGNAL_RUN_LAST,
	                                               0,
	                                               NULL,
	                                               NULL,
	                                               g_cclosure_marshal_VOID__POINTER,
	                                               G_TYPE_NONE,
	                                               1, G_TYPE_POINTER);
}

static void
gdaex_init (GdaEx *gdaex)
{
	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	priv->tables_name_prefix = NULL;
	priv->debug = 0;
	priv->log_file = 0;
}

static GdaEx
*gdaex_new_ ()
{
	GdaEx *gdaex = GDAEX (g_object_new (gdaex_get_type (), NULL));

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	return gdaex;
}

/**
 * gdaex_new_from_dsn:
 * @dsn: GDA data source name to connect to.
 * @username: user name to use to connect.
 * @password: password for @username.
 *
 * If @username and @password are both NULL or empty, it will be used those
 * defined into datasource.
 *
 * Returns: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_dsn (const gchar *dsn, const gchar *username, const gchar *password)
{
	GdaEx *gdaex;
	GdaExPrivate *priv;
	gchar *new_user;
	gchar *new_pwd;
	gchar *auth_string;

	GError *error;

	if (dsn == NULL || strcmp (g_strstrip (g_strdup (dsn)), "") == 0)
		{
			/* TO DO */
			g_warning ("datasource must not be empty.");
			return NULL;
		}

	gdaex = gdaex_new_ ();
	if (gdaex == NULL)
		{
			/* TO DO */
			return NULL;
		}

	priv = GDAEX_GET_PRIVATE (gdaex);

	auth_string = NULL;
	new_user = NULL;
	new_pwd = NULL;

	if (username != NULL)
		{
			new_user = g_strstrip (g_strdup (username));
			if (g_strcmp0 (new_user, "") != 0)
				{
					new_user = gda_rfc1738_encode (new_user);
				}
			else
				{
					new_user = NULL;
				}
		}
	if (password != NULL)
		{
			new_pwd = g_strstrip (g_strdup (password));
			if (g_strcmp0 (new_pwd, "") != 0)
				{
					new_pwd = gda_rfc1738_encode (new_pwd);
				}
			else
				{
					new_pwd = NULL;
				}
		}

	if (new_user != NULL || new_pwd != NULL)
		{
			auth_string = g_strdup ("");
			if (new_user != NULL)
				{
					auth_string = g_strdup_printf ("USERNAME=%s", new_user);
				}
			if (new_pwd != NULL)
				{
					auth_string = g_strconcat (auth_string,
						(new_user != NULL ? ";" : ""),
						g_strdup_printf ("PASSWORD=%s", new_pwd),
						NULL);
				}
		}

	/* open database connection */
	error = NULL;
	priv->gda_conn = gda_connection_open_from_dsn (dsn,
	                                               auth_string,
	                                               GDA_CONNECTION_OPTIONS_NONE,
	                                               &error);
	if (error != NULL)
		{
			g_warning ("Error creating database connection: %s\n",
			           error->message);
			return NULL;
		}

	gdaex_create_connection_parser (gdaex);

	return gdaex;
}

/**
 * gdaex_new_from_string:
 * @cnc_string: the connection string.
 *
 * Returns: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_string (const gchar *cnc_string)
{
	GError *error;
	GdaEx *gdaex;
	GdaExPrivate *priv;

	if (cnc_string == NULL || strcmp (g_strstrip (g_strdup (cnc_string)), "") == 0)
		{
			/* TO DO */
			g_warning ("cnc_string must not be empty.");
			return NULL;
		}

	gdaex = gdaex_new_ ();
	if (gdaex == NULL)
		{
			/* TO DO */
			return NULL;
		}

	priv = GDAEX_GET_PRIVATE (gdaex);

	/* open database connection */
	error = NULL;
	priv->gda_conn = gda_connection_open_from_string (NULL,
	                                                  cnc_string,
	                                                  NULL,
	                                                  GDA_CONNECTION_OPTIONS_NONE,
                                                      &error);
	if (error != NULL)
		{
			g_warning ("Error creating database connection: %s",
			           error->message != NULL ? error->message : "no details.");
			return NULL;
		}

	gdaex_create_connection_parser (gdaex);

	return gdaex;
}

/**
 * gdaex_new_from_connection:
 * @conn: a #GdaConnection.
 *
 * Returns a #GdaEx from an existing #GdaConnection.
 *
 * Returns: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_connection (GdaConnection *conn)
{
	GdaEx *gdaex;
	GdaExPrivate *priv;

	g_return_val_if_fail (GDA_IS_CONNECTION (conn), NULL);

	gdaex = gdaex_new_ ();

	priv = GDAEX_GET_PRIVATE (gdaex);

	priv->gda_conn = conn;

	gdaex_create_connection_parser (gdaex);

	return gdaex;
}

static void
gdaex_log_handler (const gchar *log_domain,
                   GLogLevelFlags log_level,
                   const gchar *message,
                   gpointer user_data)
{
	GError *error;

	gchar *msg;

	GdaExPrivate *priv = GDAEX_GET_PRIVATE ((GdaEx *)user_data);

	msg = g_strdup_printf ("%s **: %s\n\n", log_domain, message);

	if (g_output_stream_write (G_OUTPUT_STREAM (priv->log_file),
	    msg, strlen (msg), NULL, &error) < 0)
		{
			g_warning ("Error on writing on log file: %s",
			           error != NULL && error->message != NULL ? error->message : "no details.");
		}
}

static gboolean
gdaex_post_parse_options (GOptionContext *context,
                          GOptionGroup *group,
                          gpointer user_data,
                          GError **error)
{
	GdaExPrivate *priv = GDAEX_GET_PRIVATE ((GdaEx *)user_data);

	GError *my_error;

	priv->debug = debug;
	if (log_file == NULL)
		{
			priv->log_file = 0;
		}
	else if (priv->debug > 0)
		{
			gchar *filename = g_strstrip (g_strdup (log_file));
			if (g_ascii_strncasecmp (filename, "stdout", 6) == 0
			    || g_ascii_strncasecmp (filename, "stderr", 6) == 0)
				{
				}
			else
				{
					my_error = NULL;
					priv->log_file = g_file_replace (g_file_new_for_path (filename),
					                                 NULL, FALSE, G_FILE_CREATE_NONE, NULL, &my_error);
					if (priv->log_file == NULL)
						{
							g_warning ("Error on opening log file: %s.",
							           my_error != NULL && my_error->message != NULL ? my_error->message : "no details.");
						}
					else
						{
							/* set handler */
							g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL
							                   | G_LOG_FLAG_RECURSION, gdaex_log_handler, user_data);
						}
				}	
		}

	return TRUE;
}

/**
 * gdaex_get_option_group:
 * #gdaex: a #GdaEx object.
 *
 * Returns: the #GOptionGroup.
 */
GOptionGroup
*gdaex_get_option_group (GdaEx *gdaex)
{
	GOptionGroup *ret;

	ret = g_option_group_new ("gdaex", "GdaEx", "GdaEx", (gpointer)gdaex, g_free);
	if (ret != NULL)
		{
			g_option_group_add_entries (ret, entries);
			g_option_group_set_parse_hooks (ret, NULL, gdaex_post_parse_options);
		}

	return ret;
}

/**
 * gdaex_get_gdaconnection:
 * @gdaex: a #GdaEx object.
 *
 * Returns: the #GdaConnection associated to the #GdaEx.
 */
const GdaConnection
*gdaex_get_gdaconnection (GdaEx *gdaex)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	return priv->gda_conn;
}

/**
 * gdaex_get_provider:
 * @gdaex: a #GdaEx object.
 *
 * Returns: the provider id associated to the #GdaEx.
 */
const gchar
*gdaex_get_provider (GdaEx *gdaex)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	return gda_server_provider_get_name (gda_connection_get_provider (priv->gda_conn));
}

/**
 * gdaex_get_tables_name_prefix:
 * @gdaex: a #GdaEx object.
 *
 */
const gchar
*gdaex_get_tables_name_prefix (GdaEx *gdaex)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	return g_strdup (priv->tables_name_prefix);
}

/**
 * gdaex_set_tables_name_prefix:
 * #gdaex: a #GdaEx object.
 * @tables_name_prefix:
 *
 */
void
gdaex_set_tables_name_prefix (GdaEx *gdaex, const gchar *tables_name_prefix)
{
	g_return_if_fail (IS_GDAEX (gdaex));

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	if (tables_name_prefix == NULL)
		{
			priv->tables_name_prefix = g_strdup ("");
		}
	else
		{
			priv->tables_name_prefix = g_strstrip (g_strdup (tables_name_prefix));
		}
}

static void
gdaex_set_tables_name_prefix_into_statement (GdaEx *gdaex, GdaStatement **stmt)
{
	GdaStatement *stmp;
	GdaSqlStatement *sstmt;

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	stmp = *stmt;
	g_object_get (G_OBJECT (stmp), "structure", &sstmt, NULL);
	if (sstmt == NULL)
		{
			g_warning ("Unable to get the GdaSqlStatement from the GdaStatement.");
			return;
		}

	switch (sstmt->stmt_type)
		{
			case GDA_SQL_STATEMENT_SELECT:
				{
					GdaSqlStatementSelect *select;
					GSList *tables;
					GSList *joins;
					GdaSqlSelectTarget *target;
					GdaSqlExpr *expr;
					GSList *operands;
					gchar **splits;
					gchar *field_name;
					gchar *tables_names;

					select = (GdaSqlStatementSelect *)sstmt->contents;

					tables_names = g_strdup ("");
					tables = NULL;
					tables = select->from->targets;
					while (tables != NULL)
						{
							target = (GdaSqlSelectTarget *)tables->data;
							if (g_ascii_strncasecmp (g_value_get_string (target->expr->value), priv->tables_name_prefix, strlen (priv->tables_name_prefix)) != 0)
								{
									g_value_set_string (target->expr->value,
									                    g_strdup_printf ("%s%s",
									                                     priv->tables_name_prefix,
									                                     g_value_get_string (target->expr->value)));
								}
							tables_names = g_strconcat (tables_names, g_value_get_string (target->expr->value), "|", NULL);
							tables = g_slist_next (tables);
						}

					joins = NULL;
					joins = select->from->joins;
					while (joins != NULL)
						{
							expr = ((GdaSqlSelectJoin *)joins->data)->expr;
							operands = ((GdaSqlOperation *)expr->cond)->operands;
							while (operands != NULL)
								{
									splits = gda_sql_identifier_split (g_value_get_string (((GdaSqlExpr *)operands->data)->value));
									if (g_strv_length (splits) > 1)
										{
											if (g_strrstr (tables_names, g_strdup_printf ("%s|", splits[0])) != NULL
											    && g_ascii_strncasecmp (splits[0], priv->tables_name_prefix, strlen (priv->tables_name_prefix)) != 0)
												{
													field_name = g_strdup_printf ("%s%s",
													                              priv->tables_name_prefix,
													                              g_value_get_string (((GdaSqlExpr *)operands->data)->value));
												}
											else
												{
													field_name = g_strdup (g_value_get_string (((GdaSqlExpr *)operands->data)->value));
												}
										}
									else
										{
											field_name = g_strdup (splits[0]);
										}

									g_value_set_string (((GdaSqlExpr *)operands->data)->value, field_name);
									g_free (field_name);
									operands = g_slist_next (operands);
								}

							joins = g_slist_next (joins);
						}
				}
				break;

			case GDA_SQL_STATEMENT_INSERT:
				{
					GdaSqlStatementInsert *insert;
					GValue *gval;

					insert = (GdaSqlStatementInsert *)sstmt->contents;
					gval = gda_value_new_from_string (g_strdup_printf ("%s%s",
					                                                   priv->tables_name_prefix,
					                                                   insert->table->table_name),
					                                  G_TYPE_STRING);
					gda_sql_statement_insert_take_table_name (sstmt, gval);
				}
				break;

			case GDA_SQL_STATEMENT_UPDATE:
				{
					GdaSqlStatementUpdate *update;
					GValue *gval;

					update = (GdaSqlStatementUpdate *)sstmt->contents;
					gval = gda_value_new_from_string (g_strdup_printf ("%s%s", priv->tables_name_prefix, update->table->table_name),
					                                  G_TYPE_STRING);
					gda_sql_statement_update_take_table_name (sstmt, gval);
				}
				break;

			case GDA_SQL_STATEMENT_DELETE:
				{
					GdaSqlStatementDelete *delete;
					GValue *gval;

					delete = (GdaSqlStatementDelete *)sstmt->contents;
					gval = gda_value_new_from_string (g_strdup_printf ("%s%s", priv->tables_name_prefix, delete->table->table_name),
					                                  G_TYPE_STRING);
					gda_sql_statement_delete_take_table_name (sstmt, gval);
				}
				break;

			default:
				g_warning ("Statement type %s not implemented.",
				           gda_sql_statement_type_to_string (sstmt->stmt_type));
				return;
		}

	g_object_set (G_OBJECT (stmp), "structure", sstmt, NULL);
	g_free (sstmt);
}

/**
 * gdaex_query:
 * @gdaex: a #GdaEx object.
 * @sql: the sql text.
 *
 * Execute a selection query (SELECT).
 *
 * Returns: a #GdaDataModel, or #NULL if query fails.
 */
GdaDataModel
*gdaex_query (GdaEx *gdaex, const gchar *sql)
{
	GError *error;

	GdaStatement *stmt;
	GdaDataModel *dm;

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	error = NULL;
	stmt = gda_sql_parser_parse_string (priv->gda_parser, sql, NULL, &error);
	if (!GDA_IS_STATEMENT (stmt))
		{
			g_warning ("Error parsing query string: %s\n%s",
			           error != NULL && error->message != NULL ? error->message : "no details", sql);
			return NULL;
		}

	if (priv->tables_name_prefix != NULL
	    && g_strcmp0 (priv->tables_name_prefix, "") != 0)
		{
			gdaex_set_tables_name_prefix_into_statement (gdaex, &stmt);
		}

	error = NULL;
	dm = gda_connection_statement_execute_select (priv->gda_conn, stmt, NULL, &error);
	g_object_unref (stmt);

	if (!GDA_IS_DATA_MODEL (dm))
		{
			g_warning ("Error executing selection query: %s\n%s",
			           error != NULL && error->message != NULL ? error->message : "no details", sql);
			return NULL;
		}
	else
		{
			if (priv->debug > 0)
				{
					g_message ("Selection query executed: %s", sql);
				}
		}

	return dm;
}

/**
 * gdaex_data_model_get_field_value_stringify_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gchar (stringify)
 */
gchar
*gdaex_data_model_get_field_value_stringify_at (GdaDataModel *data_model,
                                               gint row,
                                               const gchar *field_name)
{
	gchar *value;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_stringify_at (data_model, row, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_integer_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gint
 */
gint
gdaex_data_model_get_field_value_integer_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gint value = 0;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_integer_at (data_model, row, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_float_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gfloat
 */
gfloat
gdaex_data_model_get_field_value_float_at (GdaDataModel *data_model,
                                          gint row,
                                          const gchar *field_name)
{
	gfloat value = 0.0f;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_float_at (data_model, row, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_double_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gdouble
 */
gdouble
gdaex_data_model_get_field_value_double_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	gdouble value = 0.0;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_double_at (data_model, row, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_boolean_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gboolean
 */
gboolean
gdaex_data_model_get_field_value_boolean_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gboolean value = FALSE;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_boolean_at (data_model, row, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_gdatimestamp_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GdaTimestamp.
 */
GdaTimestamp
*gdaex_data_model_get_field_value_gdatimestamp_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	const GdaTimestamp *value;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_gdatimestamp_at (data_model, row, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return gda_timestamp_copy ((gpointer)value);
}

/**
 * gdaex_data_model_get_field_value_gdate_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GDate.
 */
GDate
*gdaex_data_model_get_field_value_gdate_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	GDate *value;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_gdate_at (data_model, row, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_gdatetime_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GDateTime.
 */
GDateTime
*gdaex_data_model_get_field_value_gdatetime_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	GDateTime *value;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_gdatetime_at (data_model, row, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_get_field_value_tm_at:
 * @data_model: a #GdaDataModel object.
 * @row:
 * @field_name: the field's name.
 *
 * Returns: the @field_name's value as a struct tm.
 */
struct tm
*gdaex_data_model_get_field_value_tm_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	struct tm *value;
	gint col;

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_tm_at (data_model, row, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_get_value_stringify_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #gchar (stringify).
 */
gchar
*gdaex_data_model_get_value_stringify_at (GdaDataModel *data_model, gint row, gint col)
{
	gchar *ret;
	const GValue *v;
	GError *error;

	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (error == NULL)
		{
			if (!gda_value_is_null (v))
				{
					ret = gda_value_stringify (v);
				}
			else
				{
					ret = g_strdup ("");
				}
		}
	else
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
			ret = NULL;
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_integer_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #gint.
 */
gint
gdaex_data_model_get_value_integer_at (GdaDataModel *data_model, gint row, gint col)
{
	gint ret = 0;
	const GValue *v;
	GError *error;

	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_INT))
				{
					ret = g_value_get_int (v);
				}
			else
				{
					ret = strtol (gda_value_stringify (v), NULL, 10);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_float_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #gfloat.
 */
gfloat
gdaex_data_model_get_value_float_at (GdaDataModel *data_model, gint row, gint col)
{
	gfloat ret = 0.0f;
	const GValue *v;
	GError *error;

	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_FLOAT))
				{
					ret = g_value_get_float (v);
				}
			else
				{
					ret = g_strtod (gda_value_stringify (v), NULL);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_double_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #gdouble.
 */
gdouble
gdaex_data_model_get_value_double_at (GdaDataModel *data_model, gint row, gint col)
{
	gdouble ret = 0.0;
	const GValue *v;
	GError *error;

	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_DOUBLE))
				{
					ret = g_value_get_double (v);
				}
			else
				{
					ret = g_strtod (gda_value_stringify (v), NULL);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_boolean_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #gboolean.
 */
gboolean
gdaex_data_model_get_value_boolean_at (GdaDataModel *data_model, gint row, gint col)
{
	gboolean ret = FALSE;
	const GValue *v;
	GError *error;

	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_BOOLEAN))
				{
					ret = g_value_get_boolean (v);
				}
			else
				{
					gchar *vstr = g_strstrip (gda_value_stringify (v));
					if (strcasecmp (vstr, "true") == 0 ||
						strcasecmp (vstr, "t") == 0 ||
						strcasecmp (vstr, "yes") == 0 ||
						strcasecmp (vstr, "y") == 0 ||
						strtol (vstr, NULL, 10) != 0)
						{
							ret = TRUE;
						}
					else
						{
							ret = FALSE;
						}
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_gdatimestamp_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #GdaTimestamp.
 */
GdaTimestamp
*gdaex_data_model_get_value_gdatimestamp_at (GdaDataModel *data_model, gint row, gint col)
{
	GdaTimestamp *gdatimestamp;
	const GValue *v;
	GError *error;

	gdatimestamp = NULL;
	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (v == NULL || error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = (GdaTimestamp *)gda_value_get_timestamp (v);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					GDateTime *gdatetime;
					
					gdatetime = gdaex_data_model_get_value_gdatetime_at (data_model, row, col);
					gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
					gdatimestamp->year = g_date_time_get_year (gdatetime);
					gdatimestamp->month = g_date_time_get_month (gdatetime);
					gdatimestamp->day = g_date_time_get_day_of_month (gdatetime);
					gdatimestamp->hour = g_date_time_get_hour (gdatetime);
					gdatimestamp->minute = g_date_time_get_minute (gdatetime);
					gdatimestamp->second = g_date_time_get_second (gdatetime);
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					GDate *date;

					date = gdaex_data_model_get_value_gdate_at (data_model, row, col);

					if (date != NULL)
						{
							gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
							if (g_date_valid (date))
								{
									gdatimestamp->year = g_date_get_year (date);
									gdatimestamp->month = g_date_get_month (date);
									gdatimestamp->day = g_date_get_day (date);
								}
							else
								{
									gdatimestamp->year = date->year;
									gdatimestamp->month = date->month;
									gdatimestamp->day = date->day;
								}
						}
				}
			else if (gda_value_isa (v, GDA_TYPE_TIME))
				{
					const GdaTime *time;

					time = gda_value_get_time (v);

					if (time != NULL)
						{
							gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
							gdatimestamp->hour = time->hour;
							gdatimestamp->minute = time->minute;
							gdatimestamp->second = time->second;
						}
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return (GdaTimestamp *)gda_timestamp_copy ((gpointer)gdatimestamp);
}

/**
 * gdaex_data_model_get_value_gdate_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #GDate without time information.
 */
GDate
*gdaex_data_model_get_value_gdate_at (GdaDataModel *data_model, gint row, gint col)
{
	GDate *ret;
	const GdaTimestamp *gdatimestamp;
	const GDateTime *gdatetime;
	const GValue *v;
	GError *error;

	ret = NULL;
	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (v == NULL || error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = gdaex_data_model_get_value_gdatimestamp_at (data_model, row, col);
					ret = g_date_new_dmy ((GDateYear)gdatimestamp->year,
					                      (GDateMonth)gdatimestamp->month,
					                      (GDateDay)gdatimestamp->day);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					gdatetime = gdaex_data_model_get_value_gdatetime_at (data_model, row, col);
					ret = g_date_new_dmy ((GDateYear)g_date_time_get_year ((GDateTime *)gdatetime),
					                      (GDateMonth)g_date_time_get_month ((GDateTime *)gdatetime),
					                      (GDateDay)g_date_time_get_day_of_month ((GDateTime *)gdatetime));
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					ret = (GDate *)g_value_get_boxed (v);
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_gdatetime_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the #GValue as #GDateTime without time information.
 */
GDateTime
*gdaex_data_model_get_value_gdatetime_at (GdaDataModel *data_model, gint row, gint col)
{
	GDateTime *ret;
	const GdaTimestamp *gdatimestamp;
	const GDate *gdate;
	const GValue *v;
	GError *error;

	ret = NULL;
	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (v == NULL || error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = gdaex_data_model_get_value_gdatimestamp_at (data_model, row, col);
					ret = g_date_time_new_local ((gint)gdatimestamp->year,
					                             (gint)gdatimestamp->month,
					                             (gint)gdatimestamp->day,
					                             (gint)gdatimestamp->hour,
					                             (gint)gdatimestamp->minute,
					                             (gdouble)gdatimestamp->second);
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					gdate = gdaex_data_model_get_value_gdate_at (data_model, row, col);
					ret = g_date_time_new_local ((gint)g_date_get_year (gdate),
					                             (gint)g_date_get_month (gdate),
					                             (gint)g_date_get_day (gdate),
					                             0,
					                             0,
					                             0.0);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					ret = (GDateTime *)g_value_get_boxed (v);
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_tm_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Returns: the field's value as a struct tm.
 */
struct tm
*gdaex_data_model_get_value_tm_at (GdaDataModel *data_model, gint row, gint col)
{
	struct tm *ret;
	const GValue *v;
	GError *error;

	ret = NULL;
	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (v == NULL || error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else if (!gda_value_is_null (v))
		{
			GdaTimestamp *gdatimestamp;

			gdatimestamp = gdaex_data_model_get_value_gdatimestamp_at (data_model, row, col);

			if (gdatimestamp != NULL)
				{
					ret = g_malloc0 (sizeof (struct tm));

					ret->tm_year = gdatimestamp->year - 1900;
					ret->tm_mon = gdatimestamp->month - 1;
					ret->tm_mday = gdatimestamp->day;
					ret->tm_hour = gdatimestamp->hour;
					ret->tm_min = gdatimestamp->minute;
					ret->tm_sec = gdatimestamp->second;
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_field_value_stringify_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gchar (stringify)
 */
gchar
*gdaex_data_model_iter_get_field_value_stringify_at (GdaDataModelIter *iter,
                                               const gchar *field_name)
{
	GdaDataModel *data_model;
	gchar *value;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_stringify_at (iter, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_integer_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gint
 */
gint
gdaex_data_model_iter_get_field_value_integer_at (GdaDataModelIter *iter,
                                            const gchar *field_name)
{
	GdaDataModel *data_model;
	gint value = 0;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_integer_at (iter, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_float_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gfloat
 */
gfloat
gdaex_data_model_iter_get_field_value_float_at (GdaDataModelIter *iter,
                                          const gchar *field_name)
{
	GdaDataModel *data_model;
	gfloat value = 0.0f;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_float_at (iter, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_double_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gdouble
 */
gdouble
gdaex_data_model_iter_get_field_value_double_at (GdaDataModelIter *iter,
                                           const gchar *field_name)
{
	GdaDataModel *data_model;
	gdouble value = 0.0;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_double_at (iter, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_boolean_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #gboolean
 */
gboolean
gdaex_data_model_iter_get_field_value_boolean_at (GdaDataModelIter *iter,
                                            const gchar *field_name)
{
	GdaDataModel *data_model;
	gboolean value = FALSE;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_boolean_at (iter, col);
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_gdatimestamp_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GdaTimestamp.
 */
GdaTimestamp
*gdaex_data_model_iter_get_field_value_gdatimestamp_at (GdaDataModelIter *iter,
                                           const gchar *field_name)
{
	GdaDataModel *data_model;
	const GdaTimestamp *value;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_gdatimestamp_at (iter, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return gda_timestamp_copy ((gpointer)value);
}

/**
 * gdaex_data_model_iter_get_field_value_gdate_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GDate.
 */
GDate
*gdaex_data_model_iter_get_field_value_gdate_at (GdaDataModelIter *iter,
                                           const gchar *field_name)
{
	GdaDataModel *data_model;
	GDate *value;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_gdate_at (iter, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_gdatetime_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's #GValue as #GDateTime.
 */
GDateTime
*gdaex_data_model_iter_get_field_value_gdatetime_at (GdaDataModelIter *iter,
                                           const gchar *field_name)
{
	GdaDataModel *data_model;
	GDateTime *value;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_gdatetime_at (iter, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_field_value_tm_at:
 * @iter: a #GdaDataModelIter object.
 * @field_name: the field's name.
 *
 * Returns: the @field_name's value as a struct tm.
 */
struct tm
*gdaex_data_model_iter_get_field_value_tm_at (GdaDataModelIter *iter,
                                           const gchar *field_name)
{
	GdaDataModel *data_model;
	struct tm *value;
	gint col;

	g_object_get (G_OBJECT (iter), "data-model", &data_model, NULL);

	col = gda_data_model_get_column_index (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_iter_get_value_tm_at (iter, col);
			if (value == NULL)
				{
					g_warning ("Error retrieving «%s»'s value.", field_name);
				}
		}
	else
		{
			g_warning ("No column found with name «%s».", field_name);
			value = NULL;
		}

	return value;
}

/**
 * gdaex_data_model_iter_get_value_stringify_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #gchar (stringify).
 */
gchar
*gdaex_data_model_iter_get_value_stringify_at (GdaDataModelIter *iter, gint col)
{
	gchar *ret;
	const GValue *v;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else
		{
			if (!gda_value_is_null (v))
				{
					ret = gda_value_stringify (v);
				}
			else
				{
					ret = g_strdup ("");
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_integer_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #gint.
 */
gint
gdaex_data_model_iter_get_value_integer_at (GdaDataModelIter *iter, gint col)
{
	gint ret = 0;
	const GValue *v;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_INT))
				{
					ret = g_value_get_int (v);
				}
			else
				{
					ret = strtol (gda_value_stringify (v), NULL, 10);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_float_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #gfloat.
 */
gfloat
gdaex_data_model_iter_get_value_float_at (GdaDataModelIter *iter, gint col)
{
	gfloat ret = 0.0f;
	const GValue *v;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_FLOAT))
				{
					ret = g_value_get_float (v);
				}
			else
				{
					ret = g_strtod (gda_value_stringify (v), NULL);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_double_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #gdouble.
 */
gdouble
gdaex_data_model_iter_get_value_double_at (GdaDataModelIter *iter, gint col)
{
	gdouble ret = 0.0;
	const GValue *v;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_DOUBLE))
				{
					ret = g_value_get_double (v);
				}
			else
				{
					ret = g_strtod (gda_value_stringify (v), NULL);
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_boolean_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #gboolean.
 */
gboolean
gdaex_data_model_iter_get_value_boolean_at (GdaDataModelIter *iter, gint col)
{
	gboolean ret = FALSE;
	const GValue *v;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else
		{
			if (gda_value_isa (v, G_TYPE_BOOLEAN))
				{
					ret = g_value_get_boolean (v);
				}
			else
				{
					gchar *vstr = g_strstrip (gda_value_stringify (v));
					if (strcasecmp (vstr, "true") == 0 ||
						strcasecmp (vstr, "t") == 0 ||
						strcasecmp (vstr, "yes") == 0 ||
						strcasecmp (vstr, "y") == 0 ||
						strtol (vstr, NULL, 10) != 0)
						{
							ret = TRUE;
						}
					else
						{
							ret = FALSE;
						}
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_gdatimestamp_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #GdaTimestamp.
 */
GdaTimestamp
*gdaex_data_model_iter_get_value_gdatimestamp_at (GdaDataModelIter *iter, gint col)
{
	GdaTimestamp *gdatimestamp;
	const GValue *v;

	gdatimestamp = NULL;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = (GdaTimestamp *)gda_value_get_timestamp (v);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					GDateTime *gdatetime;
					
					gdatetime = gdaex_data_model_iter_get_value_gdatetime_at (iter, col);
					gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
					gdatimestamp->year = g_date_time_get_year (gdatetime);
					gdatimestamp->month = g_date_time_get_month (gdatetime);
					gdatimestamp->day = g_date_time_get_day_of_month (gdatetime);
					gdatimestamp->hour = g_date_time_get_hour (gdatetime);
					gdatimestamp->minute = g_date_time_get_minute (gdatetime);
					gdatimestamp->second = g_date_time_get_second (gdatetime);
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					GDate *date;

					date = gdaex_data_model_iter_get_value_gdate_at (iter, col);

					if (date != NULL)
						{
							gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
							if (g_date_valid (date))
								{
									gdatimestamp->year = g_date_get_year (date);
									gdatimestamp->month = g_date_get_month (date);
									gdatimestamp->day = g_date_get_day (date);
								}
							else
								{
									gdatimestamp->year = date->year;
									gdatimestamp->month = date->month;
									gdatimestamp->day = date->day;
								}
						}
				}
			else if (gda_value_isa (v, GDA_TYPE_TIME))
				{
					const GdaTime *time;

					time = gda_value_get_time (v);

					if (time != NULL)
						{
							gdatimestamp = g_malloc0 (sizeof (GdaTimestamp));
							gdatimestamp->hour = time->hour;
							gdatimestamp->minute = time->minute;
							gdatimestamp->second = time->second;
						}
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return (GdaTimestamp *)gda_timestamp_copy ((gpointer)gdatimestamp);
}

/**
 * gdaex_data_model_iter_get_value_gdate_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #GDate without time information.
 */
GDate
*gdaex_data_model_iter_get_value_gdate_at (GdaDataModelIter *iter, gint col)
{
	GDate *ret;
	const GdaTimestamp *gdatimestamp;
	const GDateTime *gdatetime;
	const GValue *v;

	ret = NULL;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = gdaex_data_model_iter_get_value_gdatimestamp_at (iter, col);
					ret = g_date_new_dmy ((GDateYear)gdatimestamp->year,
					                      (GDateMonth)gdatimestamp->month,
					                      (GDateDay)gdatimestamp->day);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					gdatetime = gdaex_data_model_iter_get_value_gdatetime_at (iter, col);
					ret = g_date_new_dmy ((GDateYear)g_date_time_get_year ((GDateTime *)gdatetime),
					                      (GDateMonth)g_date_time_get_month ((GDateTime *)gdatetime),
					                      (GDateDay)g_date_time_get_day_of_month ((GDateTime *)gdatetime));
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					ret = (GDate *)g_value_get_boxed (v);
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_gdatetime_at:
 * @data_model_iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the #GValue as #GDateTime without time information.
 */
GDateTime
*gdaex_data_model_iter_get_value_gdatetime_at (GdaDataModelIter *iter, gint col)
{
	GDateTime *ret;
	const GdaTimestamp *gdatimestamp;
	const GDate *gdate;
	const GValue *v;
	GError *error;

	ret = NULL;
	error = NULL;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = gdaex_data_model_iter_get_value_gdatimestamp_at (iter, col);
					ret = g_date_time_new_local ((gint)gdatimestamp->year,
					                             (gint)gdatimestamp->month,
					                             (gint)gdatimestamp->day,
					                             (gint)gdatimestamp->hour,
					                             (gint)gdatimestamp->minute,
					                             (gdouble)gdatimestamp->second);
				}
			else if (gda_value_isa (v, G_TYPE_DATE))
				{
					gdate = gdaex_data_model_iter_get_value_gdate_at (iter, col);
					ret = g_date_time_new_local ((gint)g_date_get_year (gdate),
					                             (gint)g_date_get_month (gdate),
					                             (gint)g_date_get_day (gdate),
					                             0,
					                             0,
					                             0.0);
				}
			else if (gda_value_isa (v, G_TYPE_DATE_TIME))
				{
					ret = (GDateTime *)g_value_get_boxed (v);
				}
			else
				{
					g_warning ("Error on retrieving field's value: unknown GValue type.");
				}
		}

	return ret;
}

/**
 * gdaex_data_model_iter_get_value_tm_at:
 * @iter: a #GdaDataModelIter object.
 * @col: col number.
 *
 * Returns: the field's value as a struct tm.
 */
struct tm
*gdaex_data_model_iter_get_value_tm_at (GdaDataModelIter *iter, gint col)
{
	struct tm *ret;
	const GValue *v;

	ret = NULL;

	v = gda_data_model_iter_get_value_at (iter, col);
	if (v == NULL)
		{
			g_warning ("Error on retrieving field's value at column %d.", col);
		}
	else if (!gda_value_is_null (v))
		{
			GdaTimestamp *gdatimestamp;

			gdatimestamp = gdaex_data_model_iter_get_value_gdatimestamp_at (iter, col);

			if (gdatimestamp != NULL)
				{
					ret = g_malloc0 (sizeof (struct tm));

					ret->tm_year = gdatimestamp->year - 1900;
					ret->tm_mon = gdatimestamp->month - 1;
					ret->tm_mday = gdatimestamp->day;
					ret->tm_hour = gdatimestamp->hour;
					ret->tm_min = gdatimestamp->minute;
					ret->tm_sec = gdatimestamp->second;
				}
		}

	return ret;
}

/**
 * gdaex_data_model_columns_to_hashtable:
 * @dm: a #GdaDataModel object.
 *
 * Returns: a #GHashTable with keys as the columns names from @dm,
 * and values as columns numbers.
 */
GHashTable
*gdaex_data_model_columns_to_hashtable (GdaDataModel *dm)
{
	GHashTable *ret;

	guint cols;
	guint col;

	g_return_val_if_fail (GDA_IS_DATA_MODEL (dm), NULL);

	ret = NULL;

	cols = gda_data_model_get_n_columns (dm);

	if (cols > 0)
		{
			ret = g_hash_table_new (g_str_hash, g_str_equal);

			for (col = 0; col < cols; col++)
				{
					g_hash_table_insert (ret,
					                     g_strdup (gda_data_model_get_column_name (dm, col)),
					                     g_strdup_printf ("%d", col));
				}
		}

	return ret;
}

/**
 * gdaex_data_model_to_gtkliststore:
 * @dm: a #GdaDataModel object.
 * @only_schema:
 *
 * Returns: a #GtkListStore, filled or not, that reflects the #GdaDataModel @dm.
 */
GtkListStore
*gdaex_data_model_to_gtkliststore (GdaDataModel *dm,
                                   gboolean only_schema)
{
	GtkListStore *ret;

	guint cols;
	guint col;

	GdaColumn *column;
	GType *gtypes;

	guint rows;
	guint row;

	GtkTreeIter iter;
	GValue *gval;

	g_return_val_if_fail (GDA_IS_DATA_MODEL (dm), NULL);

	ret = NULL;

	/* GtkListStore creation */

	cols = gda_data_model_get_n_columns (dm);
	if (cols == 0)
		{
			g_warning ("Invalid GdaDataModel.");
			return NULL;
		}

	gtypes = (GType *)g_malloc0 (cols * sizeof (GType));

	for (col = 0; col < cols; col++)
		{
			column = gda_data_model_describe_column (dm, col);
			gtypes[col] = gda_column_get_g_type (column);
		}
	ret = gtk_list_store_newv (cols, gtypes);
	if (ret == NULL)
		{
			g_warning ("Unable to create the GtkTreeModel.");
			return NULL;
		}

	if (!only_schema)
		{
			/* Filling GtkListStore */
			rows = gda_data_model_get_n_rows (dm);
			for (row = 0; row < rows; row++)
				{
					gtk_list_store_append (ret, &iter);
					for (col = 0; col < cols; col++)
						{
							gval = (GValue *)gda_data_model_get_value_at (dm, col, row, NULL);
							gtk_list_store_set_value (ret, &iter, col, gval);
						}
				}
		}

	return ret;
}

/**
 * gdaex_begin:
 * @gdaex: a #GdaEx object.
 *
 * Begin a new transaction.
 *
 * Returns: TRUE if there isn't errors.
 */
gboolean
gdaex_begin (GdaEx *gdaex)
{
	GError *error;
	gboolean ret;

	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	error = NULL;
	ret = gda_connection_begin_transaction (priv->gda_conn, "gdaex",
	                                        GDA_TRANSACTION_ISOLATION_SERIALIZABLE,
	                                        &error);

	if (error != NULL)
		{
			g_warning ("Error opening transaction: %s\n",
			           error->message != NULL ? error->message : "no details");
		}
	else
		{
			if (priv->debug > 0)
				{
					g_message ("Transaction opened.");
				}
		}

	return ret;
}

/**
 * gdaex_execute:
 * @gdaex: a #GdaEx object.
 * @sql: the sql text.
 *
 * Execute a command query (INSERT, UPDATE, DELETE).
 *
 * Returns: number of records affected by the query execution.
 */
gint
gdaex_execute (GdaEx *gdaex, const gchar *sql)
{
	GdaStatement *stmt;
	GError *error;

	const gchar *remain;
	gint nrecs;

	g_return_val_if_fail (IS_GDAEX (gdaex), -1);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);
	GdaExClass *klass = GDAEX_GET_CLASS (gdaex);

	error = NULL;
	stmt = gda_sql_parser_parse_string (priv->gda_parser, sql, &remain, &error);
	if (remain)
		{
			g_warning ("REMAINS:\n%s\nfrom\n%s", remain, sql);
		}

	if (error != NULL)
		{
			g_object_unref (stmt);
			g_warning ("Error parsing sql: %s\n%s\n",
			           error->message != NULL ? error->message : "no details", sql);
			return -1;
		}

	g_signal_emit (gdaex, klass->before_execute_signal_id, 0, stmt);

	if (priv->tables_name_prefix != NULL
	    && g_strcmp0 (priv->tables_name_prefix, "") != 0)
		{
			gdaex_set_tables_name_prefix_into_statement (gdaex, &stmt);
		}

	error = NULL;
	nrecs = gda_connection_statement_execute_non_select (priv->gda_conn, stmt, NULL, NULL, &error);

	if (error != NULL)
		{
			g_warning ("Error executing command query: %s\n%s",
			           error->message != NULL ? error->message : "no details", sql);
			return -1;
		}
	else
		{
			if (priv->debug > 0)
				{
					g_message ("Query executed: %s", sql);
				}
		}

	g_signal_emit (gdaex, klass->after_execute_signal_id, 0, stmt);

	g_object_unref (stmt);

	return nrecs;
}

/**
 * gdaex_batch_execute:
 * @gdaex: a #GdaEx object.
 * @...: a #NULL terminated list of sql texts.
 *
 */
GSList
*gdaex_batch_execute (GdaEx *gdaex, ...)
{
	GSList *ret;

	va_list ap;

	gchar *sql;
	GdaStatement *stmt;
	GError *error;

	GdaDataModel *dm;
	gint recs;

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	ret = NULL;

	va_start (ap, gdaex);

	while ((sql = va_arg (ap, gchar *)) != NULL)
		{
			error = NULL;
			stmt = gda_sql_parser_parse_string (priv->gda_parser, sql, NULL, &error);
			if (error != NULL)
				{
					g_warning ("Error parsing sql: %s\n%s\n",
					           error->message != NULL ? error->message : "no details", sql);
					return NULL;
				}

			if (gda_statement_get_statement_type (stmt) == GDA_SQL_STATEMENT_SELECT)
				{
					dm = gdaex_query (gdaex, sql);
					ret = g_slist_append (ret, dm);
				}
			else
				{
					recs = gdaex_execute (gdaex, sql);
					ret = g_slist_append (ret, GINT_TO_POINTER (recs));
				}
		}

	va_end (ap);

	return ret;
}

/**
 * gdaex_commit:
 * @gdaex: a #GdaEx object.
 *
 * Commit a transaction.
 *
 * Returns: TRUE if there isn't errors.
 */
gboolean
gdaex_commit (GdaEx *gdaex)
{
	gboolean ret;
	GError *error;
	GdaExPrivate *priv;
	GdaTransactionStatus *tstatus;

	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	priv = GDAEX_GET_PRIVATE (gdaex);

	tstatus = gda_connection_get_transaction_status (priv->gda_conn);

	if (tstatus == NULL)
		{
			ret = TRUE;
			if (priv->debug > 0)
				{
					g_message ("No transaction opened.");
				}
		}
	else
		{
			error = NULL;
			ret = gda_connection_commit_transaction (priv->gda_conn, "gdaex", &error);

			if (error != NULL)
				{
					g_warning ("Error committing transaction: %s\n",
							   error->message);
					ret = FALSE;
				}
			else
				{
					if (priv->debug > 0)
						{
							g_message ("Transaction committed.");
						}
				}
		}

	return ret;
}

/**
 * gdaex_rollback:
 * @gdaex: a #GdaEx object.
 *
 * Rollback a transaction.
 *
 * Returns: TRUE if there isn't errors.
 */
gboolean
gdaex_rollback (GdaEx *gdaex)
{
	gboolean ret;
	GError *error;
	GdaExPrivate *priv;
	GdaTransactionStatus *tstatus;

	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	priv = GDAEX_GET_PRIVATE (gdaex);

	tstatus = gda_connection_get_transaction_status (priv->gda_conn);

	if (tstatus == NULL)
		{
			ret = TRUE;
			if (priv->debug > 0)
				{
					g_message ("No transaction opened.");
				}
		}
	else
		{
			error = NULL;
			ret = gda_connection_rollback_transaction (priv->gda_conn, "gdaex", &error);

			if (error != NULL)
				{
					g_warning ("Error rollbacking transaction: %s\n",
							   error->message);
					ret = FALSE;
				}
			else
				{
					if (priv->debug > 0)
						{
							g_message ("Transaction rolled back.");
						}
				}
		}

	return ret;
}

/* TO DO */
void
gdaex_free (GdaEx *gdaex)
{
	g_return_if_fail (IS_GDAEX (gdaex));

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	/* close connection */
	if (gda_connection_is_opened (priv->gda_conn))
		{
			gda_connection_close (priv->gda_conn);
		}

	if (priv->log_file != NULL)
		{
			g_output_stream_close (G_OUTPUT_STREAM (priv->log_file), NULL, NULL);
			g_object_unref (priv->log_file);
		}
}

/* UTILITY'S FUNCTIONS */
/**
 * gdaex_strescape:
 * @source: a string to escape.
 * @exceptions: a string of characters not to escape in @source.
 *
 * As g_strescape(), but it escapes also '.
 */
gchar
*gdaex_strescape (const gchar *source, const gchar *exceptions)
{
	gchar *nsource;

	if (source == NULL)
		{
			nsource = g_strdup ("");
		}
	else
		{
			nsource = gda_default_escape_string (source);
		}

	return nsource;
}

/**
 * gdaex_get_chr_quoting:
 * @gdaex: a #GdaEx object.
 *
 */
gchar
gdaex_get_chr_quoting (GdaEx *gdaex)
{
	gchar chr = '\"';

	const gchar *provider;

	g_return_val_if_fail (IS_GDAEX (gdaex), chr);

	provider = gdaex_get_provider (gdaex);

	if (strcmp (provider, "MySQL") == 0)
		{
			chr = '`';
		}

	return chr;
}

void
gdaex_fill_liststore_from_sql (GdaEx *gdaex,
                               GtkListStore *lstore,
                               const gchar *sql,
                               guint *cols_formatted,
                               gchar *(*cols_format_func) (GdaDataModelIter *, guint))
{
	GdaDataModel *dm;

	gchar *_sql;

	g_return_if_fail (IS_GDAEX (gdaex));
	g_return_if_fail (sql != NULL);

	_sql = g_strstrip (g_strdup (sql));

	g_return_if_fail (g_strcmp0 (_sql, "") != 0);

	dm = gdaex_query (gdaex, _sql);
	gdaex_fill_liststore_from_datamodel (gdaex, lstore, dm, cols_formatted, cols_format_func);
}

void
gdaex_fill_liststore_from_datamodel (GdaEx *gdaex,
                                     GtkListStore *lstore,
                                     GdaDataModel *dm,
                                     guint *cols_formatted,
                                     gchar *(*cols_format_func) (GdaDataModelIter *, guint))
{
	GtkTreeIter iter;

	GdaDataModelIter *gda_iter;

	guint cols;
	guint col;

	GType col_gtype;

	GdaColumn *gcol;
	GType gcol_gtype;

	gint *columns;
	GValue *values;

	gdouble dval;
	GDateTime *gdatetime;

	g_return_if_fail (IS_GDAEX (gdaex));
	g_return_if_fail (GTK_IS_LIST_STORE (lstore));
	g_return_if_fail (GDA_IS_DATA_MODEL (dm));

	cols = gtk_tree_model_get_n_columns (GTK_TREE_MODEL (lstore));

	gtk_list_store_clear (lstore);

	gda_iter = gda_data_model_create_iter (dm);
	if (gda_iter == NULL)
		{
			return;
		}

	columns = g_malloc0 (cols * sizeof (gint));
	values = g_malloc0 (cols * sizeof (GValue));

	while (gda_data_model_iter_move_next (gda_iter))
		{
			gtk_list_store_append (lstore, &iter);

			for (col = 0; col < cols; col++)
				{
					columns[col] = col;

					col_gtype = gtk_tree_model_get_column_type (GTK_TREE_MODEL (lstore), col);

					GValue gval = {0};
					g_value_init (&gval, col_gtype);
					switch (col_gtype)
						{
							case G_TYPE_STRING:
								gcol = gda_data_model_describe_column (dm, col);
								gcol_gtype = gda_column_get_g_type (gcol);

								switch (gcol_gtype)
									{
										case G_TYPE_STRING:
											g_value_set_string (&gval, gdaex_data_model_iter_get_value_stringify_at (gda_iter, col));
											break;

										case G_TYPE_BOOLEAN:
											g_value_set_string (&gval, gdaex_data_model_iter_get_value_boolean_at (gda_iter, col) ? "X" : "");
											break;

										case G_TYPE_INT:
										case G_TYPE_FLOAT:
										case G_TYPE_DOUBLE:
											dval = gdaex_data_model_iter_get_value_double_at (gda_iter, col);
											g_value_set_string (&gval, gdaex_format_money (dval, -1, FALSE));
											break;

										default:
											if (cols_format_func != NULL)
												{
													g_value_set_string (&gval, (*cols_format_func) (gda_iter, col));
												}
											else if (gcol_gtype == G_TYPE_DATE
											         || gcol_gtype == GDA_TYPE_TIMESTAMP
											         || gcol_gtype == G_TYPE_DATE_TIME)
												{
													gdatetime = gdaex_data_model_iter_get_value_gdatetime_at (gda_iter, col);
													/* TODO find default format from locale */
													g_value_set_string (&gval, g_date_time_format (gdatetime, gcol_gtype == G_TYPE_DATE ? "%d/%m/%Y" : "%d/%m/%Y %H.%M.%S"));
												}
											else
												{
													g_value_set_string (&gval, gda_value_stringify (gda_data_model_iter_get_value_at (gda_iter, col)));
												}
											break;
									}

								values[col] = gval;
								break;

							case G_TYPE_INT:
								g_value_set_int (&gval, gdaex_data_model_iter_get_value_integer_at (gda_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_FLOAT:
								g_value_set_float (&gval, gdaex_data_model_iter_get_value_float_at (gda_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_DOUBLE:
								g_value_set_double (&gval, gdaex_data_model_iter_get_value_double_at (gda_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_BOOLEAN:
								g_value_set_boolean (&gval, gdaex_data_model_iter_get_value_boolean_at (gda_iter, col));
								values[col] = gval;
								break;

							default:
								values[col] = *gda_value_new_from_string (gdaex_data_model_iter_get_value_stringify_at (gda_iter, col), col_gtype);
								break;
						}
				}

			gtk_list_store_set_valuesv (lstore, &iter, columns, values, cols);
		}
}

gchar
*gdaex_format_money (gdouble number,
                     gint decimals,
                     gboolean with_currency_symbol)
{
	gchar *ret;

	GRegex *regex;
	GError *error;

	gchar *str_format;
	gchar *str;
	gssize str_len;

	/* TODO
	 * - get number of decimals from locale
	 * - get grouping char from locale
	 * - get currency symbol from locale
	 */

	ret = g_strdup ("");

	error = NULL;
	regex = g_regex_new ("(^[-\\d]?\\d+)(\\d\\d\\d)", 0, 0, &error);
	if (error != NULL)
		{
			g_warning ("Error on creating regex: %s.",
			           error->message != NULL ? error->message : "no details");
			return "";
		}

	str_format = g_strdup_printf ("%%0%sf", decimals == 0 ? ".0" : (decimals < 0 ? ".2" : g_strdup_printf (".%d", decimals)));
	ret = g_strdup_printf (str_format, number);

	while (TRUE)
		{
			error = NULL;
			str_len = g_utf8_strlen (ret, -1);
			str = g_regex_replace ((const GRegex *)regex,
			                       ret, str_len, 0,
			                       "\\1.\\2", 0,
			                       &error);
			if (error != NULL)
				{
					g_warning ("Error on regex replacing: %s.",
					           error->message != NULL ? error->message : "no details");
					g_regex_unref (regex);
					return "";
				}
			if (g_strcmp0 (ret, str) != 0)
				{
					ret = g_strdup (str);
					g_free (str);
				}
			else
				{
					break;
				}
		}

	if (with_currency_symbol)
		{
			ret = g_strconcat ("€ ", ret, NULL);
		}

	g_regex_unref (regex);

	return ret;
}

/* PRIVATE */
static void
gdaex_create_connection_parser (GdaEx *gdaex)
{
	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	priv->gda_parser = gda_connection_create_parser (priv->gda_conn);
	if (priv->gda_parser == NULL) /* @gda_conn doe snot provide its own parser => use default one */
		{
			priv->gda_parser = gda_sql_parser_new ();
		}
}

static void
gdaex_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaEx *gdaex = GDAEX (object);
	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaEx *gdaex = GDAEX (object);
	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
