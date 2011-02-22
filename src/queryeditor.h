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

GtkWidget *gdaex_query_editor_get_widget (GdaExQueryEditor *gdaex_query_editor);

typedef struct
	{
		gchar *name;
		gchar *name_visible;
		/* - tipo di campo (stringa, interno, double, date, datetime) */
		/* - sempre presente nelle query, quindi non sceglibile per la parte show */
		/* - sceglibile per la parte where */
		/* - condizioni where che è possibile applicare (es. i campi id_* non ha senso che abbiano un between, */
		/*                                             ma ci deve essere la possibilità di fare ricerche anche sulle decodifiche) */
	} GdaExQueryEditorField;

gboolean gdaex_query_editor_add_table (GdaExQueryEditor *qe, const gchar *table_name, const gchar *table_name_visibile);
gboolean gdaex_query_editor_table_add_field (GdaExQueryEditor *qe, const gchar *table_name, GdaExQueryEditorField field);
gboolean gdaex_query_editor_add_relation (GdaExQueryEditor *qe,
                                          const gchar *table_name1, GdaExQueryEditorField field1,
                                          const gchar *table_name2, GdaExQueryEditorField field2);

const gchar *gdaex_query_editor_get_sql (GdaExQueryEditor *qe);


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_H__ */
