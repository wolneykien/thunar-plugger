
G_BEGIN_DECLS;

#define SHAREMAN_TYPE_FILE            (thunar-plugger_file_get_type ())
#define SHAREMAN_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SHAREMAN_TYPE_FILE, SharemanFile))

typedef struct _SharemanFileClass SharemanFileClass;
typedef struct _SharemanFile      SharemanFile;

struct _SharemanFileClass
{
  GObjectClass __parent__;

  /* signals */
  void (*destroy) (SharemanFile *file);
};

struct _SharemanFile
{
  GObject        __parent__;

  /*< private >*/
  GFile         *gfile;
};

GType thunar-plugger_file_get_type (void) G_GNUC_CONST;

SharemanFile* thunar-plugger_file_get (const char *path);

G_END_DECLS;
