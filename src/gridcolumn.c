/*
 *  gridcolumn.c
 *
 *  Copyright (C) 2010-2011 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex_grid_column.
 *  
 *  libgdaex_grid_column is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaex_grid_column is distributed in the hope that it will be useful,
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

#include <libgda/libgda.h>

#include "gridcolumn.h"

static void gdaex_grid_column_class_init (GdaExGridColumnClass *klass);
static void gdaex_grid_column_init (GdaExGridColumn *gdaex_grid_column);

static void gdaex_grid_column_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_grid_column_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

#define GDAEX_GRID_COLUMN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDAEX_TYPE_GRID_COLUMN, GdaExGridColumnPrivate))

typedef struct _GdaExGridColumnPrivate GdaExGridColumnPrivate;
struct _GdaExGridColumnPrivate
	{
		gchar *title;
		gchar *field_name;
		GType type;
		gboolean visible;
		gboolean resizable;
		gboolean sortable;
		gboolean reorderable;
		guint decimals;

		GtkTreeViewColumn *vcolumn;
	};

G_DEFINE_TYPE (GdaExGridColumn, gdaex_grid_column, G_TYPE_OBJECT)

static void
gdaex_grid_column_class_init (GdaExGridColumnClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExGridColumnPrivate));

	object_class->set_property = gdaex_grid_column_set_property;
	object_class->get_property = gdaex_grid_column_get_property;
}

static void
gdaex_grid_column_init (GdaExGridColumn *gdaex_grid_column)
{
	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	priv->title = NULL;
	priv->field_name = NULL;
	priv->type = G_TYPE_NONE;
	priv->visible = FALSE;
	priv->resizable = FALSE;
	priv->sortable = FALSE;
	priv->reorderable = FALSE;
	priv->decimals = -1;
}

GdaExGridColumn
*gdaex_grid_column_new (const gchar *title,
                        const gchar *field_name,
                        GType type,
                        gboolean visible,
                        gboolean resizable,
                        gboolean sortable,
                        gboolean reorderable,
                        guint decimals)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (g_object_new (gdaex_grid_column_get_type (), NULL));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	gdaex_grid_column_set_title (gdaex_grid_column, title);
	gdaex_grid_column_set_field_name (gdaex_grid_column, field_name);
	gdaex_grid_column_set_gtype (gdaex_grid_column, type);
	gdaex_grid_column_set_visible (gdaex_grid_column, visible);
	gdaex_grid_column_set_resizable (gdaex_grid_column, resizable);
	gdaex_grid_column_set_sortable (gdaex_grid_column, sortable);
	gdaex_grid_column_set_reorderable (gdaex_grid_column, reorderable);
	gdaex_grid_column_set_decimals (gdaex_grid_column, decimals);

	return gdaex_grid_column;
}

void
gdaex_grid_column_set_title (GdaExGridColumn *column, const gchar *title)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	g_free (priv->title);
	priv->title = g_strdup (title);
}

const gchar
*gdaex_grid_column_get_title (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), NULL);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return (const gchar *)g_strdup (priv->title);
}

void
gdaex_grid_column_set_field_name (GdaExGridColumn *column, const gchar *field_name)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	g_free (priv->field_name);
	priv->field_name = g_strdup (field_name);
}

const gchar
*gdaex_grid_column_get_field_name (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), NULL);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return (const gchar *)g_strdup (priv->field_name);
}

void
gdaex_grid_column_set_gtype (GdaExGridColumn *column, GType type)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->type = type;
}

GType
gdaex_grid_column_get_gtype (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), G_TYPE_NONE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->type;
}

void
gdaex_grid_column_set_visible (GdaExGridColumn *column, gboolean visible)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->visible = visible;
}

gboolean
gdaex_grid_column_get_visible (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), FALSE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->visible;
}

void
gdaex_grid_column_set_resizable (GdaExGridColumn *column, gboolean resizable)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->resizable = resizable;
}

gboolean
gdaex_grid_column_get_resizable (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), FALSE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->resizable;
}

void
gdaex_grid_column_set_sortable (GdaExGridColumn *column, gboolean sortable)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->sortable = sortable;
}

gboolean
gdaex_grid_column_get_sortable (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), FALSE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->sortable;
}

void
gdaex_grid_column_set_reorderable (GdaExGridColumn *column, gboolean reorderable)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->reorderable = reorderable;
}

gboolean
gdaex_grid_column_get_reorderable (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), FALSE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->reorderable;
}

void
gdaex_grid_column_set_decimals (GdaExGridColumn *column, guint decimals)
{
	g_return_if_fail (GDAEX_IS_GRID_COLUMN (column));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	priv->decimals = decimals;
}

guint
gdaex_grid_column_get_decimals (GdaExGridColumn *column)
{
	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), FALSE);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	return priv->decimals;
}

GtkTreeViewColumn
*gdaex_grid_column_get_column (GdaExGridColumn *column)
{
	GdaExGridColumnPrivate *priv;

	GtkCellRenderer *renderer;

	g_return_val_if_fail (GDAEX_IS_GRID_COLUMN (column), NULL);

	priv = GDAEX_GRID_COLUMN_GET_PRIVATE (column);

	renderer = NULL;
	if (priv->type == G_TYPE_STRING
	    || priv->type == G_TYPE_DATE
	    || priv->type == G_TYPE_DATE_TIME
	    || priv->type == GDA_TYPE_TIMESTAMP)
		{
			renderer = gtk_cell_renderer_text_new ();
		}
	else if (priv->type == G_TYPE_INT
	    || priv->type == G_TYPE_FLOAT
	    || priv->type == G_TYPE_DOUBLE)
		{
			if (priv->decimals > -1)
				{
					renderer = gtk_cell_renderer_text_new ();
				}
			else
				{
					renderer = gtk_cell_renderer_spin_new ();
				}
			gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);
		}
	else
		{
			g_warning ("Error on creating the renderer for column «%s».", priv->title);
			return NULL;
		}

	if (priv->vcolumn != NULL)
		{
			g_object_unref (priv->vcolumn);
		}
	priv->vcolumn = gtk_tree_view_column_new ();

	gtk_tree_view_column_pack_start (priv->vcolumn, renderer, TRUE);

	gtk_tree_view_column_set_title (priv->vcolumn, priv->title);
	gtk_tree_view_column_set_resizable (priv->vcolumn, priv->resizable);
	gtk_tree_view_column_set_clickable (priv->vcolumn, priv->sortable);
	gtk_tree_view_column_set_reorderable (priv->vcolumn, priv->reorderable);

	return priv->vcolumn;
}

/* PRIVATE */
static void
gdaex_grid_column_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (object);
	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_grid_column_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (object);
	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
