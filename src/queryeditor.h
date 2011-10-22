/*
 * Copyright (C) 2011 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GDAEX_QUERY_EDITOR_H__
#define __GDAEX_QUERY_EDITOR_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <libgdaex.h>


G_BEGIN_DECLS


#define TYPE_GDAEX_QUERY_EDITOR                 (gdaex_query_editor_get_type ())
#define GDAEX_QUERY_EDITOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditor))
#define GDAEX_QUERY_EDITOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditorClass))
#define GDAEX_IS_QUERY_EDITOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GDAEX_QUERY_EDITOR))
#define GDAEX_IS_QUERY_EDITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GDAEX_QUERY_EDITOR))
#define GDAEX_QUERY_EDITOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditorClass))

typedef struct _GdaExQueryEditor GdaExQueryEditor;
typedef struct _GdaExQueryEditorClass GdaExQueryEditorClass;

struct _GdaExQueryEditor
	{
		GObject parent;
	};

struct _GdaExQueryEditorClass
	{
		GObjectClass parent_class;
	};

GType gdaex_query_editor_get_type (void) G_GNUC_CONST;


GdaExQueryEditor *gdaex_query_editor_new (GdaEx *gdaex);

GtkWidget *gdaex_query_editor_get_widget (GdaExQueryEditor *qe);

void gdaex_query_editor_set_show_visibile (GdaExQueryEditor *qe, gboolean visibile);
void gdaex_query_editor_set_where_visibile (GdaExQueryEditor *qe, gboolean visibile);
void gdaex_query_editor_set_order_visibile (GdaExQueryEditor *qe, gboolean visibile);

gboolean gdaex_query_editor_get_show_visible (GdaExQueryEditor *qe);
gboolean gdaex_query_editor_get_where_visible (GdaExQueryEditor *qe);
gboolean gdaex_query_editor_get_order_visible (GdaExQueryEditor *qe);

typedef enum
	{
		GDAEX_QE_FIELD_TYPE_TEXT = 1,
		GDAEX_QE_FIELD_TYPE_INTEGER,
		GDAEX_QE_FIELD_TYPE_DOUBLE,
		GDAEX_QE_FIELD_TYPE_DATE,
		GDAEX_QE_FIELD_TYPE_DATETIME,
		GDAEX_QE_FIELD_TYPE_TIME,
	} GdaExQueryEditorFieldType;

typedef enum
	{
		GDAEX_QE_WHERE_TYPE_EQUAL = 1,
		GDAEX_QE_WHERE_TYPE_STARTS = 2,
		GDAEX_QE_WHERE_TYPE_CONTAINS = 4,
		GDAEX_QE_WHERE_TYPE_ENDS = 8,
		GDAEX_QE_WHERE_TYPE_ISTARTS = 16,
		GDAEX_QE_WHERE_TYPE_ICONTAINS = 32,
		GDAEX_QE_WHERE_TYPE_IENDS = 64,
		GDAEX_QE_WHERE_TYPE_GREAT = 128,
		GDAEX_QE_WHERE_TYPE_GREAT_EQUAL = 256,
		GDAEX_QE_WHERE_TYPE_LESS = 512,
		GDAEX_QE_WHERE_TYPE_LESS_EQUAL = 1024,
		GDAEX_QE_WHERE_TYPE_BETWEEN = 2048,
		GDAEX_QE_WHERE_TYPE_IS_NULL = 4096
	} GdaExQueryEditorWhereType;

typedef enum
	{
		GDAEX_QE_JOIN_TYPE_INNER,
		GDAEX_QE_JOIN_TYPE_LEFT
	} GdaExQueryEditorJoinType;

#define GDAEX_QE_WHERE_TYPE_STRING GDAEX_QE_WHERE_TYPE_STARTS | GDAEX_QE_WHERE_TYPE_CONTAINS | GDAEX_QE_WHERE_TYPE_ENDS | GDAEX_QE_WHERE_TYPE_ISTARTS | GDAEX_QE_WHERE_TYPE_ICONTAINS | GDAEX_QE_WHERE_TYPE_IENDS

#define GDAEX_QE_WHERE_TYPE_NUMBER GDAEX_QE_WHERE_TYPE_EQUAL | GDAEX_QE_WHERE_TYPE_GREAT | GDAEX_QE_WHERE_TYPE_GREAT_EQUAL | GDAEX_QE_WHERE_TYPE_LESS | GDAEX_QE_WHERE_TYPE_LESS_EQUAL | GDAEX_QE_WHERE_TYPE_BETWEEN

#define GDAEX_QE_WHERE_TYPE_DATETIME GDAEX_QE_WHERE_TYPE_NUMBER

typedef struct
	{
		gchar *table_name;

		gchar *name;
		gchar *name_visible;
		gchar *description;
		gchar *alias;
		GdaExQueryEditorFieldType type;
		gboolean for_show;
		gboolean always_showed;
		gboolean for_where;
		guint available_where_type;

		/* TODO 
		 * to refactor
		 */
		gchar *decode_table2;
		GdaExQueryEditorJoinType decode_join_type;
		/* TODO 
		GSList *decode_fields1;
		GSList *decode_fields2;
		*/
		gchar *decode_field2;
		gchar *decode_field_to_show;
		gchar *decode_field_alias;
	} GdaExQueryEditorField;

gboolean gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                                       const gchar *table_name,
                                       const gchar *table_name_visibile);
gboolean gdaex_query_editor_table_add_field (GdaExQueryEditor *qe,
                                             const gchar *table_name,
                                             GdaExQueryEditorField field);
gboolean gdaex_query_editor_add_relation (GdaExQueryEditor *qe,
                                          const gchar *table1,
                                          const gchar *table2,
                                          ...);

void gdaex_query_editor_clean_choices (GdaExQueryEditor *qe);

GdaSqlBuilder *gdaex_query_editor_get_sql_as_gdasqlbuilder (GdaExQueryEditor *qe);

const gchar *gdaex_query_editor_get_sql (GdaExQueryEditor *qe);

xmlNode *gdaex_query_editor_get_sql_as_xml (GdaExQueryEditor *qe);

void gdaex_query_editor_load_choices_from_xml (GdaExQueryEditor *qe,
                                               xmlNode *root,
                                               gboolean clean);


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_H__ */
