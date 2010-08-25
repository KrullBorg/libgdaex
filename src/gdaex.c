/*
 *  gdaex.c
 *
 *  Copyright (C) 2005-2010 Andrea Zagli <azagli@libero.it>
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
 *  along with libgdaexbj; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>
#include <libgda/sql-parser/gda-sql-parser.h>

#include "libgdaex.h"

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
			g_warning ("Error creating database connection: %s\n",
			           error->message);
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

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	error = NULL;
	GdaStatement *stmt;
	stmt = gda_sql_parser_parse_string (priv->gda_parser, sql, NULL, &error);
	if (!GDA_IS_STATEMENT (stmt))
		{
			g_warning ("Error parsing query string: %s\n%s\n",
			           error != NULL && error->message != NULL ? error->message : "no details", sql);
			return NULL;
		}

	error = NULL;
	GdaDataModel *dm = gda_connection_statement_execute_select (priv->gda_conn, stmt, NULL, &error);
	if (!GDA_IS_DATA_MODEL (dm))
		{
			g_warning ("Error executing selection query: %s\n%s\n",
			           error != NULL && error->message != NULL ? error->message : "no details", sql);
			return NULL;
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
			           error->message);
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
			           error->message);
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
			           error->message);
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
			           error->message);
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
			           error->message);
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
			           error->message);
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = (GdaTimestamp *)gda_value_get_timestamp (v);
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
	const GValue *v;
	GError *error;

	ret = NULL;
	error = NULL;

	v = gda_data_model_get_value_at (data_model, col, row, &error);
	if (v == NULL || error != NULL)
		{
			g_warning ("Error on retrieving field's value: %s\n",
			           error->message);
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_TYPE_TIMESTAMP))
				{
					gdatimestamp = gdaex_data_model_get_value_gdatimestamp_at (data_model, row, col);
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
			           error->message);
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
			           error->message);
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
			g_warning ("Error parsing sql: %s\n%s\n",
			           error->message, sql);
			return -1;
		}

	g_signal_emit (gdaex, klass->before_execute_signal_id, 0, stmt);

	error = NULL;
	nrecs = gda_connection_statement_execute_non_select (priv->gda_conn, stmt, NULL, NULL, &error);

	if (error != NULL)
		{
			g_warning ("Error executing command query: %s\n%s\n",
			           error->message, sql);
			return -1;
		}

	g_signal_emit (gdaex, klass->after_execute_signal_id, 0, stmt);

	return nrecs;
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
