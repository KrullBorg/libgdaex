/*
 *  sql_builder.c
 *
 *  Copyright (C) 2010-2014 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex.
 *  
 *  libgdaex_sql_builder is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaex_sql_builder is distributed in the hope that it will be useful,
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

#include <glib/gi18n-lib.h>

#include "sqlbuilder.h"

static void gdaex_sql_builder_class_init (GdaExSqlBuilderClass *klass);
static void gdaex_sql_builder_init (GdaExSqlBuilder *gdaex_sql_builder);

static void gdaex_sql_builder_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_sql_builder_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);


#define GDAEX_SQLBUILDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDAEX_TYPE_SQLBUILDER, GdaExSqlBuilderPrivate))

typedef struct _GdaExSqlBuilderTable GdaExSqlBuilderTable;
struct _GdaExSqlBuilderTable
{
	guint id;
	gchar *name;
	gchar *alias;
	GHashTable *ht_fields;
};

typedef struct _GdaExSqlBuilderField GdaExSqlBuilderField;
struct _GdaExSqlBuilderField
{
	guint id;
	gchar *name;
	gchar *alias;
};

typedef struct _GdaExSqlBuilderPrivate GdaExSqlBuilderPrivate;
struct _GdaExSqlBuilderPrivate
{
	GdaSqlStatementType stmt_type;
	GdaSqlBuilder *sqlb;
	GHashTable *ht_tables;
};

G_DEFINE_TYPE (GdaExSqlBuilder, gdaex_sql_builder, G_TYPE_OBJECT)

static void
gdaex_sql_builder_class_init (GdaExSqlBuilderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExSqlBuilderPrivate));

	object_class->set_property = gdaex_sql_builder_set_property;
	object_class->get_property = gdaex_sql_builder_get_property;
}

static void
gdaex_sql_builder_init (GdaExSqlBuilder *gdaex_sql_builder)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (gdaex_sql_builder);

	priv->ht_tables = g_hash_table_new (g_str_hash, g_str_equal);
}

GdaExSqlBuilder
*gdaex_sql_builder_new (GdaSqlStatementType stmt_type)
{
	GdaExSqlBuilder *gdaex_sql_builder = GDAEX_SQLBUILDER (g_object_new (gdaex_sql_builder_get_type (), NULL));

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (gdaex_sql_builder);

	priv->stmt_type = stmt_type;
	priv->sqlb = gda_sql_builder_new (priv->stmt_type);

	return gdaex_sql_builder;
}

static GdaExSqlBuilderField
*gdaex_sql_builder_get_field (GdaExSqlBuilder *sqlb, GdaExSqlBuilderTable *table, const gchar *field_name, const gchar *field_alias, gboolean add)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderField *f;

	f = g_hash_table_lookup (table->ht_fields, field_name);
	if (f == NULL && add)
		{
			f = g_new0 (GdaExSqlBuilderField, 1);
			if (priv->stmt_type == GDA_SQL_STATEMENT_SELECT)
				{
					f->id = gda_sql_builder_select_add_field (priv->sqlb, field_name, table->alias != NULL ? table->alias : table->name, field_alias);
				}
			else
				{
					/* TODO */
				}
			f->name = g_strdup (field_name);
			if (field_alias != NULL)
				{
					f->alias = g_strdup (field_alias);
				}
			else
				{
					f->alias = NULL;
				}
			g_hash_table_insert (table->ht_fields, g_strdup (field_name), f);
		}
  
	return f;
}

static GdaExSqlBuilderTable
*gdaex_sql_builder_get_table (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias, gboolean add)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderTable *t;

	t = g_hash_table_lookup (priv->ht_tables, table_name);
	if (t == NULL && add)
		{
			t = g_new0 (GdaExSqlBuilderTable, 1);
			if (priv->stmt_type == GDA_SQL_STATEMENT_SELECT)
				{
					t->id = gda_sql_builder_select_add_target_id (priv->sqlb, gda_sql_builder_add_id (priv->sqlb, table_name), table_alias);
				}
			else
				{
					t->id = 0;
					gda_sql_builder_set_table (priv->sqlb, table_name);
				}
			t->name = g_strdup (table_name);
			if (table_alias != NULL)
				{
					t->alias = g_strdup (table_alias);
				}
			else
				{
					t->alias = NULL;
				}
			t->ht_fields = g_hash_table_new (g_str_hash, g_str_equal);
			g_hash_table_insert (priv->ht_tables, g_strdup (t->name), t);
		}

	return t;
}

void
gdaex_sql_builder_from (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias)
{
	gdaex_sql_builder_get_table (sqlb, table_name, table_alias, TRUE);
}

void
gdaex_sql_builder_from_v (GdaExSqlBuilder *sqlb, ...)
{
	va_list ap;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	va_start (ap, sqlb);
	do
		{
			gchar *table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
					gchar *table_alias = va_arg (ap, gchar *);
					if (table_alias != NULL)
						{
							gdaex_sql_builder_from (sqlb, table_name, table_alias);
							if (priv->stmt_type != GDA_SQL_STATEMENT_SELECT)
								{
									break;
								}
						}
					else
						{
							break;
						}
				}
			else
				{
					break;
				}
		} while (TRUE);
}

void
gdaex_sql_builder_join (GdaExSqlBuilder *sqlb,
                        const gchar *table_name,
                        GdaSqlSelectJoinType join_type,
                        const gchar *table_name_right,
                        ...)
{
	va_list ap;

	GdaExSqlBuilderTable *t1;
	GdaExSqlBuilderTable *t2;
  
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	if (priv->stmt_type != GDA_SQL_STATEMENT_SELECT) return;
	
	t1 = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
	t2 = gdaex_sql_builder_get_table (sqlb, table_name_right, NULL, TRUE);
	
	va_start (ap, table_name_right);
	do
		{
			gchar *table_name;
			gchar *field_name;
			gchar *field_alias;
			gchar *table_name_right;
			gchar *field_name_right;
			gchar *field_alias_right;

			GdaExSqlBuilderTable *t;
			GdaExSqlBuilderField *f;
			GdaExSqlBuilderTable *t_right;
			GdaExSqlBuilderField *f_right;

			gchar *tmp;
			GdaSqlOperatorType op;
		
			table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
					field_name = va_arg (ap, gchar *);
					if (field_name != NULL)
						{
							field_alias = va_arg (ap, gchar *);
							if (field_alias != NULL)
								{
									t = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
									f = gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, TRUE);
								}
							else
								{
									break;
								}
						}
					else
						{
							break;
						}
				}
			else
				{
					break;
				}

			op = va_arg (ap, guint);
		
			table_name_right = va_arg (ap, gchar *);
			if (table_name_right != NULL)
				{
					field_name_right = va_arg (ap, gchar *);
					if (field_name_right != NULL)
						{
							field_alias_right = va_arg (ap, gchar *);
							if (field_alias_right != NULL)
								{
									t_right = gdaex_sql_builder_get_table (sqlb, table_name_right, NULL, TRUE);
									f_right = gdaex_sql_builder_get_field (sqlb, t_right, field_name_right, field_alias_right, TRUE);
								}
							else
								{
									break;
								}
						}
					else
						{
							break;
						}
				}
			else
				{
					break;
				}

			GdaSqlBuilderId jid = gda_sql_builder_add_cond (priv->sqlb, op, f_right->id, f->id, 0);
			gda_sql_builder_select_join_targets (priv->sqlb, t->id, t_right->id, join_type, jid);
		} while (TRUE);
	va_end (ap);
}

void
gdaex_sql_builder_fields (GdaExSqlBuilder *sqlb, ...)
{
	va_list ap;
  
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	va_start (ap, sqlb);
	do
		{
			gchar *table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
					gchar *field_name = va_arg (ap, gchar *);
					if (field_name != NULL)
						{
							gchar *field_alias = va_arg (ap, gchar *);
							if (field_alias != NULL)
								{
									GdaExSqlBuilderTable *t = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
									GdaExSqlBuilderField *f = gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, TRUE);
								}
							else
								{
									break;
								}
						}
					else
						{
							break;
						}
				}
			else
				{
					break;
				}
		} while (TRUE);
	va_end (ap);
}

GdaSqlBuilder
*gdaex_sql_builder_get_gda_sql_builder (GdaExSqlBuilder *sqlb)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	return priv->sqlb;
}

const gchar
*gdaex_sql_builder_get_sql (GdaExSqlBuilder *sqlb, GdaConnection *cnc, GdaSet *params)
{
	gchar *ret;

	GError *error;
	GdaStatement *stmt;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	ret = NULL;

	error = NULL;
	stmt = gda_sql_builder_get_statement (priv->sqlb, &error);
	if (stmt != NULL)
		{
			error = NULL;
			ret = gda_statement_to_sql_extended (stmt,
												 cnc,
												 params,
												 GDA_STATEMENT_SQL_PARAMS_AS_VALUES,
												 NULL,
												 &error);
			if (ret == NULL
				|| error != NULL)
				{
					g_warning ("Error on creating sql statement: %s.",
							   error != NULL && error->message != NULL ? error->message : "no details");
				}
		}
	else
		{
			g_warning ("Error on creating GdaStatement: %s.",
					   error != NULL && error->message != NULL ? error->message : "no details");
		}

	return ret;
}

/* PRIVATE */
static void
gdaex_sql_builder_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExSqlBuilder *gdaex_sql_builder = GDAEX_SQLBUILDER (object);
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (gdaex_sql_builder);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_sql_builder_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExSqlBuilder *gdaex_sql_builder = GDAEX_SQLBUILDER (object);
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (gdaex_sql_builder);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
