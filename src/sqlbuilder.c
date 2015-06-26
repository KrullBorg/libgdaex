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

void
gdaex_sql_builder_from (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderTable *t;

	t = g_hash_table_lookup (priv->ht_tables, table_name);
	if (t == NULL)
	  {
		t = g_new0 (GdaExSqlBuilderTable, 1);
		t->name = g_strdup (table_name);
		t->alias = g_strdup (table_alias);
		t->id = gda_sql_builder_select_add_target_id (priv->sqlb, gda_sql_builder_add_id (priv->sqlb, table_name), NULL);
		g_hash_table_insert (priv->ht_tables, t->name, t);
	  }
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

	gda_sql_builder_select_add_field (priv->sqlb, "*", NULL, NULL);
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
