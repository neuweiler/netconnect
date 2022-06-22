#define VERSION   "2"
#define REVISION  "0"
#ifdef BETA
#define BETAVERS  " -BETA-"
#else
#define BETAVERS  ""
#endif

#define VSTRING   VERSION"."REVISION
#ifdef _M68030
#define VERS      "GenesisPrefs "VSTRING" ("__DATE__") (68030)"BETAVERS
#define VERSTAG   "$VER: GenesisPrefs "VSTRING" ("__DATE__") 68030/040/060 version"
#else
#define VERS      "GenesisPrefs "VSTRING" ("__DATE__")"BETAVERS
#define VERSTAG   "$VER: GenesisPrefs "VSTRING" ("__DATE__") Generic 68'000 version"
#endif
