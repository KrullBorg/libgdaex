/*
 *  grid.c
 *
 *  Copyright (C) 2010-2013 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex.
 *  
 *  libgdaex_grid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaex_grid is distributed in the hope that it will be useful,
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

#include <gtk/gtk.h>

#ifdef REPTOOL_FOUND
	#include <gdk/gdkkeysyms.h>
	#include <libreptool/libreptool.h>
#endif

#include "grid.h"

static void gdaex_grid_class_init (GdaExGridClass *klass);
static void gdaex_grid_init (GdaExGrid *gdaex_grid);

static void gdaex_grid_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_grid_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

#ifdef REPTOOL_FOUND
static gboolean gdaex_grid_on_key_release_event (GtkWidget *widget,
                            GdkEventKey *event,
                            gpointer user_data);
#endif

static gboolean gdaex_grid_on_button_press_event (GtkWidget *widget,
                                                  GdkEventButton *event,
                                                  gpointer user_data);
static gboolean gdaex_grid_on_popup_menu (GtkWidget *widget,
                                          gpointer user_data);
static void gdaex_grid_on_menu_item_toggled (GtkCheckMenuItem *checkmenuitem,
                                             gpointer user_data);

static GtkTreeModel *gdaex_grid_get_model (GdaExGrid *grid);
static GtkTreeView *gdaex_grid_get_view (GdaExGrid *grid);

#define GDAEX_GRID_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDAEX_TYPE_GRID, GdaExGridPrivate))

typedef struct _GdaExGridPrivate GdaExGridPrivate;
struct _GdaExGridPrivate
	{
		GPtrArray *columns; /* GdaExGridColumn */

		gchar *title;

		GtkTreeModel *model;
		GtkWidget *view;
		GtkWidget *menu;
	};

G_DEFINE_TYPE (GdaExGrid, gdaex_grid, G_TYPE_OBJECT)

static void
gdaex_grid_class_init (GdaExGridClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExGridPrivate));

	object_class->set_property = gdaex_grid_set_property;
	object_class->get_property = gdaex_grid_get_property;
}

static void
gdaex_grid_init (GdaExGrid *gdaex_grid)
{
	GdaExGridPrivate *priv = GDAEX_GRID_GET_PRIVATE (gdaex_grid);

	priv->columns = g_ptr_array_new ();
	priv->title = NULL;
	priv->view = NULL;
	priv->menu = NULL;
}

GdaExGrid
*gdaex_grid_new ()
{
	GdaExGrid *gdaex_grid = GDAEX_GRID (g_object_new (gdaex_grid_get_type (), NULL));

	GdaExGridPrivate *priv = GDAEX_GRID_GET_PRIVATE (gdaex_grid);

	return gdaex_grid;
}

void
gdaex_grid_set_title (GdaExGrid *grid, const gchar *title)
{
	GdaExGridPrivate *priv;

	g_return_if_fail (GDAEX_IS_GRID (grid));

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	if (priv->title != NULL)
		{
			g_free (priv->title);
		}

	if (title != NULL)
		{
			priv->title = g_strdup (title);
		}
	else
		{
			priv->title = NULL;
		}
}

const gchar
*gdaex_grid_get_title (GdaExGrid *grid)
{
	GdaExGridPrivate *priv;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	if (priv->title != NULL)
		{
			return g_strdup (priv->title);
		}
	else
		{
			return NULL;
		}
}

void
gdaex_grid_add_column (GdaExGrid *grid, GdaExGridColumn *column)
{
	GdaExGridPrivate *priv;

	g_return_if_fail (GDAEX_IS_GRID (grid));
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	g_ptr_array_add (priv->columns, g_object_ref (column));
}

void
gdaex_grid_add_columns (GdaExGrid *grid, GSList *columns)
{
	GSList *first;

	g_return_if_fail (GDAEX_IS_GRID (grid));

	first = columns;
	while (first != NULL)
		{
			gdaex_grid_add_column (grid, (GdaExGridColumn *)first->data);
			first = g_slist_next (first);
		}
}

void
gdaex_grid_clear (GdaExGrid *grid)
{
	GdaExGridPrivate *priv;

	guint col;

	g_return_if_fail (GDAEX_IS_GRID (grid));

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	for (col = 0; priv->columns->len; col++)
		{
			g_object_unref (g_ptr_array_index (priv->columns, col));
		}

	g_ptr_array_remove_range (priv->columns, 0, priv->columns->len);
	g_ptr_array_unref (priv->columns);

	priv->columns = g_ptr_array_new ();
}

GtkWidget
*gdaex_grid_get_widget (GdaExGrid *grid)
{
	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	return GTK_WIDGET (gdaex_grid_get_view (grid));
}

gboolean
gdaex_grid_fill_from_sql (GdaExGrid *grid, GdaEx *gdaex, const gchar *sql, GError **error)
{
	GdaDataModel *dm;

	gchar *_sql;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), FALSE);
	g_return_val_if_fail (IS_GDAEX (gdaex), FALSE);
	g_return_val_if_fail (sql != NULL, FALSE);

	_sql = g_strstrip (g_strdup (sql));

	g_return_val_if_fail (g_strcmp0 (_sql, "") != 0, FALSE);

	dm = gdaex_query (gdaex, _sql);
	return gdaex_grid_fill_from_datamodel (grid, dm, error);
}

gboolean
gdaex_grid_fill_from_datamodel (GdaExGrid *grid, GdaDataModel *dm, GError **error)
{
	GdaExGridPrivate *priv;

	GdaDataModelIter *dm_iter;
	GtkTreeIter iter;

	guint cols;
	guint col;

	GType col_gtype;

	GdaColumn *gcol;
	GType gcol_gtype;

	gint *columns;
	GValue *values;

	GDateTime *gdatetime;
	gdouble dval;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), FALSE);
	g_return_val_if_fail (GDA_IS_DATA_MODEL (dm), FALSE);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	if (priv->model == NULL)
		{
			gdaex_grid_get_model (grid);
		}
	g_return_val_if_fail (GTK_IS_TREE_MODEL (priv->model), FALSE);

	dm_iter = gda_data_model_create_iter (dm);
	g_return_val_if_fail (GDA_IS_DATA_MODEL_ITER (dm_iter), FALSE);

	gtk_list_store_clear (GTK_LIST_STORE (priv->model));

	cols = gtk_tree_model_get_n_columns (priv->model);

	columns = g_malloc0 (cols * sizeof (gint));
	values = g_malloc0 (cols * sizeof (GValue));

	while (gda_data_model_iter_move_next (dm_iter))
		{
			gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);

			for (col = 0; col < cols; col++)
				{
					columns[col] = col;

					col_gtype = gtk_tree_model_get_column_type (priv->model, col);

					GValue gval = {0};
					g_value_init (&gval, col_gtype);
					switch (col_gtype)
						{
							case G_TYPE_STRING:
								gcol = gda_data_model_describe_column (dm, col);
								gcol_gtype = gda_column_get_g_type (gcol);

								switch (gcol_gtype)
									{
										case G_TYPE_STRING:
											g_value_set_string (&gval, gdaex_data_model_iter_get_value_stringify_at (dm_iter, col));
											break;

										case G_TYPE_BOOLEAN:
											g_value_set_string (&gval, gdaex_data_model_iter_get_value_boolean_at (dm_iter, col) ? "X" : "");
											break;

										case G_TYPE_INT:
										case G_TYPE_FLOAT:
										case G_TYPE_DOUBLE:
											dval = gdaex_data_model_iter_get_value_double_at (dm_iter, col);
											g_value_set_string (&gval, gdaex_format_money (dval, gdaex_grid_column_get_decimals ((GdaExGridColumn *)g_ptr_array_index (priv->columns, col)), FALSE));
											break;

										default:
											if (gcol_gtype == G_TYPE_DATE
											    || gcol_gtype == G_TYPE_DATE_TIME
											    || gcol_gtype == GDA_TYPE_TIMESTAMP)
												{
													gdatetime = gdaex_data_model_iter_get_value_gdatetime_at (dm_iter, col);
													/* TODO find default format from locale */
													g_value_set_string (&gval, g_date_time_format (gdatetime, gcol_gtype == G_TYPE_DATE ? "%d/%m/%Y" : "%d/%m/%Y %H.%M.%S"));
												}
											else
												{
													g_value_set_string (&gval, gda_value_stringify (gda_data_model_iter_get_value_at (dm_iter, col)));
												}
											break;
									}

								values[col] = gval;
								break;

							case G_TYPE_INT:
								g_value_set_int (&gval, gdaex_data_model_iter_get_value_integer_at (dm_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_FLOAT:
								g_value_set_float (&gval, gdaex_data_model_iter_get_value_float_at (dm_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_DOUBLE:
								g_value_set_double (&gval, gdaex_data_model_iter_get_value_double_at (dm_iter, col));
								values[col] = gval;
								break;

							case G_TYPE_BOOLEAN:
								g_value_set_boolean (&gval, gdaex_data_model_iter_get_value_boolean_at (dm_iter, col));
								values[col] = gval;
								break;

							default:
								values[col] = *gda_value_new_from_string (gdaex_data_model_iter_get_value_stringify_at (dm_iter, col), col_gtype);
								break;
						}
				}

			gtk_list_store_set_valuesv (GTK_LIST_STORE (priv->model), &iter, columns, values, cols);
		}

	return TRUE;
}

/* PRIVATE */
static void
gdaex_grid_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExGrid *gdaex_grid = GDAEX_GRID (object);
	GdaExGridPrivate *priv = GDAEX_GRID_GET_PRIVATE (gdaex_grid);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_grid_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExGrid *gdaex_grid = GDAEX_GRID (object);
	GdaExGridPrivate *priv = GDAEX_GRID_GET_PRIVATE (gdaex_grid);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static GtkTreeModel
*gdaex_grid_get_model (GdaExGrid *grid)
{
	/* TODO for now it returns always a GtkListStore */
	GdaExGridPrivate *priv;

	guint col;
	GType *gtype;
	GType col_gtype;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	gtype = g_new0 (GType, priv->columns->len);

	for (col = 0; col < priv->columns->len; col++)
		{
			col_gtype = gdaex_grid_column_get_gtype ((GdaExGridColumn *)g_ptr_array_index (priv->columns, col));
			if (col_gtype == G_TYPE_DATE
			    || col_gtype == G_TYPE_DATE_TIME
			    || col_gtype == GDA_TYPE_TIMESTAMP
			    || ((col_gtype == G_TYPE_INT
			         || col_gtype == G_TYPE_FLOAT
			         || col_gtype == G_TYPE_DOUBLE)
			        && gdaex_grid_column_get_decimals ((GdaExGridColumn *)g_ptr_array_index (priv->columns, col)) > -1))
				{
					gtype[col] = G_TYPE_STRING;
				}
			else
				{
					gtype[col] = col_gtype;
				}
		}

	if (priv->model != NULL)
		{
			g_object_unref (priv->model);
		}
	priv->model = GTK_TREE_MODEL (gtk_list_store_newv (priv->columns->len, gtype));

	return priv->model;
}

static GtkTreeView
*gdaex_grid_get_view (GdaExGrid *grid)
{
	GtkWidget *view;

	GdaExGridPrivate *priv;

	GtkTreeModel *model;
	GdaExGridColumn *gcolumn;
	GtkTreeViewColumn *vcolumn;

	GList *cells;

	guint col;

	GtkWidget *mitem;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	model = gdaex_grid_get_model (grid);
	view = gtk_tree_view_new_with_model (model);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);

	if (priv->menu != NULL)
		{
			g_free (priv->menu);
		}
	if (priv->columns->len > 0)
		{
			priv->menu = gtk_menu_new ();
		}
	else
		{
			priv->menu = NULL;
		}

	for (col = 0; col < priv->columns->len; col++)
		{
			gcolumn = (GdaExGridColumn *)g_ptr_array_index (priv->columns, col);
			vcolumn = gdaex_grid_column_get_column (gcolumn);
			if (vcolumn)
				{
					cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (vcolumn));
					cells = g_list_first (cells);

					if (gdaex_grid_column_get_gtype (gcolumn) == G_TYPE_BOOLEAN)
						{
							gtk_tree_view_column_add_attribute (vcolumn, (GtkCellRenderer *)cells->data, "active", col);
						}
					else
						{
							gtk_tree_view_column_add_attribute (vcolumn, (GtkCellRenderer *)cells->data, "text", col);
						}

					if (gdaex_grid_column_get_reorderable (gcolumn))
						{
							gtk_tree_view_column_set_sort_column_id (vcolumn, col);
						}

					gtk_tree_view_append_column (GTK_TREE_VIEW (view), vcolumn);

					/* popup menu */
					mitem = gtk_check_menu_item_new_with_label (gdaex_grid_column_get_title (gcolumn));
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mitem), gdaex_grid_column_get_visible (gcolumn));

					g_object_set_data (G_OBJECT (mitem), "GdaExGridColumn", gcolumn);

					g_signal_connect (G_OBJECT (mitem), "toggled",
					                  G_CALLBACK (gdaex_grid_on_menu_item_toggled), (gpointer)grid);

					gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), mitem);
					gtk_widget_show (mitem);
				}
		}

	priv->view = view;
	if (priv->menu != NULL)
		{
			gtk_menu_attach_to_widget (GTK_MENU (priv->menu), priv->view, NULL);

			g_signal_connect (G_OBJECT (priv->view), "button-press-event",
			                  G_CALLBACK (gdaex_grid_on_button_press_event), (gpointer)grid);
			g_signal_connect (G_OBJECT (priv->view), "popup-menu",
			                  G_CALLBACK (gdaex_grid_on_popup_menu), (gpointer)grid);
		}

#ifdef REPTOOL_FOUND
	g_signal_connect (view,
	                  "key-release-event", G_CALLBACK (gdaex_grid_on_key_release_event), (gpointer)grid);
#endif

	return GTK_TREE_VIEW (priv->view);
}

#ifdef REPTOOL_FOUND
static gboolean
gdaex_grid_on_key_release_event (GtkWidget *widget,
                            GdkEventKey *event,
                            gpointer user_data)
{
	GdaExGridPrivate *priv;

	RptReport *rptr;
	RptPrint *rptp;

	gchar *_title;

	switch (event->keyval)
		{
			case GDK_F12:
				{
					if (event->state & GDK_CONTROL_MASK)
						{
							priv = GDAEX_GRID_GET_PRIVATE (user_data);

							if (priv->title != NULL)
								{
									_title = g_strdup_printf ("\"%s\"", priv->title);
								}
							else
								{
									_title = NULL;
								}
							rptr = rpt_report_new_from_gtktreeview (GTK_TREE_VIEW (priv->view), _title);

							if (rptr != NULL)
								{
									xmlDoc *report = rpt_report_get_xml (rptr);
									rpt_report_set_output_type (rptr, RPT_OUTPUT_GTK);

									xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);

									rptp = rpt_print_new_from_xml (rptprint);
									if (rptp != NULL)
										{
											rpt_print_set_output_type (rptp, RPT_OUTPUT_GTK);
											rpt_print_print (rptp, GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (priv->view))));
										}
								}

							if (_title != NULL)
								{
									g_free (_title);
								}
							return TRUE;
						}
					break;
				}
		}

	return FALSE;
}
#endif

static gboolean
gdaex_grid_on_button_press_event (GtkWidget *widget,
                                  GdkEventButton *event,
                                  gpointer user_data)
{
	GdaExGridPrivate *priv;

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
		{
			priv = GDAEX_GRID_GET_PRIVATE (user_data);

			gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL, NULL, NULL,
			                event->button, event->time);
			return TRUE;
		}

	return FALSE;
}

static gboolean
gdaex_grid_on_popup_menu (GtkWidget *widget,
                          gpointer user_data)
{
	GdaExGridPrivate *priv;

	priv = GDAEX_GRID_GET_PRIVATE (user_data);
	gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL, NULL, NULL,
	                0, gtk_get_current_event_time ());

	return TRUE;
}

static void
gdaex_grid_on_menu_item_toggled (GtkCheckMenuItem *checkmenuitem,
                                 gpointer user_data)
{
	GdaExGridPrivate *priv;

	priv = GDAEX_GRID_GET_PRIVATE (user_data);

	GdaExGridColumn *gcolumn = g_object_get_data (G_OBJECT (checkmenuitem), "GdaExGridColumn");

	gdaex_grid_column_set_visible (gcolumn,
	                               gtk_check_menu_item_get_active (checkmenuitem));
}
