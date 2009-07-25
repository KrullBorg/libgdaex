/*
 *  gdaex.c
 *
 *  Copyright (C) 2005-2009 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex.
 *  
 *  libgdaobj is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaobj is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaobj; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>

#include "libgdaex.h"

static void gdao_class_init (GdaOClass *klass);
static void gdao_init (GdaO *gdao);

static void gdao_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdao_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

static void get_errors (GdaConnection *connection);

#define GDAO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAO, GdaOPrivate))

typedef struct _GdaOPrivate GdaOPrivate;
struct _GdaOPrivate
	{
		GdaClient *gda_client;
		GdaConnection *gda_conn;
		GdaTransaction *gda_trans;
	};

GType
gdao_get_type (void)
{
	static GType gdao_type = 0;

	if (!gdao_type)
		{
			static const GTypeInfo gdao_info =
			{
				sizeof (GdaOClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) gdao_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (GdaO),
				0,	/* n_preallocs */
				(GInstanceInitFunc) gdao_init,
				NULL
			};

			gdao_type = g_type_register_static (G_TYPE_OBJECT, "GdaO",
			                                    &gdao_info, 0);
		}

	return gdao_type;
}

static void
gdao_class_init (GdaOClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaOPrivate));

	object_class->set_property = gdao_set_property;
	object_class->get_property = gdao_get_property;
}

static void
gdao_init (GdaO *gdao)
{
	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	priv->gda_trans = NULL;
}

static GdaO
*gdao_new_ (GdaClient *gda_client)
{
	GdaO *gdao = GDAO (g_object_new (gdao_get_type (), NULL));

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	if (gda_client == NULL)
		{
		  /* create a new GdaClient */
			priv->gda_client = gda_client_new ();
			if (priv->gda_client == NULL)
				{
					g_warning ("Unable to create a libgda's client.");
					return NULL;
				}
		}
	else
		{
			priv->gda_client = gda_client;
		}

	return gdao;
}

/**
 * gdao_new:
 * @gda_client: a #GdaClient object. If it's %NULL, it will be created a new one.
 * @datasource: GDA data source name to connect to.
 * @username: user name to use to connect.
 * @password: password for @username.
 *
 * If @username and @password are both NULL or empty, it will be used those
 * defined into datasource.
 *
 * Return value: the newly created #GdaO.
 */
GdaO
*gdao_new (GdaClient *gda_client,
           const gchar *datasource,
           const gchar *username,
           const gchar *password)
{
	GdaO *gdao;
	GdaOPrivate *priv;

	if (datasource == NULL || strcmp (g_strstrip (g_strdup (datasource)), "") == 0)
		{
		  /* TO DO */
			g_warning ("datasource must not be empty.");
			return NULL;
		}

	gdao = gdao_new_ (gda_client);
	if (gdao == NULL)
		{
			/* TO DO */
			return NULL;
		}

	priv = GDAO_GET_PRIVATE (gdao);

	/* open database connection */
	priv->gda_conn = gda_client_open_connection (priv->gda_client,
                                         datasource,
																				 username,
																				 password,
                                         0);
	if (priv->gda_conn == NULL)
		{
			g_warning ("Unable to create the connection.");
			return NULL;
		}

	return gdao;
}

/**
 * gdao_new_from_string:
 * @gda_client: a #GdaClient object. If it's %NULL, it will be created a new one.
 * @provider_id: the provider id.
 * @cnc_string: the connection string.
 *
 * Return value: the newly created #GdaO.
 */
GdaO
*gdao_new_from_string (GdaClient *gda_client,
                       const gchar *provider_id,
                       const gchar *cnc_string)
{
	GdaO *gdao;
	GdaOPrivate *priv;

	if (provider_id == NULL || strcmp (g_strstrip (g_strdup (provider_id)), "") == 0)
		{
		  /* TO DO */
			g_warning ("provider_id must not be empty.");
			return NULL;
		}

	if (cnc_string == NULL || strcmp (g_strstrip (g_strdup (cnc_string)), "") == 0)
		{
		  /* TO DO */
			g_warning ("cnc_string must not be empty.");
			return NULL;
		}

	gdao = gdao_new_ (gda_client);
	if (gdao == NULL)
		{
			/* TO DO */
			return NULL;
		}

	priv = GDAO_GET_PRIVATE (gdao);

	/* open database connection */
	priv->gda_conn = gda_client_open_connection_from_string (priv->gda_client,
                                                           provider_id,
										                                       cnc_string,
                                                           0);
	if (priv->gda_conn == NULL)
		{
			g_warning ("Unable to create the connection.\n"
			           "provider_id: %s\tcnc_string: %s\n",
			           provider_id, cnc_string);
			return NULL;
		}

	return gdao;
}

/**
 * gdao_new_from_connection:
 * @conn: a #GdaConnection.
 *
 * Returns a #GdaO from an existing #GdaConnection.
 *
 * Return value: the newly created #GdaO.
 */
GdaO
*gdao_new_from_connection (GdaConnection *conn)
{
	GdaO *gdao;

	if (conn == NULL) return NULL;

	gdao = gdao_new_ (gda_connection_get_client (conn));

	return gdao;
}

/**
 * gdao_get_gdaclient:
 * @gdao: a #GdaO object.
 *
 * Return value: the #GdaClient associated to the #GdaO.
 */
const GdaClient
*gdao_get_gdaclient (GdaO *gdao)
{
	if (gdao == NULL)
		{
			return NULL;
		}
	else
		{
			GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);
			return priv->gda_client;
		}
}

/**
 * gdao_get_gdaconnection:
 * @gdao: a #GdaO object.
 *
 * Return value: the #GdaConnection associated to the #GdaO.
 */
const GdaConnection
*gdao_get_gdaconnection (GdaO *gdao)
{
	if (gdao == NULL)
		{
			return NULL;
		}
	else
		{
			GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);
			return priv->gda_conn;
		}
}

/**
 * gdao_get_provider:
 * @gdao: a #GdaO object.
 *
 * Return value: the provider id associated to the #GdaO.
 */
const gchar
*gdao_get_provider (GdaO *gdao)
{
	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);
	return gda_connection_get_provider (priv->gda_conn);
}

/**
 * gdao_query:
 * @gdao: a #GdaO object.
 * @sql: the sql text.
 *
 * Execute a selection query (SELECT).
 *
 * Return value: a #GdaDataModel, or NULL if query fails.
 */
GdaDataModel
*gdao_query (GdaO *gdao, const gchar *sql)
{
	if (gdao == NULL) return NULL;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

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
*gdao_data_model_get_field_value_stringify_at (GdaDataModel *data_model,
                                               gint row,
                                               const gchar *field_name)
{
	gchar *value = "";
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_stringify_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gint
 */
gint
gdao_data_model_get_field_value_integer_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gint value = 0;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_integer_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gfloat
 */
gfloat
gdao_data_model_get_field_value_float_at (GdaDataModel *data_model,
                                          gint row,
                                          const gchar *field_name)
{
	gfloat value = 0.0f;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_float_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gdouble
 */
gdouble
gdao_data_model_get_field_value_double_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	gdouble value = 0.0;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_double_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as gboolean
 */
gboolean
gdao_data_model_get_field_value_boolean_at (GdaDataModel *data_model,
                                            gint row,
                                            const gchar *field_name)
{
	gboolean value = FALSE;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_boolean_at (data_model, row, col);
		}

	return value;
}

/* TO DO - define as macro */
/* TO DO - catch error for gda_data_model_get_column_position */
/**
 * return the field_name's GdaValue as GDate
 */
GDate
*gdao_data_model_get_field_value_gdate_at (GdaDataModel *data_model,
                                           gint row,
                                           const gchar *field_name)
{
	GDate *value = NULL;
	gint col;

	col = gda_data_model_get_column_position (data_model, field_name);

	if (col >= 0)
		{
			value = gdao_data_model_get_value_gdate_at (data_model, row, col);
		}

	return value;
}

/**
 * gdao_data_model_get_value_stringify_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gchar (stringify).
 */
gchar
*gdao_data_model_get_value_stringify_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_data_model_get_value_integer_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gint.
 */
gint
gdao_data_model_get_value_integer_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_data_model_get_value_float_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gfloat.
 */
gfloat
gdao_data_model_get_value_float_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_data_model_get_value_double_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gdouble.
 */
gdouble
gdao_data_model_get_value_double_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_data_model_get_value_boolean_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #gboolean.
 */
gboolean
gdao_data_model_get_value_boolean_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_data_model_get_value_gdate_at:
 * @data_model: a #GdaDataModel object.
 * @row: row number.
 * @col: col number.
 *
 * Return value: the #GdaValue as #GDate.
 */
GDate
*gdao_data_model_get_value_gdate_at (GdaDataModel *data_model, gint row, gint col)
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
 * gdao_begin:
 * @gdao: a #GdaO object.
 *
 * Begin a new transaction.
 */
gboolean
gdao_begin (GdaO *gdao)
{
	if (gdao == NULL) return FALSE;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	priv->gda_trans = gda_transaction_new ("gdao");
	gda_transaction_set_isolation_level (priv->gda_trans,
	                                     GDA_TRANSACTION_ISOLATION_SERIALIZABLE);

	return gda_connection_begin_transaction (priv->gda_conn, priv->gda_trans);
}

/**
 * gdao_execute:
 * @gdao: a #GdaO object.
 * @sql: the sql text.
 *
 * Execute a command query (INSERT, UPDATE, DELETE).
 *
 * Return value: number of records affected by the query execution.
 */
gint
gdao_execute (GdaO *gdao, const gchar *sql)
{
	GdaCommand *gda_comm;
	gint nrecs;

	if (gdao == NULL) return -1;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

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
 * gdao_commit:
 * @gdao: a #GdaO object.
 *
 * Commit a transaction.
 */
gboolean
gdao_commit (GdaO *gdao)
{
	gboolean ret;

	if (gdao == NULL) return FALSE;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

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
 * gdao_rollback:
 * @gdao: a #GdaO object.
 *
 * Rollback a transaction.
 */
gboolean
gdao_rollback (GdaO *gdao)
{
	gboolean ret;

	if (gdao == NULL) return FALSE;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

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
gdao_free (GdaO *gdao)
{
	if (gdao == NULL) return;

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	/* close connection */
	if (gda_connection_is_open (priv->gda_conn))
		{
			gda_connection_close (priv->gda_conn);
		}

	/*g_free (gdao);*/
}

/* UTILITY'S FUNCTIONS */
/**
 * gdao_strescape:
 * @source: a string to escape.
 * @exceptions: a string of characters not to escape in @source.
 *
 * As g_strescape(), but it escapes also '.
 */
gchar
*gdao_strescape (const gchar *source, const gchar *exceptions)
{
	gchar *ret = "", *nsource;
	gint l = strlen (source);

	if (source == NULL || l == 0) return "";

	nsource = g_strescape (g_strstrip (g_strdup (source)), exceptions);

	/* escape di ' */
	if (strchr (nsource, '\'') != NULL)
		{
			while (*nsource)
				{
					if (*nsource == '\'')
						{
							ret = g_strconcat (ret, "'", g_strdup_printf ("%c", *nsource), NULL);
						}
					else
						{
							ret = g_strconcat (ret, g_strdup_printf ("%c", *nsource), NULL);
						}
				
					*nsource++;
				}

			return g_strdup (ret);
		}
	else
		{
			return g_strdup (nsource);
		}
}

/**
 * gdao_get_chr_quoting:
 * @gdao: a #GdaO object.
 *
 */
gchar
gdao_get_chr_quoting (GdaO *gdao)
{
	gchar chr = '\"';

	const gchar *provider = gdao_get_provider (gdao);

	if (strcmp (provider, "MySQL") == 0)
		{
			chr = '`';
		}

	return chr;
}

/* PRIVATE */
static void
get_errors (GdaConnection *connection)
{
	GList *list;
	GList *node;
	GdaError *error;

	list = (GList *) gda_connection_get_errors (connection);

	for (node = g_list_first (list); node != NULL; node = g_list_next (node))
		{
			error = (GdaError *) node->data;
			g_fprintf (stderr, "Error no: %d\t", gda_error_get_number (error));
			g_fprintf (stderr, "desc: %s\t", gda_error_get_description (error));
			g_fprintf (stderr, "source: %s\t", gda_error_get_source (error));
			g_fprintf (stderr, "sqlstate: %s\n", gda_error_get_sqlstate (error));
		}
}

static void
gdao_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaO *gdao = GDAO (object);

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdao_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaO *gdao = GDAO (object);

	GdaOPrivate *priv = GDAO_GET_PRIVATE (gdao);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
