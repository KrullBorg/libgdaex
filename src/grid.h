/*
 *  grid.h
 *
 *  Copyright (C) 2010-2013 Andrea Zagli <azagli@libero.it>
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

#ifndef __GDAEX_GRID_H__
#define __GDAEX_GRID_H__

#include <glib.h>
#include <glib-object.h>

#ifdef SOLIPA_FOUND
	#include <libsolipa/libsolipa.h>
#endif

#include "gdaex.h"
#include "gridcolumn.h"

G_BEGIN_DECLS


#define GDAEX_TYPE_GRID                 (gdaex_grid_get_type ())
#define GDAEX_GRID(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_TYPE_GRID, GdaExGrid))
#define GDAEX_GRID_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GDAEX_TYPE_GRID, GdaExGridClass))
#define GDAEX_IS_GRID(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_TYPE_GRID))
#define GDAEX_IS_GRID_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GDAEX_TYPE_GRID))
#define GDAEX_GRID_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GDAEX_TYPE_GRID, GdaExGridClass))


typedef struct _GdaExGrid GdaExGrid;
typedef struct _GdaExGridClass GdaExGridClass;

struct _GdaExGrid
	{
		GObject parent;
	};

struct _GdaExGridClass
	{
		GObjectClass parent_class;
	};

GType gdaex_grid_get_type (void) G_GNUC_CONST;


GdaExGrid *gdaex_grid_new (void);

void gdaex_grid_set_title (GdaExGrid *grid, const gchar *title);
const gchar *gdaex_grid_get_title (GdaExGrid *grid);

void gdaex_grid_add_column (GdaExGrid *grid, GdaExGridColumn *column);
void gdaex_grid_add_columns (GdaExGrid *grid, GSList *columns);

void gdaex_grid_clear (GdaExGrid *grid);

GtkWidget *gdaex_grid_get_widget (GdaExGrid *grid);

gboolean gdaex_grid_fill_from_sql (GdaExGrid *grid, GdaEx *gdaex, const gchar *sql, GError **error);
gboolean gdaex_grid_fill_from_datamodel (GdaExGrid *grid, GdaDataModel *dm, GError **error);

#ifdef SOLIPA_FOUND
void gdaex_grid_set_solipa (GdaExGrid *grid, Solipa *solipa);
#endif


G_END_DECLS

#endif /* __GDAEX_GRID_H__ */
