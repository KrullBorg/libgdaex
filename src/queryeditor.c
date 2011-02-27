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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "queryeditor.h"

typedef struct
	{
		gchar *name;
		gchar *name_visible;
		GHashTable *fields;	/* GdaExQueryEditorField */
	} GdaExQueryEditorTable;

static void gdaex_query_editor_class_init (GdaExQueryEditorClass *class);
static void gdaex_query_editor_init (GdaExQueryEditor *gdaex_query_editor);

static void gdaex_query_editor_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_query_editor_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

static void gdaex_query_editor_refresh_gui (GdaExQueryEditor *qe);
static void gdaex_query_editor_refresh_gui_add_fields (GdaExQueryEditor *qe,
                                                       GdaExQueryEditorTable *table,
                                                       GtkTreeIter *iter_parent);

#define GDAEX_QUERY_EDITOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditorPrivate))

typedef struct _GdaExQueryEditorPrivate GdaExQueryEditorPrivate;
struct _GdaExQueryEditorPrivate
	{
		GdaEx *gdaex;

		GtkBuilder *gtkbuilder;

		GtkWidget *dialog;
		GtkWidget *hpaned_main;

		GtkTreeStore *tstore_fields;

		GHashTable *tables;	/* GdaExQueryEditorTable */
	};

G_DEFINE_TYPE (GdaExQueryEditor, gdaex_query_editor, G_TYPE_OBJECT)

enum
{
	CONDITION_EQ,
	CONDITION_LIKE,
	CONDITION_ILIKE,
	CONDITION_G,
	CONDITION_G_EQ,
	CONDITION_L,
	CONDITION_L_EQ,
	CONDITION_BETWEEN
};

enum
{
	COL_FIELDS_NAME,
	COL_FIELDS_VISIBLE_NAME,
	COL_FIELDS_DESCRIPTION
};

enum
{
	COL_SHOW_NAME,
	COL_SHOW_VISIBLE_NAME
};

enum
{
	COL_WHERE_NAME,
	COL_WHERE_VISIBLE_NAME,
	COL_WHERE_CONDITION_NOT,
	COL_WHERE_CONDITION_TYPE,
	COL_WHERE_CONDITION_FROM,
	COL_WHERE_CONDITION_TO
};

enum
{
	COL_ORDER_NAME,
	COL_ORDER_VISIBLE_NAME,
	COL_ORDER_ORDER
};

static void
gdaex_query_editor_class_init (GdaExQueryEditorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->set_property = gdaex_query_editor_set_property;
	object_class->get_property = gdaex_query_editor_get_property;

	g_type_class_add_private (object_class, sizeof (GdaExQueryEditorPrivate));
}

static void
gdaex_query_editor_init (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	priv->tables = g_hash_table_new (g_str_hash, g_str_equal);

	priv->dialog = NULL;
	priv->hpaned_main = NULL;
}

/**
 * gdaex_query_editor_new:
 * @gdaex:
 *
 * Returns: the newly created #GdaExQueryEditor object.
 */
GdaExQueryEditor
*gdaex_query_editor_new (GdaEx *gdaex)
{
	GError *error;

	GdaExQueryEditor *gdaex_query_editor;
	GdaExQueryEditorPrivate *priv;

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	gdaex_query_editor = GDAEX_QUERY_EDITOR (g_object_new (gdaex_query_editor_get_type (), NULL));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	priv->gdaex = gdaex;

	priv->gtkbuilder = gdaex_get_gtkbuilder (priv->gdaex);

	error = NULL;
	gtk_builder_add_objects_from_file (priv->gtkbuilder,
	                                   gdaex_get_guifile (priv->gdaex),
	                                   g_strsplit ("tstore_fields"
	                                               "|lstore_show"
	                                               "|tstore_where"
	                                               "|lstore_order"
	                                               "|diag_query_editor",
	                                               "|", -1),
	                                   &error);
	if (error != NULL)
		{
			g_warning ("Error on gui initialization: %s.",
			           error->message != NULL ? error->message : "no details");
			return NULL;
		}

	priv->tstore_fields = GTK_TREE_STORE (gtk_builder_get_object (priv->gtkbuilder, "tstore_fields"));

	return gdaex_query_editor;
}

GtkWidget
*gdaex_query_editor_get_dialog (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (gdaex_query_editor));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	if (priv->dialog == NULL)
		{
			priv->dialog = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "diag_query_editor"));
		}

	gdaex_query_editor_refresh_gui (gdaex_query_editor);

	return priv->dialog;
}

GtkWidget
*gdaex_query_editor_get_widget (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (gdaex_query_editor));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	if (priv->hpaned_main == NULL)
		{
			priv->hpaned_main = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "hpaned1"));
		}

	gdaex_query_editor_refresh_gui (gdaex_query_editor);

	return priv->hpaned_main;
}

gboolean
gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visibile)
{
	gboolean ret;

	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_new0 (GdaExQueryEditorTable, 1);
	table->name = g_strstrip (g_strdup (table_name));
	table->name_visible = g_strstrip (g_strdup (table_name_visibile));
	table->fields = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (priv->tables, table->name, table);

	ret = TRUE;

	return ret;
}

gboolean
gdaex_query_editor_table_add_field (GdaExQueryEditor *qe,
                                    const gchar *table_name,
                                    GdaExQueryEditorField field)
{
	gboolean ret;

	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_hash_table_lookup (priv->tables, table_name);
	if (table == NULL)
		{
			g_warning ("Table «%s» doesn't exists.", table_name);
			return FALSE;
		}

	g_hash_table_insert (table->fields, field.name, g_memdup (&field, sizeof (GdaExQueryEditorField)));

	ret = TRUE;

	return ret;
}

/* PRIVATE */
static void
gdaex_query_editor_set_property (GObject *object,
                   guint property_id,
                   const GValue *value,
                   GParamSpec *pspec)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;

	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_get_property (GObject *object,
                   guint property_id,
                   GValue *value,
                   GParamSpec *pspec)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;

	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_refresh_gui (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GHashTableIter hiter;
	gpointer key, value;
	GdaExQueryEditorTable *table;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_store_clear (priv->tstore_fields);

	g_hash_table_iter_init (&hiter, priv->tables);
	while (g_hash_table_iter_next (&hiter, &key, &value))
		{
			table = (GdaExQueryEditorTable *)value;

			gtk_tree_store_append (priv->tstore_fields, &iter, NULL);
			gtk_tree_store_set (priv->tstore_fields, &iter,
			                    COL_FIELDS_NAME, table->name,
			                    COL_FIELDS_VISIBLE_NAME, table->name_visible,
			                    -1);

			gdaex_query_editor_refresh_gui_add_fields (qe, table, &iter);
		}
}

static void
gdaex_query_editor_refresh_gui_add_fields (GdaExQueryEditor *qe,
                                           GdaExQueryEditorTable *table,
                                           GtkTreeIter *iter_parent)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GHashTableIter hiter;
	gpointer key, value;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	g_hash_table_iter_init (&hiter, table->fields);
	while (g_hash_table_iter_next (&hiter, &key, &value))
		{
			field = (GdaExQueryEditorField *)value;

			gtk_tree_store_append (priv->tstore_fields, &iter, iter_parent);
			gtk_tree_store_set (priv->tstore_fields, &iter,
			                    COL_FIELDS_NAME, field->name,
			                    COL_FIELDS_VISIBLE_NAME, field->name_visible,
			                    COL_FIELDS_DESCRIPTION, field->description,
			                    -1);
		}
}
