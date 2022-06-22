#define VERSION	"1"
#define REVISION	"02"
#ifdef BETA
#define BETAVERS	" -BETA-"
#else
#define BETAVERS	""
#endif

#define VSTRING	VERSION"."REVISION
#ifdef _M68030
#define VERS		"AmiTCP Prefs "VSTRING" ("__DATE__") (68030)"BETAVERS
#define VERSTAG	"$VER: AmiTCP Prefs "VSTRING" ("__DATE__") 68030/040/060 version"
#else
#define VERS		"AmiTCP Prefs "VSTRING" ("__DATE__")"BETAVERS
#define VERSTAG	"$VER: AmiTCP Prefs "VSTRING" ("__DATE__") Generic 68'000 version"
#endif
