
#ifndef __gck_marshal_MARSHAL_H__
#define __gck_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:VOID (gck-marshal.list:1) */
extern void gck_marshal_BOOLEAN__VOID (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data);

/* VOID:STRING,ULONG (gck-marshal.list:2) */
extern void gck_marshal_VOID__STRING_ULONG (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

G_END_DECLS

#endif /* __gck_marshal_MARSHAL_H__ */

