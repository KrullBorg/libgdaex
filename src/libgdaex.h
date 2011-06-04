/*
 *  libgdaex.h
 *
 *  Copyright (C) 2005-2011 Andrea Zagli <azagli@libero.it>
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
 *  along with libgdaexbj; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 */

#include <time.h>

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <libgda/libgda.h>

#ifndef __GDAEX_H__
#define __GDAEX_H__

G_BEGIN_DECLS


#define TYPE_GDAEX                 (gdaex_get_type ())
#define GDAEX(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GDAEX, GdaEx))
#define GDAEX_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GDAEX, GdaExClass))
#define IS_GDAEX(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GDAEX))
#define IS_GDAEX_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GDAEX))
#define GDAEX_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GDAEX, GdaExClass))


typedef struct _GdaEx GdaEx;
typedef struct _GdaExClass GdaExClass;

struct _GdaEx
	{
		GObject parent;
	};

struct _GdaExClass
	{
		GObjectClass parent_class;

		guint before_execute_signal_id;
		guint after_execute_signal_id;
	};

GType gdaex_get_type (void) G_GNUC_CONST;


GdaEx *gdaex_new_from_dsn (const gchar *dsn,
                           const gchar *username,
                           const gchar *password);
GdaEx *gdaex_new_from_string (const gchar *cnc_string);
GdaEx *gdaex_new_from_connection (GdaConnection *conn);

GOptionGroup *gdaex_get_option_group (GdaEx *gdaex);

const GdaConnection *gdaex_get_gdaconnection (GdaEx *gdaex);
const gchar *gdaex_get_provider (GdaEx *gdaex);

const gchar *gdaex_get_tables_name_prefix (GdaEx *gdaex);
void gdaex_set_tables_name_prefix (GdaEx *gdaex, const gchar *tables_name_prefix);

GdaDataModel *gdaex_query (GdaEx *gdaex, const gchar *sql);

gchar *gdaex_data_model_get_field_value_stringify_at (GdaDataModel *data_model,
                                                     gint row,
                                                     const gchar *field_name);
gint gdaex_data_model_get_field_value_integer_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
gfloat gdaex_data_model_get_field_value_float_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
gdouble gdaex_data_model_get_field_value_double_at (GdaDataModel *data_model,
                                                   gint row,
                                                   const gchar *field_name);
gboolean gdaex_data_model_get_field_value_boolean_at (GdaDataModel *data_model,
                                                     gint row,
                                                     const gchar *field_name);
GdaTimestamp *gdaex_data_model_get_field_value_gdatimestamp_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
GDate *gdaex_data_model_get_field_value_gdate_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
GDateTime *gdaex_data_model_get_field_value_gdatetime_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);
struct tm *gdaex_data_model_get_field_value_tm_at (GdaDataModel *data_model,
                                                 gint row,
                                                 const gchar *field_name);

gchar *gdaex_data_model_get_value_stringify_at (GdaDataModel *data_model,
                                               gint row,
                                               gint col);
gint gdaex_data_model_get_value_integer_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
gfloat gdaex_data_model_get_value_float_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
gdouble gdaex_data_model_get_value_double_at (GdaDataModel *data_model,
                                             gint row,
                                             gint col);
gboolean gdaex_data_model_get_value_boolean_at (GdaDataModel *data_model,
                                               gint row,
                                               gint col);
GdaTimestamp *gdaex_data_model_get_value_gdatimestamp_at (GdaDataModel *data_model,
                                                          gint row,
                                                          gint col);
GDate *gdaex_data_model_get_value_gdate_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
GDateTime *gdaex_data_model_get_value_gdatetime_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);
struct tm *gdaex_data_model_get_value_tm_at (GdaDataModel *data_model,
                                           gint row,
                                           gint col);

gchar *gdaex_data_model_iter_get_field_value_stringify_at (GdaDataModelIter *iter,
                                                     const gchar *field_name);
gint gdaex_data_model_iter_get_field_value_integer_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);
gfloat gdaex_data_model_iter_get_field_value_float_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);
gdouble gdaex_data_model_iter_get_field_value_double_at (GdaDataModelIter *iter,
                                                   const gchar *field_name);
gboolean gdaex_data_model_iter_get_field_value_boolean_at (GdaDataModelIter *iter,
                                                     const gchar *field_name);
GdaTimestamp *gdaex_data_model_iter_get_field_value_gdatimestamp_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);
GDate *gdaex_data_model_iter_get_field_value_gdate_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);
GDateTime *gdaex_data_model_iter_get_field_value_gdatetime_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);
struct tm *gdaex_data_model_iter_get_field_value_tm_at (GdaDataModelIter *iter,
                                                 const gchar *field_name);

gchar *gdaex_data_model_iter_get_value_stringify_at (GdaDataModelIter *iter,
                                               gint col);
gint gdaex_data_model_iter_get_value_integer_at (GdaDataModelIter *iter,
                                           gint col);
gfloat gdaex_data_model_iter_get_value_float_at (GdaDataModelIter *iter,
                                           gint col);
gdouble gdaex_data_model_iter_get_value_double_at (GdaDataModelIter *iter,
                                             gint col);
gboolean gdaex_data_model_iter_get_value_boolean_at (GdaDataModelIter *iter,
                                               gint col);
GdaTimestamp *gdaex_data_model_iter_get_value_gdatimestamp_at (GdaDataModelIter *iter,
                                                          gint col);
GDate *gdaex_data_model_iter_get_value_gdate_at (GdaDataModelIter *iter,
                                           gint col);
GDateTime *gdaex_data_model_iter_get_value_gdatetime_at (GdaDataModelIter *iter,
                                           gint col);
struct tm *gdaex_data_model_iter_get_value_tm_at (GdaDataModelIter *iter,
                                           gint col);

gboolean gdaex_begin (GdaEx *gdaex);

gint gdaex_execute (GdaEx *gdaex, const gchar *sql);

GSList *gdaex_batch_execute (GdaEx *gdaex, ...);

gboolean gdaex_commit (GdaEx *gdaex);
gboolean gdaex_rollback (GdaEx *gdaex);

void gdaex_free (GdaEx *gdaex);

gchar *gdaex_strescape (const gchar *source, const gchar *exceptions);

gchar gdaex_get_chr_quoting (GdaEx *gdaex);

void gdaex_fill_liststore_from_sql (GdaEx *gdaex, GtkListStore *lstore, const gchar *sql,
                                     guint *cols_formatted,
                                     gchar *(*cols_format_func) (GdaDataModelIter *, guint));
void gdaex_fill_liststore_from_datamodel (GdaEx *gdaex, GtkListStore *lstore, GdaDataModel *dm,
                                     guint *cols_formatted,
                                     gchar *(*cols_format_func) (GdaDataModelIter *, guint));


G_END_DECLS

#endif /* __GDAEX_H__ */
