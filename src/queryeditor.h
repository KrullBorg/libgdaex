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


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_H__ */
