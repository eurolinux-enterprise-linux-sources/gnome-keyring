/*
 * Generated by gdbus-codegen 2.46.2. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef _____DAEMON_DBUS_GKD_DAEMON_GENERATED_H__
#define _____DAEMON_DBUS_GKD_DAEMON_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.gnome.keyring.Daemon */

#define GKD_TYPE_EXPORTED_DAEMON (gkd_exported_daemon_get_type ())
#define GKD_EXPORTED_DAEMON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GKD_TYPE_EXPORTED_DAEMON, GkdExportedDaemon))
#define GKD_IS_EXPORTED_DAEMON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GKD_TYPE_EXPORTED_DAEMON))
#define GKD_EXPORTED_DAEMON_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), GKD_TYPE_EXPORTED_DAEMON, GkdExportedDaemonIface))

struct _GkdExportedDaemon;
typedef struct _GkdExportedDaemon GkdExportedDaemon;
typedef struct _GkdExportedDaemonIface GkdExportedDaemonIface;

struct _GkdExportedDaemonIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_get_control_directory) (
    GkdExportedDaemon *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_environment) (
    GkdExportedDaemon *object,
    GDBusMethodInvocation *invocation);

};

GType gkd_exported_daemon_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *gkd_exported_daemon_interface_info (void);
guint gkd_exported_daemon_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void gkd_exported_daemon_complete_get_environment (
    GkdExportedDaemon *object,
    GDBusMethodInvocation *invocation,
    GVariant *Environment);

void gkd_exported_daemon_complete_get_control_directory (
    GkdExportedDaemon *object,
    GDBusMethodInvocation *invocation,
    const gchar *ControlDirectory);



/* D-Bus method calls: */
void gkd_exported_daemon_call_get_environment (
    GkdExportedDaemon *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gkd_exported_daemon_call_get_environment_finish (
    GkdExportedDaemon *proxy,
    GVariant **out_Environment,
    GAsyncResult *res,
    GError **error);

gboolean gkd_exported_daemon_call_get_environment_sync (
    GkdExportedDaemon *proxy,
    GVariant **out_Environment,
    GCancellable *cancellable,
    GError **error);

void gkd_exported_daemon_call_get_control_directory (
    GkdExportedDaemon *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gkd_exported_daemon_call_get_control_directory_finish (
    GkdExportedDaemon *proxy,
    gchar **out_ControlDirectory,
    GAsyncResult *res,
    GError **error);

gboolean gkd_exported_daemon_call_get_control_directory_sync (
    GkdExportedDaemon *proxy,
    gchar **out_ControlDirectory,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define GKD_TYPE_EXPORTED_DAEMON_PROXY (gkd_exported_daemon_proxy_get_type ())
#define GKD_EXPORTED_DAEMON_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GKD_TYPE_EXPORTED_DAEMON_PROXY, GkdExportedDaemonProxy))
#define GKD_EXPORTED_DAEMON_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GKD_TYPE_EXPORTED_DAEMON_PROXY, GkdExportedDaemonProxyClass))
#define GKD_EXPORTED_DAEMON_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GKD_TYPE_EXPORTED_DAEMON_PROXY, GkdExportedDaemonProxyClass))
#define GKD_IS_EXPORTED_DAEMON_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GKD_TYPE_EXPORTED_DAEMON_PROXY))
#define GKD_IS_EXPORTED_DAEMON_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GKD_TYPE_EXPORTED_DAEMON_PROXY))

typedef struct _GkdExportedDaemonProxy GkdExportedDaemonProxy;
typedef struct _GkdExportedDaemonProxyClass GkdExportedDaemonProxyClass;
typedef struct _GkdExportedDaemonProxyPrivate GkdExportedDaemonProxyPrivate;

struct _GkdExportedDaemonProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  GkdExportedDaemonProxyPrivate *priv;
};

struct _GkdExportedDaemonProxyClass
{
  GDBusProxyClass parent_class;
};

GType gkd_exported_daemon_proxy_get_type (void) G_GNUC_CONST;

void gkd_exported_daemon_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GkdExportedDaemon *gkd_exported_daemon_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
GkdExportedDaemon *gkd_exported_daemon_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void gkd_exported_daemon_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GkdExportedDaemon *gkd_exported_daemon_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
GkdExportedDaemon *gkd_exported_daemon_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define GKD_TYPE_EXPORTED_DAEMON_SKELETON (gkd_exported_daemon_skeleton_get_type ())
#define GKD_EXPORTED_DAEMON_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GKD_TYPE_EXPORTED_DAEMON_SKELETON, GkdExportedDaemonSkeleton))
#define GKD_EXPORTED_DAEMON_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GKD_TYPE_EXPORTED_DAEMON_SKELETON, GkdExportedDaemonSkeletonClass))
#define GKD_EXPORTED_DAEMON_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GKD_TYPE_EXPORTED_DAEMON_SKELETON, GkdExportedDaemonSkeletonClass))
#define GKD_IS_EXPORTED_DAEMON_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GKD_TYPE_EXPORTED_DAEMON_SKELETON))
#define GKD_IS_EXPORTED_DAEMON_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GKD_TYPE_EXPORTED_DAEMON_SKELETON))

typedef struct _GkdExportedDaemonSkeleton GkdExportedDaemonSkeleton;
typedef struct _GkdExportedDaemonSkeletonClass GkdExportedDaemonSkeletonClass;
typedef struct _GkdExportedDaemonSkeletonPrivate GkdExportedDaemonSkeletonPrivate;

struct _GkdExportedDaemonSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  GkdExportedDaemonSkeletonPrivate *priv;
};

struct _GkdExportedDaemonSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType gkd_exported_daemon_skeleton_get_type (void) G_GNUC_CONST;

GkdExportedDaemon *gkd_exported_daemon_skeleton_new (void);


G_END_DECLS

#endif /* _____DAEMON_DBUS_GKD_DAEMON_GENERATED_H__ */
