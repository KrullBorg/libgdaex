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

#include <stdarg.h>
#include <string.h>
#include <gtkdateentry.h>

#include "queryeditor.h"

typedef struct
	{
		gchar *name;
		gchar *name_visible;
		gboolean visible;

		GHashTable *fields;	/* GdaExQueryEditorField */
	} GdaExQueryEditorTable;

typedef struct
	{
		GdaExQueryEditorTable *table1;
		GdaExQueryEditorTable *table2;

		GdaExQueryEditorJoinType join_type;

		GSList *fields1;
		GSList *fields2;
	} GdaExQueryEditorRelation;

typedef enum
	{
		GDAEX_QE_LINK_TYPE_AND = 1,
		GDAEX_QE_LINK_TYPE_OR
	} GdaExQueryEditorLinkType;

enum
	{
		GDAEX_QE_PAGE_SHOW,
		GDAEX_QE_PAGE_WHERE,
		GDAEX_QE_PAGE_ORDER
	};

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

static gboolean _gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visibile,
                              gboolean is_visible);

static void gdaex_query_editor_refresh_gui (GdaExQueryEditor *qe);
static void gdaex_query_editor_refresh_gui_add_fields (GdaExQueryEditor *qe,
                                                       GdaExQueryEditorTable *table,
                                                       GtkTreeIter *iter_parent);

static void gdaex_query_editor_refill_always_show (GdaExQueryEditor *qe);

static void gdaex_query_editor_store_remove_iter (GdaExQueryEditor *qe,
                                       GtkTreeSelection *sel,
                                       GObject *store);
static void gdaex_query_editor_store_move_iter_up_down (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store,
                                      gboolean up);

static gboolean gdaex_query_editor_model_has_value (GtkTreeModel *model,
                                                    guint column,
                                                    GValue *value);

static void gdaex_query_editor_remove_child_from_vbx_values (GdaExQueryEditor *qe);

static gchar *gdaex_query_editor_get_where_type_str_from_type (guint where_type);
static gchar *gdaex_query_editor_get_link_type_str_from_type (guint link_type);

static void gdaex_query_editor_on_cb_where_type_changed (GtkComboBox *widget,
                                                         gpointer user_data);
static void gdaex_query_editor_on_btn_cancel_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data);

static void gdaex_query_editor_on_sel_fields_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);
static void gdaex_query_editor_on_trv_fields_row_activated (GtkTreeView *tree_view,
                                                            GtkTreePath *path,
                                                            GtkTreeViewColumn *column,
                                                            gpointer user_data);

static void gdaex_query_editor_show_add_iter (GdaExQueryEditor *qe, GtkTreeIter *iter);

static void gdaex_query_editor_on_btn_show_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_show_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

static void gdaex_query_editor_on_btn_where_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_where_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

static void gdaex_query_editor_on_btn_order_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_order_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

#define GDAEX_QUERY_EDITOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditorPrivate))

typedef struct _GdaExQueryEditorPrivate GdaExQueryEditorPrivate;
struct _GdaExQueryEditorPrivate
	{
		GdaEx *gdaex;

		GtkBuilder *gtkbuilder;

		GtkWidget *notebook;

		GtkWidget *hpaned_main;

		GtkWidget *vbx_values_container;
		GtkWidget *vbx_values;

		GtkTreeStore *tstore_fields;
		GtkListStore *lstore_show;
		GtkTreeStore *tstore_where;
		GtkListStore *lstore_order;

		GtkTreeSelection *sel_fields;
		GtkTreeSelection *sel_show;
		GtkTreeSelection *sel_where;
		GtkTreeSelection *sel_order;

		GHashTable *tables;	/* GdaExQueryEditorTable */

		GSList *relations;	/* GdaExQueryEditorRelation */

		/* for value choosing */
		guint editor_type;

		GtkWidget *hbox;
		GtkWidget *tbl;
		GtkWidget *cb_link_type;
		GtkWidget *not;
		GtkWidget *cb_where_type;
		GtkWidget *lbl_txt1;
		GtkWidget *txt1;
		GtkWidget *lbl_txt2;
		GtkWidget *txt2;
		GtkWidget *opt_asc;
		GtkWidget *opt_desc;

		GtkListStore *lstore_link_type;
		GtkListStore *lstore_where_type;
	};

G_DEFINE_TYPE (GdaExQueryEditor, gdaex_query_editor, G_TYPE_OBJECT)

enum
	{
		COL_FIELDS_TABLE_NAME,
		COL_FIELDS_NAME,
		COL_FIELDS_VISIBLE_NAME,
		COL_FIELDS_DESCRIPTION
	};

enum
	{
		COL_SHOW_TABLE_NAME,
		COL_SHOW_NAME,
		COL_SHOW_VISIBLE_NAME,
		COL_SHOW_ALIAS
	};

enum
	{
		COL_WHERE_LINK_TYPE,
		COL_WHERE_LINK_TYPE_VISIBLE,
		COL_WHERE_TABLE_NAME,
		COL_WHERE_NAME,
		COL_WHERE_VISIBLE_NAME,
		COL_WHERE_CONDITION_NOT,
		COL_WHERE_CONDITION_TYPE,
		COL_WHERE_CONDITION_TYPE_VISIBLE,
		COL_WHERE_CONDITION_FROM,
		COL_WHERE_CONDITION_FROM_SQL,
		COL_WHERE_CONDITION_TO,
		COL_WHERE_CONDITION_TO_SQL
	};

enum
	{
		COL_ORDER_TABLE_NAME,
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

	priv->hpaned_main = NULL;
	priv->relations = NULL;

	priv->lstore_link_type = gtk_list_store_new (2,
	                                             G_TYPE_UINT,
	                                             G_TYPE_STRING);
	priv->lstore_where_type = gtk_list_store_new (2,
	                                              G_TYPE_UINT,
	                                              G_TYPE_STRING);
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
	                                               "|hpaned_query_editor",
	                                               "|", -1),
	                                   &error);
	if (error != NULL)
		{
			g_warning ("Error on gui initialization: %s.",
			           error->message != NULL ? error->message : "no details");
			return NULL;
		}

	priv->notebook = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "notebook1"));

	priv->tstore_fields = GTK_TREE_STORE (gtk_builder_get_object (priv->gtkbuilder, "tstore_fields"));
	priv->lstore_show = GTK_LIST_STORE (gtk_builder_get_object (priv->gtkbuilder, "lstore_show"));
	priv->tstore_where = GTK_TREE_STORE (gtk_builder_get_object (priv->gtkbuilder, "tstore_where"));
	priv->lstore_order = GTK_LIST_STORE (gtk_builder_get_object (priv->gtkbuilder, "lstore_order"));

	priv->sel_fields = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview1")));
	priv->sel_show = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview2")));
	priv->sel_where = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview3")));
	priv->sel_order = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview4")));

	priv->vbx_values_container = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "vbox3"));
	priv->vbx_values = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "vbox4"));

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button16"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_cancel_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button15"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_save_clicked), (gpointer)gdaex_query_editor);

	g_signal_connect (priv->sel_fields, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_fields_changed), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "treeview1"), "row-activated",
	                  G_CALLBACK (gdaex_query_editor_on_trv_fields_row_activated), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button3"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button4"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button5"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button6"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button1"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_show, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_show_changed), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button7"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button8"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button9"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button10"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button2"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_where, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_where_changed), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button11"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button12"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button13"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button14"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button17"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_order, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_order_changed), (gpointer)gdaex_query_editor);

	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), FALSE);

	return gdaex_query_editor;
}

GtkWidget
*gdaex_query_editor_get_widget (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (gdaex_query_editor));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	if (priv->hpaned_main == NULL)
		{
			priv->hpaned_main = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "hpaned_query_editor"));
		}

	gdaex_query_editor_refresh_gui (gdaex_query_editor);

	return priv->hpaned_main;
}

void
gdaex_query_editor_set_show_visibile (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_SHOW);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

void
gdaex_query_editor_set_where_visibile (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_WHERE);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

void
gdaex_query_editor_set_order_visibile (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_ORDER);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

gboolean
gdaex_query_editor_get_show_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_SHOW);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_get_where_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_WHERE);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_get_order_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_ORDER);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visibile)
{
	return _gdaex_query_editor_add_table (qe, table_name, table_name_visibile, TRUE);
}

gboolean
gdaex_query_editor_table_add_field (GdaExQueryEditor *qe,
                                    const gchar *table_name,
                                    GdaExQueryEditorField field)
{
	gboolean ret;

	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *_field;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_hash_table_lookup (priv->tables, table_name);
	if (table == NULL)
		{
			g_warning ("Table «%s» doesn't exists.", table_name);
			return FALSE;
		}

	_field = g_memdup (&field, sizeof (GdaExQueryEditorField));
	_field->table_name = g_strdup (table_name);

	if (_field->name == NULL)
		{
			g_warning ("No field added: the field must have a name.");
			g_free (_field);
			return FALSE;
		}
	else
		{
			if (g_strcmp0 (g_strstrip (_field->name), "") == 0)
				{
					g_warning ("No field added: the field must have a name.");
					g_free (_field);
					return FALSE;
				}
		}

	if (_field->name_visible == NULL)
		{
			_field->name_visible = g_strdup (_field->name);
		}
	else
		{
			if (g_strcmp0 (g_strstrip (_field->name_visible), "") == 0)
				{
					_field->name_visible = g_strdup (_field->name);
				}
		}

	g_hash_table_insert (table->fields, _field->name, _field);

	ret = TRUE;

	return ret;
}

/**
 * gdaex_query_editor_add_relation:
 * @table1: relation's left part.
 * @table2: relation's right part.
 * @...: couples of fields name, one from @table1 and one from @table2.; 
 *       must be terminated with a #NULL value.
 *
 */
gboolean
gdaex_query_editor_add_relation (GdaExQueryEditor *qe,
                                 const gchar *table1,
                                 const gchar *table2,
                                 GdaExQueryEditorJoinType join_type,
                                 ...)
{
	GdaExQueryEditorPrivate *priv;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field1;
	GdaExQueryEditorField *field2;

	GdaExQueryEditorRelation *relation;

	va_list fields;
	gchar *field_name1;
	gchar *field_name2;

	gboolean create_relation;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	create_relation = FALSE;

	relation = g_new0 (GdaExQueryEditorRelation, 1);

	/* check if table exists */
	table = g_hash_table_lookup (priv->tables, table1);
	if (table == NULL)
		{
			g_warning ("Table «%s» doesn't exists.", table1);
			return FALSE;
		}
	relation->table1 = table;

	table = g_hash_table_lookup (priv->tables, table2);
	if (table == NULL)
		{
			g_warning ("Table «%s» doesn't exists.", table2);
			return FALSE;
		}
	relation->table2 = table;

	va_start (fields, join_type);

	while ((field_name1 = va_arg (fields, gchar *)) != NULL
	       && (field_name2 = va_arg (fields, gchar *)) != NULL)
		{
			field1 = g_hash_table_lookup (relation->table1->fields, field_name1);
			if (field1 == NULL)
				{
					continue;
				}

			field2 = g_hash_table_lookup (relation->table2->fields, field_name2);
			if (field2 == NULL)
				{
					continue;
				}

			relation->fields1 = g_slist_append (relation->fields1, field1);
			relation->fields2 = g_slist_append (relation->fields2, field2);
			create_relation = TRUE;
		}

	va_end (fields);

	if (create_relation)
		{
			relation->join_type = join_type;
			priv->relations = g_slist_append (priv->relations, relation);
		}
	else
		{
			g_warning ("Relation not created: no field added to the relation.");
			g_free (relation);
			return FALSE;
		}

	return TRUE;
}

void
gdaex_query_editor_clean_choices (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_show);
	gtk_tree_store_clear (priv->tstore_where);
	gtk_list_store_clear (priv->lstore_order);

	gdaex_query_editor_refill_always_show (qe);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static GDate
*gdaex_query_editor_get_gdate_from_sql (const gchar *sql)
{
	GDate *ret;

	gint year;
	gint month;
	gint day;

	ret = NULL;

	year = 0;
	month = 0;
	day = 0;

	if (strlen (sql) >= 10)
		{
			year = strtol (g_strndup (sql, 4), NULL, 10);
			month = strtol (g_strndup (sql + 5, 2), NULL, 10);
			day = strtol (g_strndup (sql + 8, 2), NULL, 10);

			ret = g_date_new_dmy (day,
			                      month,
			                      year);
		}

	return ret;
}

static GdaTimestamp
*gdaex_query_editor_get_gdatimestamp_from_sql (const gchar *sql)
{
	GdaTimestamp *ret;

	ret = NULL;

	if (strlen (sql) >= 10)
		{
			ret = g_new0 (GdaTimestamp, 1);

			ret->year = strtol (g_strndup (sql, 4), NULL, 10);
			ret->month = strtol (g_strndup (sql + 5, 2), NULL, 10);
			ret->day = strtol (g_strndup (sql + 8, 2), NULL, 10);

			if (strlen (sql) >= 12)
				{
					ret->hour = strtol (g_strndup (sql + 11, 2), NULL, 10);
					ret->minute = strtol (g_strndup (sql + 14, 2), NULL, 10);
					if (strlen (sql) >= 16)
						{
							ret->second = strtol (g_strndup (sql + 17, 2), NULL, 10);
						}
				}
		}

	return ret;
}

GdaExQueryEditorRelation
*gdaex_query_editor_get_table_relation (GdaExQueryEditor *qe,
                                        const gchar *table_name)
{
	GdaExQueryEditorRelation *ret;

	GdaExQueryEditorPrivate *priv;

	GSList *relations;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (priv->relations == NULL)
		{
			return NULL;
		}

	ret = NULL;

	relations = priv->relations;
	while (relations != NULL)
		{
			if (g_strcmp0 (((GdaExQueryEditorRelation *)relations->data)->table2->name, table_name) == 0)
				{
					ret = (GdaExQueryEditorRelation *)relations->data;
					break;
				}

			relations = g_slist_next (relations);
		}

	return ret;
}

static void
gdaex_query_editor_add_table_relation_to_gdasqlbuilder (GdaExQueryEditor *qe,
                                                        GdaSqlBuilder *sqlbuilder,
                                                        const gchar *table_name)
{
	GdaExQueryEditorRelation *relation;

	GSList *fields1;
	GSList *fields2;

	guint id_target1;
	guint id_target2;
	guint id_join1;
	guint id_join2;
	guint join_cond;

	GdaExQueryEditorPrivate *priv;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	/* check if table is part of a relation */
	relation = gdaex_query_editor_get_table_relation (qe, table_name);
	if (relation != NULL)
		{
			id_target1 = gda_sql_builder_select_add_target_id (sqlbuilder,
			                                                   gda_sql_builder_add_id (sqlbuilder, relation->table1->name),
			                                                   NULL);
			id_target2 = gda_sql_builder_select_add_target_id (sqlbuilder,
			                                                   gda_sql_builder_add_id (sqlbuilder, relation->table2->name),
			                                                   NULL);

			/* for each fields couple */
			fields1 = relation->fields1;
			fields2 = relation->fields2;
			while (fields1 != NULL)
				{
					id_join1 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (relation->table1->name, ".", ((GdaExQueryEditorField *)fields1->data)->name, NULL));
					id_join2 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (relation->table2->name, ".", ((GdaExQueryEditorField *)fields2->data)->name, NULL));

					join_cond = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_EQ,
					                                      id_join1, id_join2, 0);

					gda_sql_builder_select_join_targets (sqlbuilder, id_target1, id_target2,
					                                     relation->join_type == GDAEX_QE_JOIN_TYPE_LEFT ? GDA_SQL_SELECT_JOIN_LEFT : GDA_SQL_SELECT_JOIN_INNER,
					                                     join_cond);

					fields1 = g_slist_next (fields1);
					fields2 = g_slist_next (fields2);
				}
		}
}

GdaSqlBuilder
*gdaex_query_editor_get_sql_as_gdasqlbuilder (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GdaSqlBuilder *sqlbuilder;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *alias;
	gchar *asc_desc;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	sqlbuilder = gda_sql_builder_new (GDA_SQL_STATEMENT_SELECT);

	/* SHOW */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_show), &iter))
		{
			guint id_target1;
			guint id_target2;
			guint id_join1;
			guint id_join2;
			guint join_cond;

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
					                    COL_SHOW_TABLE_NAME, &table_name,
					                    COL_SHOW_NAME, &field_name,
					                    COL_SHOW_ALIAS, &alias,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					if (field->decode_table2 != NULL)
						{
							id_target1 = gda_sql_builder_select_add_target_id (sqlbuilder,
							                                                   gda_sql_builder_add_id (sqlbuilder, table->name),
							                                                   NULL);
							id_target2 = gda_sql_builder_select_add_target_id (sqlbuilder,
							                                                   gda_sql_builder_add_id (sqlbuilder, field->decode_table2),
							                                                   NULL);

							id_join1 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (field->table_name, ".", field->name, NULL));
							id_join2 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (field->decode_table2, ".", field->decode_field2, NULL));

							join_cond = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_EQ,
							                                      id_join1, id_join2, 0);

							gda_sql_builder_select_join_targets (sqlbuilder, id_target1, id_target2,
							                                     field->decode_join_type == GDAEX_QE_JOIN_TYPE_LEFT ? GDA_SQL_SELECT_JOIN_LEFT : GDA_SQL_SELECT_JOIN_INNER,
							                                     join_cond);
							gda_sql_builder_select_add_field (sqlbuilder, field->decode_field_to_show,
							                                  field->decode_table2,
							                                  alias != NULL && g_strcmp0 (alias, "") != 0 ? alias : field->decode_field_alias);
						}
					else
						{
							gda_sql_builder_select_add_field (sqlbuilder, field->name, table->name,
							                                  alias != NULL && g_strcmp0 (alias, "") != 0 ? alias : field->alias);
							gda_sql_builder_select_add_target_id (sqlbuilder,
							                                      gda_sql_builder_add_id (sqlbuilder, table->name),
							                                      NULL);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	/* WHERE */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			guint link_type;
			GdaSqlOperatorType link_op;
			gboolean not;
			guint where_type;
			GdaSqlOperatorType where_op;
			gchar *from_str;
			gchar *to_str;
			GDate *from_date;
			GDateTime *from_datetime;

			guint id_field;
			guint id_value1;
			guint id_value2;
			guint id_cond;
			guint id_cond_iter;

			gboolean case_insensitive;

			id_cond = 0;
			do
				{
					id_value2 = 0;

					gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
					                    COL_WHERE_LINK_TYPE, &link_type,
					                    COL_WHERE_TABLE_NAME, &table_name,
					                    COL_WHERE_NAME, &field_name,
					                    COL_WHERE_CONDITION_NOT, &not,
					                    COL_WHERE_CONDITION_TYPE, &where_type,
					                    COL_WHERE_CONDITION_FROM_SQL, &from_str,
					                    COL_WHERE_CONDITION_TO_SQL, &to_str,
					                    -1);

					case_insensitive = (where_type == GDAEX_QE_WHERE_TYPE_ISTARTS
					                    || where_type == GDAEX_QE_WHERE_TYPE_ICONTAINS
					                    || where_type == GDAEX_QE_WHERE_TYPE_IENDS);

					if (from_str == NULL)
						{
							from_str = g_strdup ("");
						}
					else
						{
							if (case_insensitive)
								{
									from_str = g_utf8_strdown (from_str, -1);
								}
						}
					if (to_str != NULL && g_strcmp0 (g_strstrip (to_str), "") != 0)
						{
							if (case_insensitive)
								{
									to_str = g_utf8_strdown (to_str, -1);
								}
						}
					else
						{
							to_str = NULL;
						}

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_STARTS:
							case GDAEX_QE_WHERE_TYPE_ISTARTS:
								from_str = g_strconcat (from_str, "%", NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat (to_str, "%", NULL);
									}
								break;

							case GDAEX_QE_WHERE_TYPE_CONTAINS:
							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
								from_str = g_strconcat ("%", from_str, "%", NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat ("%", to_str, "%", NULL);
									}
								break;

							case GDAEX_QE_WHERE_TYPE_ENDS:
							case GDAEX_QE_WHERE_TYPE_IENDS:
								from_str = g_strconcat ("%", from_str, NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat ("%", to_str, NULL);
									}
								break;
						}

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					id_field = gda_sql_builder_add_id (sqlbuilder,
					                                   g_strconcat (case_insensitive ? "LOWER(" : "",
					                                                table->name, ".", field->name,
					                                                case_insensitive ? ")" : "",
					                                                NULL));

					switch (link_type)
						{
							case GDAEX_QE_LINK_TYPE_AND:
								link_op = GDA_SQL_OPERATOR_TYPE_AND;
								break;

							case GDAEX_QE_LINK_TYPE_OR:
								link_op = GDA_SQL_OPERATOR_TYPE_OR;
								break;
						}

					if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
						{
							id_value1 = 0;
						}
					else
						{
							switch (field->type)
								{
									case GDAEX_QE_FIELD_TYPE_TEXT:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_STRING, from_str);
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_STRING, to_str);
											}
										break;

									case GDAEX_QE_FIELD_TYPE_INTEGER:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_INT, strtol (from_str, NULL, 10));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_INT, strtol (to_str, NULL, 10));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DOUBLE:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DOUBLE, g_strtod (from_str, NULL));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DOUBLE, g_strtod (to_str, NULL));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DATE:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DATE, gdaex_query_editor_get_gdate_from_sql (from_str));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DATE, gdaex_query_editor_get_gdate_from_sql (to_str));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DATETIME:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, GDA_TYPE_TIMESTAMP, gdaex_query_editor_get_gdatimestamp_from_sql (from_str));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, GDA_TYPE_TIMESTAMP, gdaex_query_editor_get_gdatimestamp_from_sql (to_str));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_TIME:
										/* TODO
										type = G_TYPE_DATE_TIME; */
										break;
								};
						}

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_EQ;
								break;

							case GDAEX_QE_WHERE_TYPE_STARTS:
							case GDAEX_QE_WHERE_TYPE_CONTAINS:
							case GDAEX_QE_WHERE_TYPE_ENDS:
							case GDAEX_QE_WHERE_TYPE_ISTARTS:
							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
							case GDAEX_QE_WHERE_TYPE_IENDS:
								where_op = GDA_SQL_OPERATOR_TYPE_LIKE;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT:
								where_op = GDA_SQL_OPERATOR_TYPE_GT;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_GEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS:
								where_op = GDA_SQL_OPERATOR_TYPE_LT;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_LEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_BETWEEN:
								where_op = GDA_SQL_OPERATOR_TYPE_BETWEEN;
								break;

							case GDAEX_QE_WHERE_TYPE_IS_NULL:
								if (not)
									{
										where_op = GDA_SQL_OPERATOR_TYPE_ISNOTNULL;
									}
								else
									{
										where_op = GDA_SQL_OPERATOR_TYPE_ISNULL;
									}
								break;

							default:
								g_warning ("Where type «%d» not valid.", where_type);
								continue;
						}

					id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, where_op, id_field, id_value1, id_value2);
					if (not && where_type != GDAEX_QE_WHERE_TYPE_IS_NULL)
						{
							id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_NOT, id_cond_iter, 0, 0);
						}
					if (id_cond == 0)
						{
							id_cond = id_cond_iter;
						}
					else
						{
							id_cond = gda_sql_builder_add_cond (sqlbuilder, link_op, id_cond, id_cond_iter, 0);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_where), &iter));

			if (id_cond != 0)
				{
					gda_sql_builder_set_where (sqlbuilder, id_cond);
				}
		}

	/* ORDER */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_order), &iter))
		{
			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
					                    COL_ORDER_TABLE_NAME, &table_name,
					                    COL_ORDER_NAME, &field_name,
					                    COL_ORDER_ORDER, &asc_desc,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					gda_sql_builder_select_order_by (sqlbuilder,
					                                 gda_sql_builder_add_id (sqlbuilder, field->name),
					                                 (g_strcmp0 (asc_desc, "ASC") == 0),
					                                 NULL);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_order), &iter));
		}

	return sqlbuilder;
}

const gchar
*gdaex_query_editor_get_sql (GdaExQueryEditor *qe)
{
	const gchar *ret;

	GdaExQueryEditorPrivate *priv;

	GdaSqlBuilder *sqlbuilder;
	GdaStatement *stmt;
	GError *error;

	ret = NULL;

	sqlbuilder = gdaex_query_editor_get_sql_as_gdasqlbuilder (qe);
	if (sqlbuilder == NULL)
		{
			return ret;
		}

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	error = NULL;
	stmt = gda_sql_builder_get_statement (sqlbuilder, &error);
	if (stmt == NULL || error != NULL)
		{
			g_object_unref (sqlbuilder);
			g_warning ("Unable to create GdaStatement: %s.",
			           error != NULL && error->message != NULL ? error->message : "no details");
			return NULL;
		}

	error = NULL;
	ret = gda_statement_to_sql_extended (stmt,
	                                     (GdaConnection *)gdaex_get_gdaconnection (priv->gdaex),
	                                     NULL, 0, NULL, &error);
	if (error != NULL)
		{
			ret = NULL;
			g_warning ("Unable to create sql: %s.",
			           error->message != NULL ? error->message : "no details");
		}

	g_object_unref (sqlbuilder);

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_select (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
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

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_from (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
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

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_where (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
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

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_order (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
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

	return ret;
}


xmlNode
*gdaex_query_editor_get_sql_as_xml (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	xmlNode *ret;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	xmlNode *node;

	ret = NULL;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	ret = xmlNewNode (NULL, "gdaex_query_editor_choices");

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_show), &iter))
		{
			gchar *alias;
			xmlNode *node_show;

			node_show = xmlNewNode (NULL, "show");
			xmlAddChild (ret, node_show);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
					                    COL_SHOW_TABLE_NAME, &table_name,
					                    COL_SHOW_NAME, &field_name,
					                    COL_SHOW_ALIAS, &alias,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_show, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					if (alias != NULL && g_strcmp0 (alias, "") != 0)
						{
							xmlNewProp (node, "alias", alias);
						}
					else
						{
							xmlNewProp (node, "alias", field->alias);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			xmlNode *node_where;

			GtkTreePath *path;
			gint *indices;

			gboolean not;
			guint link_type;
			guint where_type;
			gchar *from_str;
			gchar *to_str;
			gchar *str_link;
			gchar *str_op;

			node_where = xmlNewNode (NULL, "where");
			xmlAddChild (ret, node_where);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
					                    COL_WHERE_LINK_TYPE, &link_type,
					                    COL_WHERE_TABLE_NAME, &table_name,
					                    COL_WHERE_NAME, &field_name,
					                    COL_WHERE_CONDITION_NOT, &not,
					                    COL_WHERE_CONDITION_TYPE, &where_type,
					                    COL_WHERE_CONDITION_FROM_SQL, &from_str,
					                    COL_WHERE_CONDITION_TO_SQL, &to_str,
					                    -1);

					switch (link_type)
						{
							case GDAEX_QE_LINK_TYPE_AND:
								str_link = g_strdup ("AND");
								break;

							case GDAEX_QE_LINK_TYPE_OR:
								str_link = g_strdup ("OR");
								break;

							default:
								path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter);
								indices = gtk_tree_path_get_indices (path);
								if (indices[0] != 0)
									{
										g_warning ("Link type «%d» not valid.", link_type);
										continue;
									}
								else
									{
										str_link = g_strdup ("");
										break;
									}
						}

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								str_op = g_strdup ("EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_STARTS:
								str_op = g_strdup ("STARTS");
								break;

							case GDAEX_QE_WHERE_TYPE_CONTAINS:
								str_op = g_strdup ("CONTAINS");
								break;

							case GDAEX_QE_WHERE_TYPE_ENDS:
								str_op = g_strdup ("ENDS");
								break;

							case GDAEX_QE_WHERE_TYPE_ISTARTS:
								str_op = g_strdup ("ISTARTS");
								break;

							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
								str_op = g_strdup ("ICONTAINS");
								break;

							case GDAEX_QE_WHERE_TYPE_IENDS:
								str_op = g_strdup ("IENDS");
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT:
								str_op = g_strdup ("GREAT");
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
								str_op = g_strdup ("GREAT_EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_LESS:
								str_op = g_strdup ("LESS");
								break;

							case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
								str_op = g_strdup ("LESS_EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_BETWEEN:
								str_op = g_strdup ("BETWEEN");
								break;

							case GDAEX_QE_WHERE_TYPE_IS_NULL:
								str_op = g_strdup ("IS_NULL");
								break;

							default:
								g_warning ("Where type «%d» not valid.", where_type);
								continue;
						}

					node = xmlNewNode (NULL, "field");

					xmlAddChild (node_where, node);

					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					xmlNewProp (node, "link_type", str_link);
					xmlNewProp (node, "not", (not ? "y" : "n"));
					xmlNewProp (node, "where_type", str_op);
					xmlNewProp (node, "from", from_str);
					xmlNewProp (node, "to", to_str);

					g_free (str_op);
					g_free (str_link);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_where), &iter));
		}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_order), &iter))
		{
			gchar *asc_desc;
			xmlNode *node_order;

			node_order = xmlNewNode (NULL, "order");
			xmlAddChild (ret, node_order);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
					                    COL_ORDER_TABLE_NAME, &table_name,
					                    COL_ORDER_NAME, &field_name,
					                    COL_ORDER_ORDER, &asc_desc,
					                    -1);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_order, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					xmlNewProp (node, "asc_desc", asc_desc);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_order), &iter));
		}

	return ret;
}

void
gdaex_query_editor_load_choices_from_xml (GdaExQueryEditor *qe, xmlNode *root,
                                          gboolean clean)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *name_visible;
	gchar *alias;
	GValue *v_table_field;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	xmlNode *node;
	xmlNode *node_field;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	g_return_if_fail (xmlStrEqual (root->name, "gdaex_query_editor_choices"));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (clean)
		{
			gdaex_query_editor_clean_choices (qe);
		}

	node = root->children;
	while (node != NULL)
	{
		if (xmlStrEqual (node->name, "show")
		    || xmlStrEqual (node->name, "where")
		    || xmlStrEqual (node->name, "order"))
			{
				node_field = node->children;
				while (node_field != NULL)
					{
						if (xmlStrEqual (node_field->name, "field"))
							{
								table_name = xmlGetProp (node_field, "table");
								table = g_hash_table_lookup (priv->tables, table_name);
								if (table == NULL)
									{
										g_warning ("Table «%s» not found.", table_name);
										continue;
									}

								field_name = xmlGetProp (node_field, "field");
								field = g_hash_table_lookup (table->fields, field_name);
								if (field == NULL)
									{
										g_warning ("Field «%s» not found in table «%s».", field_name, table_name);
										continue;
									}

								name_visible = g_strconcat (table->name_visible, " - ", field->name_visible, NULL);
								v_table_field = gda_value_new (G_TYPE_STRING);
								g_value_set_string (v_table_field, name_visible);

								if (xmlStrEqual (node->name, "show"))
									{
										if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show), COL_SHOW_VISIBLE_NAME, v_table_field))
											{
												alias = xmlGetProp (node_field, "alias");
												if (alias == NULL)
													{
														alias = g_strdup ("");
													}

												gtk_list_store_append (priv->lstore_show, &iter);
												gtk_list_store_set (priv->lstore_show, &iter,
												                    COL_SHOW_TABLE_NAME, field->table_name,
												                    COL_SHOW_NAME, field_name,
												                    COL_SHOW_VISIBLE_NAME, name_visible,
												                    COL_SHOW_ALIAS, alias,
												                    -1);
											}
									}
								else if (xmlStrEqual (node->name, "where"))
									{
										gchar *link;
										gchar *not;
										gchar *condition;
										gchar *from;
										gchar *to;
										gchar *from_sql;
										gchar *to_sql;

										GdaExQueryEditorLinkType link_type;
										GdaExQueryEditorWhereType where_type;

										link = xmlGetProp (node_field, "link_type");
										not = xmlGetProp (node_field, "not");
										condition = xmlGetProp (node_field, "where_type");

										from_sql = xmlGetProp (node_field, "from");
										to_sql = xmlGetProp (node_field, "to");
										if (field->type == GDAEX_QE_FIELD_TYPE_DATE)
											{
												GDate *gdate = gdaex_query_editor_get_gdate_from_sql (from_sql);
												from = g_strdup_printf ("%02d/%02d/%04d",
												                        g_date_get_day (gdate),
												                        g_date_get_month (gdate),
												                        g_date_get_year (gdate));

												gdate = gdaex_query_editor_get_gdate_from_sql (to_sql);
												to = g_strdup_printf ("%02d/%02d/%04d",
												                      g_date_get_day (gdate),
												                      g_date_get_month (gdate),
												                      g_date_get_year (gdate));
											}
										else if (field->type = GDAEX_QE_FIELD_TYPE_DATETIME)
											{
												GdaTimestamp *gdate = gdaex_query_editor_get_gdatimestamp_from_sql (from_sql);
												from = g_strdup_printf ("%02d/%02d/%04d %02d.%02d.%02d",
												                        gdate->day,
												                        gdate->month,
												                        gdate->year,
												                        gdate->hour,
												                        gdate->minute,
												                        gdate->second);

												gdate = gdaex_query_editor_get_gdatimestamp_from_sql (to_sql);
												to = g_strdup_printf ("%02d/%02d/%04d %02d.%02d.%02d",
												                      gdate->day,
												                      gdate->month,
												                      gdate->year,
												                      gdate->hour,
												                      gdate->minute,
												                      gdate->second);
											}
										else if (field->type = GDAEX_QE_FIELD_TYPE_TIME)
											{
												/* TODO */
											}
										else
											{
												from = xmlGetProp (node_field, "from");
												to = xmlGetProp (node_field, "to");
											}

										link_type = 0;
										if (g_strcmp0 (link, "AND") == 0)
											{
												link_type = GDAEX_QE_LINK_TYPE_AND;
											}
										else if (g_strcmp0 (link, "OR") == 0)
											{
												link_type = GDAEX_QE_LINK_TYPE_OR;
											}

										if (g_strcmp0 (condition, "EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_EQUAL;
											}
										else if (g_strcmp0 (condition, "STARTS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_STARTS;
											}
										else if (g_strcmp0 (condition, "CONTAINS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_CONTAINS;
											}
										else if (g_strcmp0 (condition, "ENDS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ENDS;
											}
										else if (g_strcmp0 (condition, "ISTARTS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ISTARTS;
											}
										else if (g_strcmp0 (condition, "ICONTAINS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ICONTAINS;
											}
										else if (g_strcmp0 (condition, "IENDS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_IENDS;
											}
										else if (g_strcmp0 (condition, "GREAT") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_GREAT;
											}
										else if (g_strcmp0 (condition, "GREAT_EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_GREAT_EQUAL;
											}
										else if (g_strcmp0 (condition, "LESS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_LESS;
											}
										else if (g_strcmp0 (condition, "LESS_EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_LESS_EQUAL;
											}
										else if (g_strcmp0 (condition, "BETWEEN") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_BETWEEN;
											}
										else if (g_strcmp0 (condition, "IS_NULL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_IS_NULL;
											}

										if (where_type != 0)
											{
												gtk_tree_store_append (priv->tstore_where, &iter, NULL);
												gtk_tree_store_set (priv->tstore_where, &iter,
												                    COL_WHERE_LINK_TYPE, link_type,
												                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
												                    COL_WHERE_TABLE_NAME, table_name,
												                    COL_WHERE_NAME, field_name,
												                    COL_WHERE_VISIBLE_NAME, name_visible,
												                    COL_WHERE_CONDITION_NOT, (g_strcmp0 (not, "n") != 0),
												                    COL_WHERE_CONDITION_TYPE, where_type,
												                    COL_WHERE_CONDITION_TYPE_VISIBLE, gdaex_query_editor_get_where_type_str_from_type (where_type),
												                    COL_WHERE_CONDITION_FROM, from,
												                    COL_WHERE_CONDITION_FROM_SQL, from_sql,
												                    COL_WHERE_CONDITION_TO, (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN ? to : ""),
												                    COL_WHERE_CONDITION_TO_SQL, (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN ? to_sql : ""),
												                    -1);
											}

										g_free (from);
										g_free (to);
										g_free (from_sql);
										g_free (to_sql);
										g_free (link);
										g_free (not);
										g_free (condition);
									}
								else if (xmlStrEqual (node->name, "order"))
									{
										if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show), COL_SHOW_VISIBLE_NAME, v_table_field))
											{
												gchar *asc_desc;
												asc_desc = xmlGetProp (node_field, "asc_desc");

												if (g_strcmp0 (asc_desc, "ASC") == 0
												    || g_strcmp0 (asc_desc, "DESC") == 0)
													{
														gtk_list_store_append (priv->lstore_order, &iter);
														gtk_list_store_set (priv->lstore_order, &iter,
														                    COL_ORDER_TABLE_NAME, field->table_name,
														                    COL_ORDER_NAME, field_name,
														                    COL_ORDER_VISIBLE_NAME, name_visible,
														                    COL_ORDER_ORDER, asc_desc,
														                    -1);
													}
											}
									}

								g_free (field_name);
								g_free (table_name);
								g_free (name_visible);
							}

						node_field = node_field->next;
					}
			}
		node = node->next;
	}
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

static gboolean
_gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visible,
                              gboolean is_visible)
{
	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;

	gchar *_table_name;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);
	g_return_val_if_fail (table_name != NULL, FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_new0 (GdaExQueryEditorTable, 1);

	_table_name = g_strdup (table_name);

	g_return_val_if_fail (g_strcmp0 (g_strstrip (_table_name), "") != 0, FALSE);

	table->name = g_strdup (_table_name);
	g_free (_table_name);

	if (table_name_visible == NULL)
		{
			table->name_visible = g_strdup (table->name);
		}
	else
		{
			_table_name = g_strdup (table_name_visible);
			if (g_strcmp0 (g_strstrip (_table_name), "") == 0)
				{
					table->name_visible = g_strdup (table->name);
					g_free (_table_name);
				}
			else
				{
					table->name_visible = g_strdup (_table_name);
					g_free (_table_name);
				}
		}

	table->fields = g_hash_table_new (g_str_hash, g_str_equal);
	table->visible = is_visible;

	g_hash_table_insert (priv->tables, table->name, table);

	return TRUE;
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

			if (table->visible)
				{
					gtk_tree_store_append (priv->tstore_fields, &iter, NULL);
					gtk_tree_store_set (priv->tstore_fields, &iter,
					                    COL_FIELDS_NAME, table->name,
					                    COL_FIELDS_VISIBLE_NAME, table->name_visible,
					                    -1);

					gdaex_query_editor_refresh_gui_add_fields (qe, table, &iter);

					/* if no fields, remove table */
					if (!gtk_tree_model_iter_has_child (GTK_TREE_MODEL (priv->tstore_fields), &iter))
						{
							gtk_tree_store_remove (priv->tstore_fields, &iter);
						}
				}
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

			if (field->for_show || field->for_where || field->for_order)
				{
					gtk_tree_store_append (priv->tstore_fields, &iter, iter_parent);
					gtk_tree_store_set (priv->tstore_fields, &iter,
					                    COL_FIELDS_TABLE_NAME, table->name,
					                    COL_FIELDS_NAME, field->name,
					                    COL_FIELDS_VISIBLE_NAME, field->name_visible,
					                    COL_FIELDS_DESCRIPTION, field->description,
					                    -1);
				}

			if (field->for_show && field->always_showed)
				{
					gdaex_query_editor_show_add_iter (qe, &iter);
				}
		}
}

static void
gdaex_query_editor_refill_always_show (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter_table;
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_fields), &iter_table))
		{
			do
				{
					if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tstore_fields), &iter, &iter_table))
						{
							do
								{
									gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
									                    COL_FIELDS_TABLE_NAME, &table_name,
									                    COL_FIELDS_NAME, &field_name,
									                    -1);
									table = g_hash_table_lookup (priv->tables, table_name);
									field = g_hash_table_lookup (table->fields, field_name);
									if (field->always_showed)
										{
											gdaex_query_editor_show_add_iter (qe, &iter);
										}
								} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_fields), &iter));
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_fields), &iter_table));
		}
}

static void
gdaex_query_editor_store_remove_iter (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	guint risp;

	g_return_if_fail (GTK_IS_LIST_STORE (store) || GTK_IS_TREE_STORE (store));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (sel, NULL, &iter))
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_QUESTION,
			                                 GTK_BUTTONS_YES_NO,
			                                 "Are you sure you want to remove the selected field?");
			risp = gtk_dialog_run (GTK_DIALOG (dialog));
			if (risp == GTK_RESPONSE_YES)
				{
					if (GTK_IS_LIST_STORE (store))
						{
							gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
						}
					else
						{
							gtk_tree_store_remove (GTK_TREE_STORE (store), &iter);
						}
				}
			gtk_widget_destroy (dialog);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 "You must select a field before.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_store_move_iter_up_down (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store,
                                      gboolean up)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeIter iter_prev;

	gboolean swap_link;
	guint link_type;

	GtkWidget *dialog;

	g_return_if_fail (GTK_IS_LIST_STORE (store) || GTK_IS_TREE_STORE (store));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (sel, NULL, &iter))
		{
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
			if (path != NULL)
				{
					/* if the first item of where is changed,
					   swap link type */
					swap_link = FALSE;
					if (sel == priv->sel_where)
						{
							gint *indices = gtk_tree_path_get_indices (path);
							if (indices[0] == (up ? 1 : 0))
								{
									swap_link = TRUE;
								}
						}

					up ? gtk_tree_path_prev (path) : gtk_tree_path_next (path);
					if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter_prev, path))
						{
							if (up)
								{
									if (GTK_IS_LIST_STORE (store))
										{
											gtk_list_store_move_before (GTK_LIST_STORE (store), &iter, &iter_prev);
										}
									else
										{
											gtk_tree_store_move_before (GTK_TREE_STORE (store), &iter, &iter_prev);
										}
								}
							else
								{
									if (GTK_IS_LIST_STORE (store))
										{
											gtk_list_store_move_after (GTK_LIST_STORE (store), &iter, &iter_prev);
										}
									else
										{
											gtk_tree_store_move_after (GTK_TREE_STORE (store), &iter, &iter_prev);
										}
								}

							if (swap_link)
								{
									gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter_prev);
									gtk_tree_model_get (GTK_TREE_MODEL (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, &link_type,
									                    -1);
									gtk_tree_store_set (GTK_TREE_STORE (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, 0,
									                    COL_WHERE_LINK_TYPE_VISIBLE, "",
									                    -1);

									gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter_prev);
									gtk_tree_store_set (GTK_TREE_STORE (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, link_type,
									                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
									                    -1);
								}
						}
				}
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 "You must select a field before.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static gboolean
gdaex_query_editor_model_has_value (GtkTreeModel *model,
                                    guint column,
                                    GValue *value)
{
	gboolean ret;

	GtkTreeIter iter;
	GValue iter_value = { 0 };

	g_return_val_if_fail (gtk_tree_model_get_column_type (model, column) == G_VALUE_TYPE (value), FALSE);

	ret = FALSE;

	if (gtk_tree_model_get_iter_first (model, &iter))
		{
			do
				{
					gtk_tree_model_get_value (model, &iter, column, &iter_value);
					if (!gda_value_differ (value, &iter_value))
						{
							g_value_unset (&iter_value);
							return TRUE;
						}
					g_value_unset (&iter_value);
				} while (gtk_tree_model_iter_next (model, &iter));
		}

	return ret;
}

static void
gdaex_query_editor_remove_child_from_vbx_values (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	GList *lw = gtk_container_get_children (GTK_CONTAINER (priv->vbx_values));
	if (lw != NULL && lw->data != NULL)
		{
			gtk_container_remove (GTK_CONTAINER (priv->vbx_values), (GtkWidget *)lw->data);
		}
}

static gchar *
gdaex_query_editor_get_where_type_str_from_type (guint where_type)
{
	gchar *ret;

	ret = NULL;

	switch (where_type)
		{
			case GDAEX_QE_WHERE_TYPE_EQUAL:
				ret = g_strdup ("Equal");
				break;

			case GDAEX_QE_WHERE_TYPE_STARTS:
				ret = g_strdup ("Starts with");
				break;

			case GDAEX_QE_WHERE_TYPE_CONTAINS:
				ret = g_strdup ("Contains");
				break;

			case GDAEX_QE_WHERE_TYPE_ENDS:
				ret = g_strdup ("Ends with");
				break;

			case GDAEX_QE_WHERE_TYPE_ISTARTS:
				ret = g_strdup ("Case-insensitive starts with");
				break;

			case GDAEX_QE_WHERE_TYPE_ICONTAINS:
				ret = g_strdup ("Case-insensitive contains");
				break;

			case GDAEX_QE_WHERE_TYPE_IENDS:
				ret = g_strdup ("Case-insensitive ends with");
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT:
				ret = g_strdup ("Greater");
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
				ret = g_strdup ("Greater or equal");
				break;

			case GDAEX_QE_WHERE_TYPE_LESS:
				ret = g_strdup ("Lesser");
				break;

			case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
				ret = g_strdup ("Lesser or equal");
				break;

			case GDAEX_QE_WHERE_TYPE_BETWEEN:
				ret = g_strdup ("Between");
				break;

			case GDAEX_QE_WHERE_TYPE_IS_NULL:
				ret = g_strdup ("Is NULL");
				break;
		};

	return ret;
}

static gchar *
gdaex_query_editor_get_link_type_str_from_type (guint link_type)
{
	gchar *ret;

	ret = NULL;

	switch (link_type)
		{
			case GDAEX_QE_LINK_TYPE_AND:
				ret = g_strdup ("And");
				break;

			case GDAEX_QE_LINK_TYPE_OR:
				ret = g_strdup ("Or");
				break;
		};

	return ret;
}

static void
gdaex_query_editor_on_sel_fields_changed (GtkTreeSelection *treeselection,
                                          gpointer user_data)
{
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	gchar *table_field;
	GValue *v_table_field;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), FALSE);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			if (table_name == NULL || g_strcmp0 (table_name, "") == 0)
				{
					/* cannot add a table */
					return;
				}

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			if (field->for_where)
				{
					gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), TRUE);
				}

			table_field = g_strconcat (table->name_visible, " - ", field->name_visible, NULL);
			v_table_field = gda_value_new (G_TYPE_STRING);
			g_value_set_string (v_table_field, table_field);
			if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show),
			                                         COL_ORDER_VISIBLE_NAME,
			                                         v_table_field))
				{
					if (!field->always_showed && field->for_show)
						{
							gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), TRUE);
						}
				}
			if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_order),
			                                         COL_ORDER_VISIBLE_NAME,
			                                         v_table_field))
				{
				if (field->for_order)
					{
						gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), TRUE);
					}
				}

			g_free (table_field);
			g_value_unset (v_table_field);
		}
}

static void
gdaex_query_editor_on_trv_fields_row_activated (GtkTreeView *tree_view,
                                                GtkTreePath *path,
                                                GtkTreeViewColumn *column,
                                                gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	guint page;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (!gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			return;
		}

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook));
	switch (page)
		{
			case GDAEX_QE_PAGE_SHOW:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button3")));
					}
				break;

			case GDAEX_QE_PAGE_WHERE:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button7")));
					}
				break;

			case GDAEX_QE_PAGE_ORDER:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button11")));
					}
				break;
		}
}

static void
gdaex_query_editor_on_cb_where_type_changed (GtkComboBox *widget,
                                             gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeModel *model;
	GtkTreeIter iter;
	guint where_type;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter))
		{
			model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_where_type));

			gtk_tree_model_get (model, &iter,
			                    0, &where_type,
			                    -1);

			gtk_widget_set_visible (priv->lbl_txt1, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);
			gtk_widget_set_visible (priv->txt1, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);

			if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
				{
					if (GTK_IS_DATE_ENTRY (priv->txt1))
						{
							gtk_date_entry_set_date_gdatetime (GTK_DATE_ENTRY (priv->txt1), NULL);
						}
					else
						{
							gtk_entry_set_text (GTK_ENTRY (priv->txt1), "");
						}
				}

			gtk_widget_set_visible (priv->lbl_txt2, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);
			gtk_widget_set_visible (priv->txt2, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);

			if (where_type != GDAEX_QE_WHERE_TYPE_BETWEEN)
				{
					if (GTK_IS_DATE_ENTRY (priv->txt2))
						{
							gtk_date_entry_set_date_gdatetime (GTK_DATE_ENTRY (priv->txt2), NULL);
						}
					else
						{
							gtk_entry_set_text (GTK_ENTRY (priv->txt2), "");
						}
					gtk_table_set_row_spacing (GTK_TABLE (priv->tbl), 1, 0);
				}
			else
				{
					gtk_date_entry_set_date_gdatetime (GTK_DATE_ENTRY (priv->txt1),
					                                   g_date_time_new_now_local ());
					gtk_date_entry_set_date_gdatetime (GTK_DATE_ENTRY (priv->txt2),
					                                   g_date_time_new_now_local ());
					gtk_table_set_row_spacing (GTK_TABLE (priv->tbl), 1, 5);
				}
		}
}

static void
gdaex_query_editor_on_btn_cancel_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_remove_child_from_vbx_values (qe);
	gtk_widget_hide (priv->vbx_values_container);

	switch (priv->editor_type)
		{
			case GDAEX_QE_PAGE_SHOW:
				gtk_tree_selection_unselect_all (priv->sel_show);
				break;

			case GDAEX_QE_PAGE_WHERE:
				gtk_tree_selection_unselect_all (priv->sel_where);
				break;

			case GDAEX_QE_PAGE_ORDER:
				gtk_tree_selection_unselect_all (priv->sel_order);
				break;
		}
}

static void
gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkWidget *dialog;

	GtkTreeIter iter;

	gchar *val1;
	gchar *val2;
	gchar *asc_desc;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	val1 = NULL;
	val2 = NULL;
	asc_desc = NULL;

	switch (priv->editor_type)
		{
			case GDAEX_QE_PAGE_SHOW:
				if (gtk_tree_selection_get_selected (priv->sel_show, NULL, &iter))
					{
						val1 = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt1));
						if (val1 == NULL)
							{
								val1 = g_strdup ("");
							}
						else
							{
								val1 = g_strstrip (g_strdup (val1));
							}
						gtk_list_store_set (priv->lstore_show, &iter,
						                    COL_SHOW_ALIAS, val1,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_show);
					}
				break;

			case GDAEX_QE_PAGE_WHERE:
				if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
					{
						GtkTreeModel *model;
						GtkTreeIter iter_val;

						guint link_type;
						guint where_type;

						gchar *val1_sql;
						gchar *val2_sql;

						val1_sql = NULL;
						val2_sql = NULL;

						/* if it is the first condition, "link" doesn't is visibile */
						GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter);
						gint *indices = gtk_tree_path_get_indices (path);
						if (indices[0] != 0)
							{
								model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_link_type));
								if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_link_type), &iter_val))
									{
										dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
										                                 GTK_DIALOG_DESTROY_WITH_PARENT,
										                                 GTK_MESSAGE_WARNING,
										                                 GTK_BUTTONS_OK,
										                                 "You must select a link's type before.");
										gtk_dialog_run (GTK_DIALOG (dialog));
										gtk_widget_destroy (dialog);
										return;
									}

								gtk_tree_model_get (model, &iter_val,
								                    0, &link_type,
								                    -1);
							}
						else
							{
								link_type = 0;
							}

						model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_where_type));
						if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_val))
							{
								dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
								                                 GTK_DIALOG_DESTROY_WITH_PARENT,
								                                 GTK_MESSAGE_WARNING,
								                                 GTK_BUTTONS_OK,
								                                 "You must select a condition's type before.");
								gtk_dialog_run (GTK_DIALOG (dialog));
								gtk_widget_destroy (dialog);
								return;
							}

						gtk_tree_model_get (model, &iter_val,
						                    0, &where_type,
						                    -1);

						if (GTK_IS_DATE_ENTRY (priv->txt1))
							{
								val1 = (gchar *)gtk_date_entry_get_strf (GTK_DATE_ENTRY (priv->txt1), gtk_date_entry_is_time_visible (GTK_DATE_ENTRY (priv->txt1)) ? "dmYHMS" : "dmY", NULL, NULL);
								val1_sql = (gchar *)gtk_date_entry_get_sql (GTK_DATE_ENTRY (priv->txt1));
							}
						else
							{
								val1 = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt1));
								val1_sql = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt1));
							}
						if (val1 == NULL)
							{
								val1 = g_strdup ("");
							}
						else
							{
								val1 = g_strdup (val1);
							}
						if (val1_sql == NULL)
							{
								val1_sql = g_strdup ("");
							}
						else
							{
								val1_sql = g_strdup (val1_sql);
							}

						if (GTK_IS_DATE_ENTRY (priv->txt2))
							{
								val2 = (gchar *)gtk_date_entry_get_strf (GTK_DATE_ENTRY (priv->txt2), gtk_date_entry_is_time_visible (GTK_DATE_ENTRY (priv->txt2)) ? "dmYHMS" : "dmY", NULL, NULL);
								val2_sql = (gchar *)gtk_date_entry_get_sql (GTK_DATE_ENTRY (priv->txt2));
							}
						else
							{
								val2 = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt2));
								val2_sql = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt2));
							}
						if (val2 == NULL)
							{
								val2 = g_strdup ("");
							}
						else
							{
								val2 = g_strdup (val2);
							}
						if (val2_sql == NULL)
							{
								val2_sql = g_strdup ("");
							}
						else
							{
								val2_sql = g_strdup (val2_sql);
							}

						gtk_tree_store_set (priv->tstore_where, &iter,
						                    COL_WHERE_LINK_TYPE, link_type,
						                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
						                    COL_WHERE_CONDITION_NOT, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->not)),
						                    COL_WHERE_CONDITION_TYPE, where_type,
						                    COL_WHERE_CONDITION_TYPE_VISIBLE, gdaex_query_editor_get_where_type_str_from_type (where_type),
						                    COL_WHERE_CONDITION_FROM, val1,
						                    COL_WHERE_CONDITION_FROM_SQL, val1_sql,
						                    COL_WHERE_CONDITION_TO, val2,
						                    COL_WHERE_CONDITION_TO_SQL, val2_sql,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_where);

						g_free (val1_sql);
						g_free (val2_sql);
					}
				break;

			case GDAEX_QE_PAGE_ORDER:
				if (gtk_tree_selection_get_selected (priv->sel_order, NULL, &iter))
					{
						if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->opt_asc)))
							{
								asc_desc = g_strdup ("ASC");
							}
						else
							{
								asc_desc = g_strdup ("DESC");
							}

						gtk_list_store_set (priv->lstore_order, &iter,
						                    COL_ORDER_ORDER, asc_desc,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_order);
					}
				break;
		}

	g_free (asc_desc);
	g_free (val1);
	g_free (val2);

	gtk_widget_hide (priv->vbx_values_container);
}

static void
gdaex_query_editor_show_add_iter (GdaExQueryEditor *qe, GtkTreeIter *iter)
{
	GdaExQueryEditorPrivate *priv;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), iter,
	                    COL_FIELDS_TABLE_NAME, &table_name,
	                    COL_FIELDS_NAME, &field_name,
	                    -1);

	table = g_hash_table_lookup (priv->tables, table_name);
	field = g_hash_table_lookup (table->fields, field_name);

	gtk_list_store_append (priv->lstore_show, iter);
	gtk_list_store_set (priv->lstore_show, iter,
	                    COL_SHOW_TABLE_NAME, field->table_name,
	                    COL_SHOW_NAME, field_name,
	                    COL_SHOW_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
	                    -1);

	g_free (table_name);
	g_free (field_name);
}

static void
gdaex_query_editor_on_btn_show_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gdaex_query_editor_show_add_iter (qe, &iter);
			gdaex_query_editor_on_sel_fields_changed (NULL, user_data);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 "You must select a field before.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_show_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show));
}

static void
gdaex_query_editor_on_btn_show_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_show_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_show_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_show);
	gdaex_query_editor_refill_always_show (qe);
}

static void
gdaex_query_editor_on_sel_show_changed (GtkTreeSelection *treeselection,
                                          gpointer user_data)
{
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *alias;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkWidget *lbl;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_show, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_SHOW;

			gtk_tree_selection_unselect_all (priv->sel_where);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
			                    COL_SHOW_TABLE_NAME, &table_name,
			                    COL_SHOW_NAME, &field_name,
			                    COL_SHOW_ALIAS, &alias,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button4")), !field->always_showed);

			priv->hbox = gtk_hbox_new (TRUE, 0);

			priv->tbl = gtk_table_new (2, 2, FALSE);
			gtk_table_set_row_spacings (GTK_TABLE (priv->tbl), 5);
			gtk_table_set_col_spacings (GTK_TABLE (priv->tbl), 5);
			gtk_box_pack_start (GTK_BOX (priv->hbox), priv->tbl, TRUE, TRUE, 0);

			lbl = gtk_label_new ("Alias");
			gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);

			lbl = gtk_label_new (g_strconcat (table->name_visible, " - ", field->name_visible, NULL));
			gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 0, 1, 1, 2, 0, 0, 0, 0);

			priv->txt1 = gtk_entry_new ();
			gtk_entry_set_text (GTK_ENTRY (priv->txt1), alias == NULL ? "" : alias);
			gtk_table_attach (GTK_TABLE (priv->tbl), priv->txt1, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0);

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);

			g_free (table_name);
			g_free (field_name);
			g_free (alias);
		}
}

static void
gdaex_query_editor_on_btn_where_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_tree_store_append (priv->tstore_where, &iter, NULL);
			gtk_tree_store_set (priv->tstore_where, &iter,
			                    COL_WHERE_TABLE_NAME, field->table_name,
			                    COL_WHERE_NAME, field_name,
			                    COL_WHERE_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
			                    -1);

			g_free (table_name);
			g_free (field_name);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 "You must select a field before.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_where_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where));
}

static void
gdaex_query_editor_on_btn_where_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_where_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_where_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_store_clear (priv->tstore_where);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static void
gdaex_query_editor_on_sel_where_changed (GtkTreeSelection *treeselection,
                                         gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gint *indices;

	guint link_type;
	gchar *table_name;
	gchar *field_name;
	gboolean not;
	guint where_type;
	gchar *from;
	gchar *to;
	gchar *from_sql;
	gchar *to_sql;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkWidget *lbl;
	GtkWidget *widget_val1;
	GtkWidget *widget_val2;
	GtkTreeIter iter_cb;
	GtkCellRenderer *renderer;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_WHERE;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
			                    COL_WHERE_LINK_TYPE, &link_type,
			                    COL_WHERE_TABLE_NAME, &table_name,
			                    COL_WHERE_NAME, &field_name,
			                    COL_WHERE_CONDITION_NOT, &not,
			                    COL_WHERE_CONDITION_TYPE, &where_type,
			                    COL_WHERE_CONDITION_FROM, &from,
			                    COL_WHERE_CONDITION_FROM_SQL, &from_sql,
			                    COL_WHERE_CONDITION_TO, &to,
			                    COL_WHERE_CONDITION_TO_SQL, &to_sql,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			priv->hbox = gtk_hbox_new (TRUE, 0);

			priv->tbl = gtk_table_new (3, 5, FALSE);
			gtk_table_set_row_spacings (GTK_TABLE (priv->tbl), 5);
			gtk_table_set_col_spacings (GTK_TABLE (priv->tbl), 5);
			gtk_box_pack_start (GTK_BOX (priv->hbox), priv->tbl, TRUE, TRUE, 0);

			/* if it is the first condition, "link" isn't visibile */
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter);
			indices = gtk_tree_path_get_indices (path);
			if (indices[0] != 0)
				{
					lbl = gtk_label_new ("Link");
					gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
				}

			lbl = gtk_label_new ("Not");
			gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 2, 3, 0, 1, GTK_FILL, 0, 0, 0);

			lbl = gtk_label_new ("Condition");
			gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 3, 4, 0, 1, GTK_FILL, 0, 0, 0);

			priv->lbl_txt1 = gtk_label_new ("Value");
			gtk_table_attach (GTK_TABLE (priv->tbl), priv->lbl_txt1, 4, 5, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);

			/* if it is the first condition, "link" isn't visibile */
			if (indices[0] != 0)
				{
					priv->cb_link_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (priv->lstore_link_type));

					renderer = gtk_cell_renderer_text_new ();
					gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->cb_link_type), renderer, TRUE);
					gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (priv->cb_link_type), renderer, "text", 1);

					gtk_list_store_clear (priv->lstore_link_type);

					gtk_list_store_append (priv->lstore_link_type, &iter_cb);
					gtk_list_store_set (priv->lstore_link_type, &iter_cb,
					                    0, GDAEX_QE_LINK_TYPE_AND,
					                    1, gdaex_query_editor_get_link_type_str_from_type (GDAEX_QE_LINK_TYPE_AND),
					                    -1);
					if (link_type == GDAEX_QE_LINK_TYPE_AND)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_link_type), &iter_cb);
						}
					gtk_list_store_append (priv->lstore_link_type, &iter_cb);
					gtk_list_store_set (priv->lstore_link_type, &iter_cb,
					                    0, GDAEX_QE_LINK_TYPE_OR,
					                    1, gdaex_query_editor_get_link_type_str_from_type (GDAEX_QE_LINK_TYPE_OR),
					                    -1);
					if (link_type == GDAEX_QE_LINK_TYPE_OR)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_link_type), &iter_cb);
						}
					gtk_table_attach (GTK_TABLE (priv->tbl), priv->cb_link_type, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
				}
			else
				{
					gtk_table_set_col_spacing (GTK_TABLE (priv->tbl), 0, 0);
				}

			lbl = gtk_label_new (g_strconcat (table->name_visible, " - ", field->name_visible, NULL));
			gtk_table_attach (GTK_TABLE (priv->tbl), lbl, 1, 2, 1, 2, 0, 0, 0, 0);

			priv->not = gtk_check_button_new ();
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->not), not);
			gtk_table_attach (GTK_TABLE (priv->tbl), priv->not, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);

			priv->cb_where_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (priv->lstore_where_type));

			renderer = gtk_cell_renderer_text_new ();
			gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->cb_where_type), renderer, TRUE);
			gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (priv->cb_where_type), renderer, "text", 1);

			gtk_list_store_clear (priv->lstore_where_type);
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_EQUAL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_EQUAL,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_EQUAL),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_EQUAL)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_STARTS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_STARTS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_STARTS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_STARTS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_CONTAINS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_CONTAINS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_CONTAINS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_CONTAINS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ENDS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_ENDS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ENDS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_ENDS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ISTARTS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_ISTARTS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ISTARTS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_ISTARTS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ICONTAINS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_ICONTAINS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ICONTAINS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_ICONTAINS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_IENDS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_IENDS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_IENDS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_IENDS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_GREAT,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_GREAT)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT_EQUAL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_GREAT_EQUAL,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT_EQUAL),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_GREAT_EQUAL)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_LESS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_LESS)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS_EQUAL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_LESS_EQUAL,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS_EQUAL),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_LESS_EQUAL)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_BETWEEN)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_BETWEEN,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_BETWEEN),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_IS_NULL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_IS_NULL,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_IS_NULL),
					                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
						}
				}
			gtk_table_attach (GTK_TABLE (priv->tbl), priv->cb_where_type, 3, 4, 1, 2, GTK_FILL, 0, 0, 0);

			priv->lbl_txt2 = gtk_label_new ("and");
			gtk_misc_set_alignment (GTK_MISC (priv->lbl_txt2), 1.0, 0.5);
			gtk_table_attach (GTK_TABLE (priv->tbl), priv->lbl_txt2, 3, 4, 2, 3, GTK_FILL, 0, 0, 0);

			switch (field->type)
				{
					/* TODO for now are all equals */
					case GDAEX_QE_FIELD_TYPE_TEXT:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from == NULL ? "" : from);
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt2), to == NULL ? "" : to);
						widget_val2 = priv->txt2;
						break;

					case GDAEX_QE_FIELD_TYPE_INTEGER:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from == NULL ? "" : from);
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt2), to == NULL ? "" : to);
						widget_val2 = priv->txt2;
						break;

					case GDAEX_QE_FIELD_TYPE_DOUBLE:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from == NULL ? "" : from);
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt2), to == NULL ? "" : to);
						widget_val2 = priv->txt2;
						break;

					case GDAEX_QE_FIELD_TYPE_DATE:
						priv->txt1 = gtk_date_entry_new (NULL, NULL, TRUE);
						gtk_date_entry_set_time_visible (GTK_DATE_ENTRY (priv->txt1), FALSE);
						gtk_date_entry_set_date_strf (GTK_DATE_ENTRY (priv->txt1), from_sql == NULL ? g_date_time_format (g_date_time_new_now_local (), "%Y-%m-%d") : from_sql, "Ymd");
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_date_entry_new (NULL, NULL, TRUE);
						gtk_date_entry_set_time_visible (GTK_DATE_ENTRY (priv->txt2), FALSE);
						gtk_date_entry_set_date_strf (GTK_DATE_ENTRY (priv->txt2), to_sql == NULL ? g_date_time_format (g_date_time_new_now_local (), "%Y-%m-%d") : to_sql, "Ymd");
						widget_val2 = priv->txt2;
						break;

					case GDAEX_QE_FIELD_TYPE_DATETIME:
						priv->txt1 = gtk_date_entry_new (NULL, NULL, TRUE);
						gtk_date_entry_set_time_separator (GTK_DATE_ENTRY (priv->txt1), ".");
						gtk_date_entry_set_date_strf (GTK_DATE_ENTRY (priv->txt1), from_sql == NULL ? g_date_time_format (g_date_time_new_now_local (), "%Y-%m-%d %H.%M.%S") : from_sql, "YmdHMS");
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_date_entry_new (NULL, NULL, TRUE);
						gtk_date_entry_set_time_separator (GTK_DATE_ENTRY (priv->txt2), ".");
						gtk_date_entry_set_date_strf (GTK_DATE_ENTRY (priv->txt2), to_sql == NULL ? g_date_time_format (g_date_time_new_now_local (), "%Y-%m-%d %H.%M.%S") : to_sql, "YmdHMS");
						widget_val2 = priv->txt2;
						break;

					case GDAEX_QE_FIELD_TYPE_TIME:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_max_length (GTK_ENTRY (priv->txt1), 8);
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from == NULL ? "" : from);
						widget_val1 = priv->txt1;

						priv->txt2 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt2), to == NULL ? "" : to);
						widget_val2 = priv->txt2;
						break;

					default:
						g_warning ("Field's type «%d» not valid.", field->type);
						break;
				};

			gtk_table_attach (GTK_TABLE (priv->tbl), widget_val1, 4, 5, 1, 2, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);
			gtk_table_attach (GTK_TABLE (priv->tbl), widget_val2, 4, 5, 2, 3, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);

			gtk_widget_set_visible (priv->lbl_txt1, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);
			gtk_widget_set_visible (priv->txt1, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);

			if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
				{
					gtk_table_set_col_spacing (GTK_TABLE (priv->tbl), 3, 0);
				}

			gtk_widget_set_visible (priv->lbl_txt2, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);
			gtk_widget_set_visible (priv->txt2, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);

			if (where_type == 0)
				{
					gtk_table_set_row_spacing (GTK_TABLE (priv->tbl), 1, 0);
				}

			g_signal_connect (G_OBJECT (priv->cb_where_type), "changed",
			                  G_CALLBACK (gdaex_query_editor_on_cb_where_type_changed), user_data);
		}
}

static void
gdaex_query_editor_on_btn_order_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_list_store_append (priv->lstore_order, &iter);
			gtk_list_store_set (priv->lstore_order, &iter,
			                    COL_ORDER_TABLE_NAME, field->table_name,
			                    COL_ORDER_NAME, field_name,
			                    COL_ORDER_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
			                    COL_ORDER_ORDER, "ASC",
			                    -1);

			gdaex_query_editor_on_sel_fields_changed (NULL, user_data);

			g_free (table_name);
			g_free (field_name);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 "You must select a field before.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_order_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order));
}

static void
gdaex_query_editor_on_btn_order_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_order_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_order_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_order);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static void
gdaex_query_editor_on_sel_order_changed (GtkTreeSelection *treeselection,
                                         gpointer user_data)
{
	GtkTreeIter iter;

	gchar *field_name;
	gchar *order;

	GtkWidget *lbl;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_order, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_ORDER;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_where);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
			                    COL_ORDER_VISIBLE_NAME, &field_name,
			                    COL_ORDER_ORDER, &order,
			                    -1);

			priv->hbox = gtk_hbox_new (TRUE, 5);

			lbl = gtk_label_new (field_name);
			gtk_box_pack_start (GTK_BOX (priv->hbox), lbl, FALSE, FALSE, 0);

			priv->opt_asc = gtk_radio_button_new_with_label (NULL, "Ascending");
			gtk_box_pack_start (GTK_BOX (priv->hbox), priv->opt_asc, FALSE, FALSE, 0);

			priv->opt_desc = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (priv->opt_asc), "Descending");
			gtk_box_pack_start (GTK_BOX (priv->hbox), priv->opt_desc, FALSE, FALSE, 0);

			if (g_strcmp0 (order, "ASC") == 0)
				{
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->opt_asc), TRUE);
				}
			else
				{
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->opt_desc), TRUE);
				}

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);

			g_free (field_name);
			g_free (order);
		}
}
