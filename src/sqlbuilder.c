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

	priv->sqlb = gda_sql_builder_new (stmt_type);

	return gdaex_sql_builder;
}

GdaExSqlBuilderField
*gdaex_sql_builder_get_field (GdaExSqlBuilder *sqlb, GdaExSqlBuilderTable *table, const gchar *field_name, gboolean add)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderField *f;

	f = g_hash_table_lookup (table->ht_fields, field_name);
  if (f == NULL && add)
	{
	  f = g_new0 (GdaExSqlBuilderField, 1);
	  f->id = gda_sql_builder_select_add_field (priv->sqlb, field_name, table->name, NULL);
	  f->name = g_strdup (field_name);
	  f->alias = NULL;
	  g_hash_table_insert (table->ht_fields, g_strdup (field_name), f);
	}
  
  return f;
}

GdaExSqlBuilderTable
*gdaex_sql_builder_get_table (GdaExSqlBuilder *sqlb, const gchar *table_name, gboolean add)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderTable *t;

	t = g_hash_table_lookup (priv->ht_tables, table_name);
	if (t == NULL && add)
	  {
		t = g_new0 (GdaExSqlBuilderTable, 1);
		t->id = gda_sql_builder_select_add_target_id (priv->sqlb, gda_sql_builder_add_id (priv->sqlb, table_name), NULL);
		t->name = g_strdup (table_name);
		t->alias = NULL;
		t->ht_fields = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (priv->ht_tables, g_strdup (t->name), t);
	  }

	return t;
}

void
gdaex_sql_builder_from (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderTable *t;

	t = gdaex_sql_builder_get_table (sqlb, table_name, TRUE);
	if (t->alias != NULL)
	  {
		g_free (t->alias);
		t->alias = g_strdup (table_alias);
	  }
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
					GdaExSqlBuilderTable *t = gdaex_sql_builder_get_table (sqlb, table_name, TRUE);
					GdaExSqlBuilderField *f = gdaex_sql_builder_get_field (sqlb, t, field_name, TRUE);

					gchar *_field_alias = g_strstrip (g_strdup (field_alias));
					gda_sql_builder_select_add_field (priv->sqlb, field_name, table_name, _field_alias);
					g_free (_field_alias);
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
}

GdaSqlBuilder
*gdaex_sql_builder_get_gda_sql_builder (GdaExSqlBuilder *sqlb)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	return priv->sqlb;
}

const gchar
*gdaex_sql_builder_get_sql (GdaExSqlBuilder *sqlb)
{
  gchar *ret;
  GdaStatement *stmt;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	ret = NULL;

	stmt = gda_sql_builder_get_statement (priv->sqlb, NULL);
	if (stmt != NULL)
	  {
		ret = gda_statement_to_sql (stmt, NULL, NULL);
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
