
G_BEGIN_DECLS;

#define THUNAR_PLUGGER_TYPE_FILE            (thunar_plugger_file_get_type ())
#define THUNAR_PLUGGER_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), THUNAR_PLUGGER_TYPE_FILE, ThunarPluggerFile))

typedef struct _ThunarPluggerFileClass ThunarPluggerFileClass;
typedef struct _ThunarPluggerFile      ThunarPluggerFile;

struct _ThunarPluggerFileClass
{
  GObjectClass __parent__;

  /* signals */
  void (*destroy) (ThunarPluggerFile *file);
};

struct _ThunarPluggerFile
{
  GObject        __parent__;

  /*< private >*/
  GFile         *gfile;
};

GType thunar_plugger_file_get_type (void) G_GNUC_CONST;

ThunarPluggerFile* thunar_plugger_file_get (const char *path);

G_END_DECLS;
