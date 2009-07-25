/*
 *  gdaex.c
 *
 *  Copyright (C) 2005-2009 Andrea Zagli <azagli@libero.it>
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

#include "libgdaex.h"

static void gdaex_class_init (GdaExClass *klass);
static void gdaex_init (GdaEx *gdaex);

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
		GdaTransaction *gda_trans;
	};

G_DEFINE_TYPE (GdaEx, gdaex, G_TYPE_OBJECT)

static void
gdaex_class_init (GdaExClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExPrivate));

	object_class->set_property = gdaex_set_property;
	object_class->get_property = gdaex_get_property;
}

static void
gdaex_init (GdaEx *gdaex)
{
	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	priv->gda_trans = NULL;
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
 * Return value: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_dsn (const gchar *dsn,
                     const gchar *username,
                     const gchar *password)
{
	GdaEx *gdaex;
	GdaExPrivate *priv;

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

	/* open database connection */
	error = NULL;
	priv->gda_conn = gda_connection_open_from_dsn (dsn,
	                                               username,
                                                   password,
	                                               GDA_CONNECTION_OPTIONS_NONE,
	                                               &error);
	if (error != NULL)
		{
			g_warning ("Errror creating database connection: %s\n",
			           error->message);
			return NULL;
		}

	return gdaex;
}

/**
 * gdaex_new_from_string:
 * @cnc_string: the connection string.
 *
 * Return value: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_string (const gchar *cnc_string)
{
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
	priv->gda_conn = gda_connection_open_from_string (NULL,
	                                                  cnc_string,
	                                                  NULL,
	                                                  GDA_CONNECTION_OPTIONS_NONE
                                                      &error);
	if (error != NULL)
		{
			g_warning ("Errror creating database connection: %s\n",
			           error->message);
			return NULL;
		}

	return gdaex;
}

/**
 * gdaex_new_from_connection:
 * @conn: a #GdaConnection.
 *
 * Returns a #GdaEx from an existing #GdaConnection.
 *
 * Return value: the newly created #GdaEx.
 */
GdaEx
*gdaex_new_from_connection (GdaConnection *conn)
{
	GdaEx *gdaex;
	GdaExPrivate *priv;

	g_return_val_if_fail (IS_GDA_CONNECTION (conn), NULL);

	gdaex = gdaex_new_ ();

	priv = GDAEX_GET_PRIVATE (gdaex);

	priv->gda_conn = conn;

	return gdaex;
}

/**
 * gdaex_get_gdaconnection:
 * @gdaex: a #GdaEx object.
 *
 * Return value: the #GdaConnection associated to the #GdaEx.
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
 * Return value: the provider id associated to the #GdaEx.
 */
const gchar
*gdaex_get_provider (GdaEx *gdaex)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	return gda_connection_get_provider (priv->gda_conn);
}

/**
 * gdaex_query:
 * @gdaex: a #GdaEx object.
 * @sql: the sql text.
 *
 * Execute a selection query (SELECT).
 *
 * Return value: a #GdaDataModel, or NULL if query fails.
 */
GdaDataModel
*gdaex_query (GdaEx *gdaex, const gchar *sql)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	GdaCommand *gda_comm = gda_command_new (sql, GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);
	GdaDataModel *dm = gda_connection_execute_single_command (priv->gda_conn, gda_comm, NULL);

	if (dm == NULL)
		{
			g_fprintf (stderr, "SQL: %s\n", sql);
			get_errors (priv->gda_conn);
		}

	gda_command_free (gda_comm);

	return dm;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gchar (stringify)
 */
gchar
*gdaex_data_model_get_field_value_stringify_at (GdaDataModel *data_model,
                                               gint row,
                                               const gchar *field_name)
{
	gchar *value = "";
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_stringify_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gint
 */
gint
gdaex_data_model_get_field_value_integer_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gint value = 0;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_integer_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gfloat
 */
gfloat
gdaex_data_model_get_field_value_float_at (GdaDataModel *data_model,
                                          gint row,
                                          const gchar *field_name)
{
	gfloat value = 0.0f;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_float_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gdouble
 */
gdouble
gdaex_data_model_get_field_value_double_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	gdouble value = 0.0;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_double_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gboolean
 */
gboolean
gdaex_data_model_get_field_value_boolean_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gboolean value = FALSE;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_boolean_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as GDate
 */
GDate
*gdaex_data_model_get_field_value_gdate_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	GDate *value = NULL;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdaex_data_model_get_value_gdate_at (data_model, row, col);
		}

	return value;
}

/**
 * gdaex_data_model_get_value_stringify_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gchar (stringify).
 */
gchar
*gdaex_data_model_get_value_stringify_at (GdaDataModel *data_model, gint row, gint col)
{
	gchar *ret = "";

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (!gda_value_is_null (v))
		{
			ret = g_strdup (gda_value_stringify (v));
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_integer_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gint.
 */
gint
gdaex_data_model_get_value_integer_at (GdaDataModel *data_model, gint row, gint col)
{
	gint ret = 0;

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (v == NULL)
		{
			/* TO DO */
			/* trap errors */
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_VALUE_TYPE_INTEGER))
				{
					ret = gda_value_get_integer (v);
				}
			else
				{
					ret = atol (gda_value_stringify (v));
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
 * Return value: the #GdaValue as #gfloat.
 */
gfloat
gdaex_data_model_get_value_float_at (GdaDataModel *data_model, gint row, gint col)
{
	gfloat ret = 0.0f;

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (!gda_value_is_null (v))
		{
			ret = gda_value_get_single (v);
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_double_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gdouble.
 */
gdouble
gdaex_data_model_get_value_double_at (GdaDataModel *data_model, gint row, gint col)
{
	gdouble ret = 0.0;

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (!gda_value_is_null (v))
		{
			ret = gda_value_get_double (v);
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_boolean_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gboolean.
 */
gboolean
gdaex_data_model_get_value_boolean_at (GdaDataModel *data_model, gint row, gint col)
{
	gboolean ret = FALSE;

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (v == NULL)
		{
			/* TO DO */
			/* trap errors */
		}
	else if (!gda_value_is_null (v))
		{
			if (gda_value_isa (v, GDA_VALUE_TYPE_BOOLEAN))
				{
					ret = gda_value_get_boolean (v);
				}
			else
				{
					gchar *vstr = g_strstrip (gda_value_stringify (v));
					if (strcasecmp (vstr, "true") == 0 ||
					    strcasecmp (vstr, "t") == 0 ||
					    strcasecmp (vstr, "yes") == 0 ||
					    strcasecmp (vstr, "y") == 0 ||
					    atol (vstr) != 0)
						{
							ret = TRUE;
						}
				}
		}

	return ret;
}

/**
 * gdaex_data_model_get_value_gdate_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #GDate.
 */
GDate
*gdaex_data_model_get_value_gdate_at (GdaDataModel *data_model, gint row, gint col)
{
	GDate *ret = NULL;
	const GdaDate *gdadate;

	GdaValue *v = (GdaValue *)gda_data_model_get_value_at (data_model, col, row);
	if (!gda_value_is_null (v))
		{
			gdadate = gda_value_get_date (v);
			if (g_date_valid_dmy ((GDateDay)gdadate->day, (GDateMonth)gdadate->month, (GDateYear)gdadate->year))
				{
					ret = g_date_new_dmy ((GDateDay)gdadate->day, (GDateMonth)gdadate->month, (GDateYear)gdadate->year);
				}
		}

	return ret;
}

/**
 * gdaex_begin:
 * @gdaex: a #GdaEx object.
 *
 * Begin a new transaction.
 */
gboolean
gdaex_begin (GdaEx *gdaex)
{
	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	priv->gda_trans = gda_transaction_new ("gdaex");
	gda_transaction_set_isolation_level (priv->gda_trans,
	                                     GDA_TRANSACTION_ISOLATION_SERIALIZABLE);

	return gda_connection_begin_transaction (priv->gda_conn, priv->gda_trans);
}

/**
 * gdaex_execute:
 * @gdaex: a #GdaEx object.
 * @sql: the sql text.
 *
 * Execute a command query (INSERT, UPDATE, DELETE).
 *
 * Return value: number of records affected by the query execution.
 */
gint
gdaex_execute (GdaEx *gdaex, const gchar *sql)
{
	GdaCommand *gda_comm;
	gint nrecs;

	g_return_val_if_fail (IS_GDAEX (gdaex), -1);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	gda_comm = gda_command_new (sql, GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);
	if (priv->gda_trans != NULL)
		{
			gda_command_set_transaction (gda_comm, priv->gda_trans);
		}
	nrecs = gda_connection_execute_non_query (priv->gda_conn, gda_comm, NULL);

	if (nrecs == -1)
		{
			g_fprintf (stderr, "SQL: %s\n", sql);
			get_errors (priv->gda_conn);
		}

	gda_command_free (gda_comm);

	return nrecs;
}

/**
 * gdaex_commit:
 * @gdaex: a #GdaEx object.
 *
 * Commit a transaction.
 */
gboolean
gdaex_commit (GdaEx *gdaex)
{
	gboolean ret;

	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	if (priv->gda_trans == NULL)
		{
			ret = FALSE;
		}
	else
		{
			ret = gda_connection_commit_transaction (priv->gda_conn, priv->gda_trans);
			g_object_unref (priv->gda_trans);
			priv->gda_trans = NULL;
		}

	return ret;
}

/**
 * gdaex_rollback:
 * @gdaex: a #GdaEx object.
 *
 * Rollback a transaction.
 */
gboolean
gdaex_rollback (GdaEx *gdaex)
{
	gboolean ret;

	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);

	GdaExPrivate *priv = GDAEX_GET_PRIVATE (gdaex);

	if (priv->gda_trans == NULL)
		{
			ret = FALSE;
		}
	else
		{
			ret = gda_connection_rollback_transaction (priv->gda_conn, priv->gda_trans);
			g_object_unref (priv->gda_trans);
			priv->gda_trans = NULL;
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
	if (gda_connection_is_open (priv->gda_conn))
		{
			gda_connection_close (priv->gda_conn);
		}

	/*g_free (gdaex);*/
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
	gint l;

	l = strlen (source);

	if (source == NULL || l == 0) return "";

	nsource = g_strstrip (g_strdup (source));
	nsource = g_strescape (nsource, exceptions);

	return gda_default_escape_string (nsource);
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

	const gchar *provider = gdaex_get_provider (gdaex);

	if (strcmp (provider, "MySQL") == 0)
		{
			chr = '`';
		}

	return chr;
}

/* PRIVATE */
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
