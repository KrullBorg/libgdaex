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
		gchar *table1;
		gchar *table2;

		GSList *fields1;
		GSList *fields2;
	} GdaExQueryEditorRelation;

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

static void gdaex_query_editor_on_btn_cancel_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data);

static void gdaex_query_editor_on_sel_fields_changed (GtkTreeSelection *treeselection,
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
		gboolean where_value;
		GtkListStore *lstore_where_type;
		GtkWidget *hbox;
		GtkWidget *not;
		GtkWidget *cb_where_type;
		GtkWidget *txt1;
		GtkWidget *txt2;
		GtkWidget *opt_asc;
		GtkWidget *opt_desc;
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
		COL_SHOW_VISIBLE_NAME
	};

enum
	{
		COL_WHERE_TABLE_NAME,
		COL_WHERE_NAME,
		COL_WHERE_VISIBLE_NAME,
		COL_WHERE_CONDITION_NOT,
		COL_WHERE_CONDITION_TYPE,
		COL_WHERE_CONDITION_TYPE_VISIBLE,
		COL_WHERE_CONDITION_FROM,
		COL_WHERE_CONDITION_TO
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
                                 ...)
{
	GdaExQueryEditorPrivate *priv;

	GdaExQueryEditorRelation *relation;

	va_list fields;
	gchar *field_name1;
	gchar *field_name2;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	relation = g_new0 (GdaExQueryEditorRelation, 1);

	relation->table1 = g_strdup (table1);
	/* TODO if table1 doesn't exists, create with visible = FALSE */

	relation->table2 = g_strdup (table2);
	/* TODO if table2 doesn't exists, create with visible = FALSE */

	va_start (fields, table2);

	while ((field_name1 = va_arg (fields, gchar *)) != NULL
	       && (field_name2 = va_arg (fields, gchar *)) != NULL)
		{
			if (field_name2 != NULL)
				{
					relation->fields1 = g_slist_append (relation->fields1, g_strdup (field_name1));
					relation->fields2 = g_slist_append (relation->fields1, g_strdup (field_name2));
				}
			else
				{
					break;
				}
		}

	va_end (fields);

	priv->relations = g_slist_append (priv->relations, relation);

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

const gchar
*gdaex_query_editor_get_sql (GdaExQueryEditor *qe)
{
	const gchar *ret;

	GdaExQueryEditorPrivate *priv;

	GdaSqlBuilder *sqlbuilder;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *asc_desc;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GdaStatement *stmt;
	GError *error;

	ret = NULL;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	sqlbuilder = gda_sql_builder_new (GDA_SQL_STATEMENT_SELECT);

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
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

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
							                                     GDA_SQL_SELECT_JOIN_INNER, join_cond);
							gda_sql_builder_select_add_field (sqlbuilder, field->decode_field_to_show, field->decode_table2, field->decode_field_alias);
						}
					else
						{
							gda_sql_builder_select_add_field (sqlbuilder, field->name, table->name, field->alias);
							gda_sql_builder_select_add_target_id (sqlbuilder,
							                                      gda_sql_builder_add_id (sqlbuilder, table->name),
							                                      NULL);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			gboolean not;
			guint where_type;
			GdaSqlOperatorType op;
			GType type;
			gchar *from_str;
			gchar *to_str;
			GDate *from_date;
			GDateTime *from_datetime;

			guint id_field;
			guint id_value1;
			guint id_value2;
			guint id_cond;
			guint id_cond_iter;

			id_cond = 0;
			do
				{
					id_value2 = 0;

					gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
					                    COL_WHERE_TABLE_NAME, &table_name,
					                    COL_WHERE_NAME, &field_name,
					                    COL_WHERE_CONDITION_NOT, &not,
					                    COL_WHERE_CONDITION_TYPE, &where_type,
					                    COL_WHERE_CONDITION_FROM, &from_str,
					                    COL_WHERE_CONDITION_TO, &to_str,
					                    -1);

					if (to_str == NULL || g_strcmp0 (g_strstrip (to_str), "") == 0)
						{
							to_str = NULL;
						}

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					id_field = gda_sql_builder_add_id (sqlbuilder, g_strconcat (table->name, ".", field->name, NULL));

					switch (field->type)
						{
							case GDAEX_QE_FIELD_TYPE_TEXT:
								type = G_TYPE_STRING;
								id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, from_str);
								if (to_str != NULL)
									{
										id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, to_str);
									}
								break;

							case GDAEX_QE_FIELD_TYPE_INTEGER:
								type = G_TYPE_INT;
								id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, strtol (from_str, NULL, 10));
								if (to_str != NULL)
									{
										id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, strtol (to_str, NULL, 10));
									}
								break;

							case GDAEX_QE_FIELD_TYPE_DOUBLE:
								type = G_TYPE_DOUBLE;
								id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, g_strtod (from_str, NULL));
								if (to_str != NULL)
									{
										id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, type, g_strtod (to_str, NULL));
									}
								break;

							case GDAEX_QE_FIELD_TYPE_DATE:
								type = G_TYPE_DATE;
								/* TODO */
								break;

							case GDAEX_QE_FIELD_TYPE_DATETIME:
								type = G_TYPE_DATE_TIME;
								/* TODO */
								break;
						};

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								op = GDA_SQL_OPERATOR_TYPE_EQ;
								break;

							case GDAEX_QE_WHERE_TYPE_LIKE:
								/* TODO */
								op = GDA_SQL_OPERATOR_TYPE_LIKE;
								break;

							case GDAEX_QE_WHERE_TYPE_ILIKE:
								/* TODO */
								op = GDA_SQL_OPERATOR_TYPE_LIKE;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT:
								op = GDA_SQL_OPERATOR_TYPE_GT;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
								op = GDA_SQL_OPERATOR_TYPE_GEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS:
								op = GDA_SQL_OPERATOR_TYPE_LT;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
								op = GDA_SQL_OPERATOR_TYPE_LEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_BETWEEN:
								op = GDA_SQL_OPERATOR_TYPE_BETWEEN;
								break;
						}

					id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, op, id_field, id_value1, id_value2);
					if (not)
						{
							id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_NOT, id_cond_iter, 0, 0);
						}
					if (id_cond == 0)
						{
							id_cond = id_cond_iter;
						}
					else
						{
							id_cond = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_AND, id_cond, id_cond_iter, 0);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_where), &iter));

			if (id_cond != 0)
				{
					gda_sql_builder_set_where (sqlbuilder, id_cond);
				}
		}

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

					gda_sql_builder_select_order_by (sqlbuilder,
					                                 gda_sql_builder_add_id (sqlbuilder, field->name),
					                                 (g_strcmp0 (asc_desc, "ASC") == 0),
					                                 NULL);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_order), &iter));
		}

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

xmlNode
*gdaex_query_editor_get_sql_as_xml (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	xmlNode *ret;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;

	xmlNode *node;

	ret = NULL;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	ret = xmlNewNode (NULL, "gdaex_query_editor_choices");

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_show), &iter))
		{
			xmlNode *node_show;

			node_show = xmlNewNode (NULL, "show");
			xmlAddChild (ret, node_show);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
					                    COL_SHOW_TABLE_NAME, &table_name,
					                    COL_SHOW_NAME, &field_name,
					                    -1);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_show, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			xmlNode *node_where;

			gboolean not;
			guint where_type;
			gchar *from_str;
			gchar *to_str;
			gchar *str_op;

			node_where = xmlNewNode (NULL, "where");
			xmlAddChild (ret, node_where);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
					                    COL_WHERE_TABLE_NAME, &table_name,
					                    COL_WHERE_NAME, &field_name,
					                    COL_WHERE_CONDITION_NOT, &not,
					                    COL_WHERE_CONDITION_TYPE, &where_type,
					                    COL_WHERE_CONDITION_FROM, &from_str,
					                    COL_WHERE_CONDITION_TO, &to_str,
					                    -1);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_where, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					xmlNewProp (node, "not", (not ? "y" : "n"));

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								str_op = g_strdup ("EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_LIKE:
								str_op = g_strdup ("LIKE");
								break;

							case GDAEX_QE_WHERE_TYPE_ILIKE:
								str_op = g_strdup ("ILIKE");
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

							default:
								g_warning ("Where type «%d» not valid.", where_type);
								continue;
						}
					xmlNewProp (node, "where_type", str_op);

					xmlNewProp (node, "from", from_str);
					xmlNewProp (node, "to", to_str);
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
                              const gchar *table_name_visibile,
                              gboolean is_visible)
{
	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_new0 (GdaExQueryEditorTable, 1);
	table->name = g_strstrip (g_strdup (table_name));
	table->name_visible = g_strstrip (g_strdup (table_name_visibile));
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
				}

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
			                    COL_FIELDS_TABLE_NAME, table->name,
			                    COL_FIELDS_NAME, field->name,
			                    COL_FIELDS_VISIBLE_NAME, field->name_visible,
			                    COL_FIELDS_DESCRIPTION, field->description,
			                    -1);

			if (field->always_showed)
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

	GtkWidget *dialog;

	g_return_if_fail (GTK_IS_LIST_STORE (store) || GTK_IS_TREE_STORE (store));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (sel, NULL, &iter))
		{
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
			if (path != NULL)
				{
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

			case GDAEX_QE_WHERE_TYPE_LIKE:
				ret = g_strdup ("Like");
				break;

			case GDAEX_QE_WHERE_TYPE_ILIKE:
				ret = g_strdup ("Non case sensitive like");
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT:
				ret = g_strdup ("Greater");
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
				ret = g_strdup ("Greater equal");
				break;

			case GDAEX_QE_WHERE_TYPE_LESS:
				ret = g_strdup ("Lesser");
				break;

			case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
				ret = g_strdup ("Lesser equal");
				break;

			case GDAEX_QE_WHERE_TYPE_BETWEEN:
				ret = g_strdup ("Between");
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

			if (!field->always_showed && field->for_show)
				{
					gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), TRUE);
				}

			if (field->for_where)
				{
					gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")),  TRUE);
				}

			table_field = g_strconcat (table->name_visible, " - ", field->name_visible, NULL);
			v_table_field = gda_value_new (G_TYPE_STRING);
			g_value_set_string (v_table_field, table_field);
			if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_order),
			                                        COL_ORDER_VISIBLE_NAME,
			                                        v_table_field))
				{
					gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), TRUE);
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
}

static void
gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	gchar *asc_desc;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (priv->where_value)
		{
			if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
				{
					GtkTreeIter iter_val;
					guint where_type;
					
					GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_where_type));
					if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_val))
						{
							GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
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

					gtk_tree_store_set (priv->tstore_where, &iter,
					                    COL_WHERE_CONDITION_NOT, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->not)),
					                    COL_WHERE_CONDITION_TYPE, where_type,
					                    COL_WHERE_CONDITION_TYPE_VISIBLE, gdaex_query_editor_get_where_type_str_from_type (where_type),
					                    COL_WHERE_CONDITION_FROM, gtk_entry_get_text (GTK_ENTRY (priv->txt1)),
					                    COL_WHERE_CONDITION_TO, priv->txt2 != NULL ? gtk_entry_get_text (GTK_ENTRY (priv->txt2)) : "",
					                    -1);
				}
		}
	else
		{
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
				}
		}

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
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_show, NULL, &iter))
		{
			gdaex_query_editor_remove_child_from_vbx_values (qe);
			gtk_widget_hide (priv->vbx_values_container);

			gtk_tree_selection_unselect_all (priv->sel_where);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
			                    COL_SHOW_TABLE_NAME, &table_name,
			                    COL_SHOW_NAME, &field_name,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button4")), !field->always_showed);
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

	gchar *table_name;
	gchar *field_name;
	gboolean not;
	guint where_type;
	gchar *from;
	gchar *to;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkWidget *tbl;
	GtkWidget *lbl;
	GtkTreeIter iter_cb;
	GtkCellRenderer *renderer;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
		{
			priv->where_value = TRUE;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
			                    COL_WHERE_TABLE_NAME, &table_name,
			                    COL_WHERE_NAME, &field_name,
			                    COL_WHERE_CONDITION_NOT, &not,
			                    COL_WHERE_CONDITION_TYPE, &where_type,
			                    COL_WHERE_CONDITION_FROM, &from,
			                    COL_WHERE_CONDITION_TO, &to,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gdaex_query_editor_remove_child_from_vbx_values (qe);

			priv->hbox = gtk_hbox_new (TRUE, 0);

			tbl = gtk_table_new (2, 6, FALSE);
			gtk_table_set_row_spacings (GTK_TABLE (tbl), 5);
			gtk_table_set_col_spacings (GTK_TABLE (tbl), 5);
			gtk_box_pack_start (GTK_BOX (priv->hbox), tbl, TRUE, TRUE, 0);

			lbl = gtk_label_new ("Not");
			gtk_table_attach (GTK_TABLE (tbl), lbl, 1, 2, 0, 1, 0, 0, 0, 0);

			lbl = gtk_label_new ("Condition");
			gtk_table_attach (GTK_TABLE (tbl), lbl, 2, 3, 0, 1, 0, 0, 0, 0);

			lbl = gtk_label_new (g_strconcat (table->name_visible, " - ", field->name_visible, NULL));
			gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2, 0, 0, 0, 0);

			priv->not = gtk_check_button_new ();
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->not), not);
			gtk_table_attach (GTK_TABLE (tbl), priv->not, 1, 2, 1, 2, 0, 0, 0, 0);

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
					if (where_type == GDAEX_QE_WHERE_TYPE_EQUAL) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LIKE)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_LIKE,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LIKE),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_LIKE) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ILIKE)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_ILIKE,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ILIKE),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_ILIKE) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_GREAT,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_GREAT) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT_EQUAL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_GREAT_EQUAL,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT_EQUAL),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_GREAT_EQUAL) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_LESS,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_LESS) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS_EQUAL)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_LESS_EQUAL,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS_EQUAL),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_LESS_EQUAL) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			if (field->available_where_type & GDAEX_QE_WHERE_TYPE_BETWEEN)
				{
					gtk_list_store_append (priv->lstore_where_type, &iter_cb);
					gtk_list_store_set (priv->lstore_where_type, &iter_cb,
			                    0, GDAEX_QE_WHERE_TYPE_BETWEEN,
			                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_BETWEEN),
			                    -1);
					if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN) gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
				}
			gtk_table_attach (GTK_TABLE (tbl), priv->cb_where_type, 2, 3, 1, 2, 0, 0, 0, 0);

			priv->txt2 = NULL;
			switch (field->type)
				{
					/* TODO for now are all equals */
					case GDAEX_QE_FIELD_TYPE_TEXT:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from);
						gtk_table_attach (GTK_TABLE (tbl), priv->txt1, 3, 4, 1, 2, 0, 0, 0, 0);
						if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
							{
								lbl = gtk_label_new ("and");
								gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 1, 2, 0, 0, 0, 0);
								priv->txt2 = gtk_entry_new ();
								gtk_entry_set_text (GTK_ENTRY (priv->txt1), to);
								gtk_table_attach (GTK_TABLE (tbl), priv->txt2, 5, 6, 1, 2, 0, 0, 0, 0);
							}
						break;
					case GDAEX_QE_FIELD_TYPE_INTEGER:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from);
						gtk_table_attach (GTK_TABLE (tbl), priv->txt1, 3, 4, 1, 2, 0, 0, 0, 0);
						if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
							{
								lbl = gtk_label_new ("and");
								gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 1, 2, 0, 0, 0, 0);
								priv->txt2 = gtk_entry_new ();
								gtk_entry_set_text (GTK_ENTRY (priv->txt1), to);
								gtk_table_attach (GTK_TABLE (tbl), priv->txt2, 5, 6, 1, 2, 0, 0, 0, 0);
							}
						break;
					case GDAEX_QE_FIELD_TYPE_DOUBLE:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from);
						gtk_table_attach (GTK_TABLE (tbl), priv->txt1, 3, 4, 1, 2, 0, 0, 0, 0);
						if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
							{
								lbl = gtk_label_new ("and");
								gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 1, 2, 0, 0, 0, 0);
								priv->txt2 = gtk_entry_new ();
								gtk_entry_set_text (GTK_ENTRY (priv->txt1), to);
								gtk_table_attach (GTK_TABLE (tbl), priv->txt2, 5, 6, 1, 2, 0, 0, 0, 0);
							}
						break;
					case GDAEX_QE_FIELD_TYPE_DATE:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_max_length (GTK_ENTRY (priv->txt1), 10);
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from);
						gtk_table_attach (GTK_TABLE (tbl), priv->txt1, 3, 4, 1, 2, 0, 0, 0, 0);
						if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
							{
								lbl = gtk_label_new ("and");
								gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 1, 2, 0, 0, 0, 0);
								priv->txt2 = gtk_entry_new ();
								gtk_entry_set_max_length (GTK_ENTRY (priv->txt2), 10);
								gtk_entry_set_text (GTK_ENTRY (priv->txt1), to);
								gtk_table_attach (GTK_TABLE (tbl), priv->txt2, 5, 6, 1, 2, 0, 0, 0, 0);
							}
						break;
					case GDAEX_QE_FIELD_TYPE_DATETIME:
						priv->txt1 = gtk_entry_new ();
						gtk_entry_set_max_length (GTK_ENTRY (priv->txt1), 19);
						gtk_entry_set_text (GTK_ENTRY (priv->txt1), from);
						gtk_table_attach (GTK_TABLE (tbl), priv->txt1, 3, 4, 1, 2, 0, 0, 0, 0);
						if (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN)
							{
								lbl = gtk_label_new ("and");
								gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 1, 2, 0, 0, 0, 0);
								priv->txt2 = gtk_entry_new ();
								gtk_entry_set_max_length (GTK_ENTRY (priv->txt2), 19);
								gtk_entry_set_text (GTK_ENTRY (priv->txt1), to);
								gtk_table_attach (GTK_TABLE (tbl), priv->txt2, 5, 6, 1, 2, 0, 0, 0, 0);
							}
						break;
					default:
						g_warning ("Field's type «%d» not valid.", field->type);
						break;
				};

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);
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

	if (gtk_tree_selection_get_selected (priv->sel_order, NULL, &iter))
		{
			priv->where_value = FALSE;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_where);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
			                    COL_ORDER_VISIBLE_NAME, &field_name,
			                    COL_ORDER_ORDER, &order,
			                    -1);

			gdaex_query_editor_remove_child_from_vbx_values (qe);

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
		}
}
