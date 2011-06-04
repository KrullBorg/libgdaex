/*
 *  grid.c
 *
 *  Copyright (C) 2010-2011 Andrea Zagli <azagli@libero.it>
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

static GtkTreeModel *gdaex_grid_get_model (GdaExGrid *grid);
static GtkTreeView *gdaex_grid_get_view (GdaExGrid *grid);

#define GDAEX_GRID_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDAEX_TYPE_GRID, GdaExGridPrivate))

typedef struct _GdaExGridPrivate GdaExGridPrivate;
struct _GdaExGridPrivate
	{
		GPtrArray *columns; /* GdaExGridColumn */
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
}

GdaExGrid
*gdaex_grid_new ()
{
	GdaExGrid *gdaex_grid = GDAEX_GRID (g_object_new (gdaex_grid_get_type (), NULL));

	GdaExGridPrivate *priv = GDAEX_GRID_GET_PRIVATE (gdaex_grid);

	return gdaex_grid;
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

	GtkListStore *store;

	GdaExGridPrivate *priv;

	guint col;
	GType *gtype;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	gtype = g_new0 (GType, priv->columns->len);

	for (col = 0; col < priv->columns->len; col++)
		{
			gtype[col] = gdaex_grid_column_get_gtype ((GdaExGridColumn *)g_ptr_array_index (priv->columns, col));
		}

	store = gtk_list_store_newv (priv->columns->len, gtype);

	return GTK_TREE_MODEL (store);
}

static GtkTreeView
*gdaex_grid_get_view (GdaExGrid *grid)
{
	GtkWidget *view;

	GdaExGridPrivate *priv;

	GtkTreeModel *model;
	GdaExGridColumn *gcolumn;

	guint col;

	g_return_val_if_fail (GDAEX_IS_GRID (grid), NULL);

	priv = GDAEX_GRID_GET_PRIVATE (grid);

	model = gdaex_grid_get_model (grid);
	view = gtk_tree_view_new_with_model (model);

	for (col = 0; col < priv->columns->len; col++)
		{
			gcolumn = (GdaExGridColumn *)g_ptr_array_index (priv->columns, col);
			if (gdaex_grid_column_get_visible (gcolumn))
				{
					gtk_tree_view_append_column (GTK_TREE_VIEW (view), gdaex_grid_column_get_column (gcolumn, col));
				}
		}

	return GTK_TREE_VIEW (view);
}
