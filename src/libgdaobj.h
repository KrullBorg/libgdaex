/*
 *  libgdaobj.h
 *
 *  Copyright (C) 2005-2006 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaobj.
 *  
 *  libgdaobj is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaobj is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaobj; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 */

#include <glib.h>
#include <glib-object.h>
#include <libgda/libgda.h>

#ifndef __GDAO_H__
#define __GDAO_H__

G_BEGIN_DECLS


#define TYPE_GDAO                 (gdao_get_type ())
#define GDAO(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GDAO, GdaO))
#define GDAO_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GDAO, GdaOClass))
#define IS_GDAO(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GDAO))
#define IS_GDAO_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GDAO))
#define GDAO_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GDAO, GdaOClass))


typedef struct _GdaO GdaO;
typedef struct _GdaOClass GdaOClass;

struct _GdaO
	{
		GObject parent;
	};

struct _GdaOClass
	{
		GObjectClass parent_class;
	};

GType gdao_get_type (void) G_GNUC_CONST;


GdaO *gdao_new (GdaClient *gda_client,
                const gchar *datasource,
                const gchar *username,
                const gchar *password);
GdaO *gdao_new_from_string (GdaClient *gda_client,
                            const gchar *provider_id,
                            const gchar *cnc_string);
GdaO *gdao_new_from_connection (GdaConnection *conn);

const GdaClient *gdao_get_gdaclient (GdaO *gdao);
const GdaConnection *gdao_get_gdaconnection (GdaO *gdao);
const gchar *gdao_get_provider (GdaO *gdao);

GdaDataModel *gdao_query (GdaO *gdao, const gchar *sql);

gchar *gdao_data_model_get_field_value_stringify_at (GdaDataModel *data_model,
                                                     gint row,
                                                     const gchar *field_name);
gint gdao_data_model_get_field_value_integer_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
gfloat gdao_data_model_get_field_value_float_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
gdouble gdao_data_model_get_field_value_double_at (GdaDataModel *data_model,
                                                   gint row,
                                                   const gchar *field_name);
gboolean gdao_data_model_get_field_value_boolean_at (GdaDataModel *data_model,
                                                     gint row,
                                                     const gchar *field_name);
GDate *gdao_data_model_get_field_value_gdate_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);

gchar *gdao_data_model_get_value_stringify_at (GdaDataModel *data_model,
                                               gint row,
                                               gint col);
gint gdao_data_model_get_value_integer_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
gfloat gdao_data_model_get_value_float_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
gdouble gdao_data_model_get_value_double_at (GdaDataModel *data_model,
                                             gint row,
                                             gint col);
gboolean gdao_data_model_get_value_boolean_at (GdaDataModel *data_model,
                                               gint row,
                                               gint col);
GDate *gdao_data_model_get_value_gdate_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);

gboolean gdao_begin (GdaO *gdao);

gint gdao_execute (GdaO *gdao, const gchar *sql);

gboolean gdao_commit (GdaO *gdao);
gboolean gdao_rollback (GdaO *gdao);

void gdao_free (GdaO *gdao);

gchar *gdao_strescape (const gchar *source, const gchar *exceptions);

gchar gdao_get_chr_quoting (GdaO *gdao);


G_END_DECLS

#endif /* __GDAO_H__ */
