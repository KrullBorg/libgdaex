/*
 *  sql_builder.c
 *
 *  Copyright (C) 2010-2016 Andrea Zagli <azagli@libero.it>
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

typedef struct _GdaExSqlBuilderField GdaExSqlBuilderField;
struct _GdaExSqlBuilderField
{
	guint id;
	gchar *name;
	gchar *alias;
};

typedef struct _GdaExSqlBuilderTable GdaExSqlBuilderTable;
struct _GdaExSqlBuilderTable
{
	guint id;
	gchar *name;
	gchar *alias;
	GHashTable *ht_fields;
};

typedef struct _GdaExSqlBuilderPrivate GdaExSqlBuilderPrivate;
struct _GdaExSqlBuilderPrivate
{
	GdaSqlStatementType stmt_type;
	GdaSqlBuilder *sqlb;
	GHashTable *ht_tables;
	GdaSqlBuilderId id_where;
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

}

/**
 * gdaex_sql_builder_new:
 * @stmt_type:
 *
 * Returns:
 */
GdaExSqlBuilder
*gdaex_sql_builder_new (GdaSqlStatementType stmt_type)
{
	GdaExSqlBuilder *gdaex_sql_builder = GDAEX_SQLBUILDER (g_object_new (gdaex_sql_builder_get_type (), NULL));

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (gdaex_sql_builder);

	priv->stmt_type = stmt_type;
	priv->sqlb = gda_sql_builder_new (priv->stmt_type);
	priv->ht_tables = g_hash_table_new (g_str_hash, g_str_equal);
	priv->id_where = 0;

	return gdaex_sql_builder;
}

static GdaExSqlBuilderField
*gdaex_sql_builder_get_field (GdaExSqlBuilder *sqlb, GdaExSqlBuilderTable *table, const gchar *field_name, const gchar *field_alias, GValue *gval, gboolean add)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	GdaExSqlBuilderField *f;

	f = g_hash_table_lookup (table->ht_fields, field_name);
	if (f == NULL && add)
		{
			f = g_new0 (GdaExSqlBuilderField, 1);
			if (priv->stmt_type == GDA_SQL_STATEMENT_SELECT)
				{
					f->id = gda_sql_builder_select_add_field (priv->sqlb, field_name, g_strcmp0 (table->alias, "") != 0 ? table->alias : table->name, field_alias);
				}
			else
				{
					f->id = gda_sql_builder_add_id (priv->sqlb, g_strcmp0 (field_alias, "") != 0 ? field_alias : field_name);
					if (gval != NULL)
						{
							gda_sql_builder_add_field_value_as_gvalue (priv->sqlb, g_strcmp0 (field_alias, "") != 0 ? field_alias : field_name, gval);
						}
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
			if (g_strcmp0 (table_name, "") == 0)
				{
					t->id = 0;
				}
			else
				{
					if (priv->stmt_type == GDA_SQL_STATEMENT_SELECT)
						{
							t->id = gda_sql_builder_select_add_target_id (priv->sqlb, gda_sql_builder_add_id (priv->sqlb, table_name), table_alias);
						}
					else
						{
							t->id = 0;
							gda_sql_builder_set_table (priv->sqlb, table_name);
						}
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

/**
 * gdaex_sql_builder_from:
 * @sqlb: a #GdaExSqlBuilder object.
 * @table_name: the table's name.
 * @table_alias: the table's alias.
 */
void
gdaex_sql_builder_from (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias)
{
	gdaex_sql_builder_get_table (sqlb, table_name, table_alias, TRUE);
}

/**
 * gdaex_sql_builder_from_v:
 * @sqlb: a #GdaExSqlBuilder object.
 * @...: an #NULL terminated array of couples table_name - table_alias.
 */
void
gdaex_sql_builder_from_v (GdaExSqlBuilder *sqlb, ...)
{
	va_list ap;

	gchar *table_name;
	gchar *table_alias;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	va_start (ap, sqlb);
	do
		{
			table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
					table_alias = va_arg (ap, gchar *);
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

/**
 * gdaex_sql_builder_join:
 * @sqlb:
 * @join_type:
 * @...:
 */
void
gdaex_sql_builder_join (GdaExSqlBuilder *sqlb,
                        GdaSqlSelectJoinType join_type,
                        ...)
{
	va_list ap;

	gchar *field_table_name_left;
	gchar *field_name_left;
	gchar *field_alias_left;
	gchar *field_table_name_right;
	gchar *field_name_right;
	gchar *field_alias_right;

	GdaExSqlBuilderTable *t_left;
	GdaExSqlBuilderField *f_left;
	GdaExSqlBuilderTable *t_right;
	GdaExSqlBuilderField *f_right;

	gchar *tmp;
	GdaSqlOperatorType op;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	if (priv->stmt_type != GDA_SQL_STATEMENT_SELECT) return;

	va_start (ap, join_type);
	do
		{
			field_table_name_left = NULL;
			field_name_left = NULL;
			field_alias_left = NULL;
			field_table_name_right = NULL;
			field_name_right = NULL;
			field_alias_right = NULL;

			field_table_name_left = va_arg (ap, gchar *);
			if (field_table_name_left != NULL)
				{
					field_name_left = va_arg (ap, gchar *);
					if (field_name_left != NULL)
						{
							field_alias_left = va_arg (ap, gchar *);
							if (field_alias_left != NULL)
								{
									t_left = gdaex_sql_builder_get_table (sqlb, field_table_name_left, NULL, TRUE);
									f_left = gdaex_sql_builder_get_field (sqlb, t_left, field_name_left, field_alias_left, NULL, TRUE);
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

			field_table_name_right = va_arg (ap, gchar *);
			if (field_table_name_right != NULL)
				{
					field_name_right = va_arg (ap, gchar *);
					if (field_name_right != NULL)
						{
							field_alias_right = va_arg (ap, gchar *);
							if (field_alias_right != NULL)
								{
									t_right = gdaex_sql_builder_get_table (sqlb, field_table_name_right, NULL, TRUE);
									f_right = gdaex_sql_builder_get_field (sqlb, t_right, field_name_right, field_alias_right, NULL, TRUE);
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

			GdaSqlBuilderId jid = gda_sql_builder_add_cond (priv->sqlb, op, f_right->id, f_left->id, 0);
			gda_sql_builder_select_join_targets (priv->sqlb, t_left->id, t_right->id, join_type, jid);
		} while (TRUE);
	va_end (ap);
}

/**
 * gdaex_sql_builder_field:
 * @sqlb:
 * @table_name:
 * @field_name:
 * @field_alias:
 * @gval:
 *
 */
void
gdaex_sql_builder_field (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *field_name, const gchar *field_alias, GValue *gval)
{
	GdaExSqlBuilderTable *t;

	t = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
	gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, gval, TRUE);
}

/**
 * gdaex_sql_builder_fields:
 * @sqlb:
 * @...:
 */
void
gdaex_sql_builder_fields (GdaExSqlBuilder *sqlb, ...)
{
	va_list ap;

	gchar *table_name;
	gchar *field_name;
	gchar *field_alias;
	GValue *gval;

	GdaExSqlBuilderTable *t;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	va_start (ap, sqlb);
	do
		{
			table_name = NULL;
			field_name = NULL;
			field_alias = NULL;
			gval = NULL;

			table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
				    field_name = va_arg (ap, gchar *);
					if (field_name != NULL)
						{
							field_alias = va_arg (ap, gchar *);
							if (field_alias != NULL)
								{
									gval = va_arg (ap, GValue *);

									t = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
								    gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, gval, TRUE);
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

/**
 * gdaex_sql_builder_where:
 * @sqlb:
 * @op:
 * @...:
 *
 * Returns:
 */
GdaSqlBuilderId
gdaex_sql_builder_where (GdaExSqlBuilder *sqlb, GdaSqlOperatorType op, ...)
{
	va_list ap;

	gchar *table_name;
	gchar *field_name;
	gchar *field_alias;
	GValue *gval;

	GdaExSqlBuilderTable *t;
	GdaExSqlBuilderField *f;

	GdaSqlOperatorType op_expr;

	GdaSqlBuilderId id_expr1;
	GdaSqlBuilderId id_expr2;
	GdaSqlBuilderId id_cond;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	if (priv->stmt_type == GDA_SQL_STATEMENT_INSERT)
		{
			return -1;
		}

	va_start (ap, op);
	do
		{
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
									f = gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, NULL, TRUE);
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

			op_expr = va_arg (ap, guint);

			id_expr1 = 0;
			id_expr2 = 0;
			switch (op_expr)
				{
				case GDA_SQL_OPERATOR_TYPE_EQ:
				case GDA_SQL_OPERATOR_TYPE_IS:
				case GDA_SQL_OPERATOR_TYPE_LIKE:
				case GDA_SQL_OPERATOR_TYPE_NOTLIKE:
				case GDA_SQL_OPERATOR_TYPE_ILIKE:
				case GDA_SQL_OPERATOR_TYPE_NOTILIKE:
				case GDA_SQL_OPERATOR_TYPE_GT:
				case GDA_SQL_OPERATOR_TYPE_LT:
				case GDA_SQL_OPERATOR_TYPE_GEQ:
				case GDA_SQL_OPERATOR_TYPE_LEQ:
				case GDA_SQL_OPERATOR_TYPE_DIFF:
				case GDA_SQL_OPERATOR_TYPE_REGEXP:
				case GDA_SQL_OPERATOR_TYPE_REGEXP_CI:
				case GDA_SQL_OPERATOR_TYPE_NOT_REGEXP:
				case GDA_SQL_OPERATOR_TYPE_NOT_REGEXP_CI:
				case GDA_SQL_OPERATOR_TYPE_SIMILAR:
				case GDA_SQL_OPERATOR_TYPE_REM:
				case GDA_SQL_OPERATOR_TYPE_DIV:
				case GDA_SQL_OPERATOR_TYPE_BITAND:
				case GDA_SQL_OPERATOR_TYPE_BITOR:
					gval = va_arg (ap, GValue *);
					if (gval != NULL)
						{
							id_expr1 = gda_sql_builder_add_expr_value (priv->sqlb, NULL, gval);
						}
					break;

				case GDA_SQL_OPERATOR_TYPE_ISNULL:
				case GDA_SQL_OPERATOR_TYPE_ISNOTNULL:
					break;

				case GDA_SQL_OPERATOR_TYPE_BETWEEN:
					gval = va_arg (ap, GValue *);
					if (gval != NULL)
						{
							id_expr1 = gda_sql_builder_add_expr_value (priv->sqlb, NULL, gval);
						}
					gval = va_arg (ap, GValue *);
					if (gval != NULL)
						{
							id_expr2 = gda_sql_builder_add_expr_value (priv->sqlb, NULL, gval);
						}
					break;
				}

			id_cond = gda_sql_builder_add_cond (priv->sqlb, op_expr, f->id, id_expr1, id_expr2);
			if (priv->id_where != 0)
				{
					priv->id_where = gda_sql_builder_add_cond (priv->sqlb, op, priv->id_where, id_cond, 0);
				}
			else
				{
					priv->id_where = id_cond;
				}
			gda_sql_builder_set_where (priv->sqlb, priv->id_where);
		} while (TRUE);
	va_end (ap);

	return priv->id_where;
}

/**
 * gdaex_sql_builder_order:
 * @sqlb:
 * @...:
 */
void
gdaex_sql_builder_order (GdaExSqlBuilder *sqlb, ...)
{
	va_list ap;

	gchar *table_name;
	gchar *field_name;
	gchar *field_alias;
	gboolean asc;

	GdaExSqlBuilderTable *t;
	GdaExSqlBuilderField *f;

	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	va_start (ap, sqlb);
	do
		{
			table_name = NULL;
			field_name = NULL;
			field_alias = NULL;

			table_name = va_arg (ap, gchar *);
			if (table_name != NULL)
				{
				    field_name = va_arg (ap, gchar *);
					if (field_name != NULL)
						{
							field_alias = va_arg (ap, gchar *);
							if (field_alias != NULL)
								{
									asc = va_arg (ap, gboolean);

									t = gdaex_sql_builder_get_table (sqlb, table_name, NULL, TRUE);
								    f = gdaex_sql_builder_get_field (sqlb, t, field_name, field_alias, NULL, TRUE);
									gda_sql_builder_select_order_by (priv->sqlb, f->id, asc, NULL);
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

/**
 * gaex_sql_builder_get_gda_sql_builder:
 * @sqlb:
 *
 * Returns:
 */
GdaSqlBuilder
*gdaex_sql_builder_get_gda_sql_builder (GdaExSqlBuilder *sqlb)
{
	GdaExSqlBuilderPrivate *priv = GDAEX_SQLBUILDER_GET_PRIVATE (sqlb);

	return priv->sqlb;
}

/**
 * gdaex_sql_builder_get_sql:
 * @sqlb:
 * @cnc:
 * @params:
 *
 * Returns:
 */
gchar
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

/**
 * gdaex_sql_builder_get_sql_select:
 * @sqlb:
 * @cnc:
 * @params:
 *
 */
gchar
*gdaex_sql_builder_get_sql_select (GdaExSqlBuilder *sqlb, GdaConnection *cnc, GdaSet *params)
{
	gchar *ret;
	gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_sql_builder_get_sql (sqlb, cnc, params);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "SELECT");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "FROM");
	if (end == NULL)
		{
			return ret;
		}

	ret = g_strndup (start + 7, strlen (sql) - 8 - strlen (end));

	g_free (sql);

	return ret;
}

/**
 * gdaex_sql_builder_get_sql_from:
 * @sqlb:
 * @cnc:
 * @params:
 *
 */
gchar
*gdaex_sql_builder_get_sql_from (GdaExSqlBuilder *sqlb, GdaConnection *cnc, GdaSet *params)
{
	gchar *ret;
	gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_sql_builder_get_sql (sqlb, cnc, params);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "FROM");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "WHERE");
	if (end == NULL)
		{
			end = g_strstr_len (sql, -1, "ORDER BY");
		}

	ret = g_strndup (start + 5, strlen (start) - (end != NULL ? 6 : 5) - (end != NULL ? strlen (end) : 0));

	g_free (sql);

	return ret;
}

/**
 * gdaex_sql_builder_get_sql_where:
 * @sqlb:
 * @cnc:
 * @params:
 *
 */
gchar
*gdaex_sql_builder_get_sql_where (GdaExSqlBuilder *sqlb, GdaConnection *cnc, GdaSet *params)
{
	gchar *ret;
	gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_sql_builder_get_sql (sqlb, cnc, params);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "WHERE");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "ORDER BY");

	ret = g_strndup (start + 6, strlen (start) - (end != NULL ? 7 : 6) - (end != NULL ? strlen (end) : 0));

	g_free (sql);

	return ret;
}

/**
 * gdaex_sql_builder_get_sql_order:
 * @sqlb:
 * @cnc:
 * @params:
 *
 */
gchar
*gdaex_sql_builder_get_sql_order (GdaExSqlBuilder *sqlb, GdaConnection *cnc, GdaSet *params)
{
	gchar *ret;
	gchar *sql;

	gchar *start;

	ret = NULL;

	sql = gdaex_sql_builder_get_sql (sqlb, cnc, params);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "ORDER BY");
	if (start == NULL)
		{
			return ret;
		}

	ret = g_strndup (start + 9, strlen (start) - 9);

	g_free (sql);

	return ret;
}

/**
 * gdaex_sql_builder_query:
 * @sqlb:
 * @gdaex:
 *
 * Returns: a #GdaDataModel.
 */
GdaDataModel
*gdaex_sql_builder_query (GdaExSqlBuilder *sqlb, GdaEx *gdaex, GdaSet *params)
{
	gchar *sql;
	GdaDataModel *dm;

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	sql = gdaex_sql_builder_get_sql (sqlb, (GdaConnection *)gdaex_get_gdaconnection (gdaex), params);

	dm = gdaex_query (gdaex, sql);
	g_free (sql);

	return dm;
}

gint
gdaex_sql_builder_execute  (GdaExSqlBuilder *sqlb, GdaEx *gdaex, GdaSet *params)
{
	gchar *sql;
	gint ret;

	g_return_val_if_fail (IS_GDAEX (gdaex), -1);

	sql = gdaex_sql_builder_get_sql (sqlb, (GdaConnection *)gdaex_get_gdaconnection (gdaex), params);

	ret = gdaex_execute (gdaex, sql);
	g_free (sql);

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
