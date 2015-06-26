/*
 *  sqlbuilder.h
 *
 *  Copyright (C) 2015 Andrea Zagli <azagli@libero.it>
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

#ifndef __GDAEX_SQL_BUILDER_H__
#define __GDAEX_SQLBUILDER_H__

#include <glib.h>
#include <glib-object.h>

#include "gdaex.h"

G_BEGIN_DECLS


#define GDAEX_TYPE_SQLBUILDER                 (gdaex_sql_builder_get_type ())
#define GDAEX_SQLBUILDER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_TYPE_SQLBUILDER, GdaExSqlBuilder))
#define GDAEX_SQLBUILDER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GDAEX_TYPE_SQLBUILDER, GdaExSqlBuilderClass))
#define GDAEX_IS_SQLBUILDER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_TYPE_SQL_BUILDER))
#define GDAEX_IS_SQLBUILDER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GDAEX_TYPE_SQLBUILDER))
#define GDAEX_SQLBUILDER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GDAEX_TYPE_SQLBUILDER, GdaExSqlBuilderClass))


typedef struct _GdaExSqlBuilder GdaExSqlBuilder;
typedef struct _GdaExSqlBuilderClass GdaExSqlBuilderClass;

struct _GdaExSqlBuilder
	{
		GObject parent;
	};

struct _GdaExSqlBuilderClass
	{
		GObjectClass parent_class;
	};

GType gdaex_sql_builder_get_type (void) G_GNUC_CONST;


GdaExSqlBuilder *gdaex_sql_builder_new (GdaSqlStatementType stmt_type);

void gdaex_sql_builder_from (GdaExSqlBuilder *sqlb, const gchar *table_name, const gchar *table_alias);
void gdaex_sql_builder_join (GdaExSqlBuilder *sqlb,
							 const gchar *table_name,
							 GdaSqlSelectJoinType join_type,
							 const gchar *table_name_right,
							 ...);
void gdaex_sql_builder_fields (GdaExSqlBuilder *sqlb, ...);

GdaSqlBuilder *gdaex_sql_builder_get_gda_sql_builder (GdaExSqlBuilder *sqlb);
const gchar *gdaex_sql_builder_get_sql (GdaExSqlBuilder *sqlb);


G_END_DECLS

#endif /* __GDAEX_SQLBUILDER_H__ */
