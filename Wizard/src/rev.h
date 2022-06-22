#define VERSION   "1"
#define REVISION  "00"
#ifdef BETA
#define BETAVERS  " -BETA-"
#else
#define BETAVERS  ""
#endif

#define VSTRING   VERSION"."REVISION
#ifdef _M68030
#define VERS      "SetupAmiTCP "VSTRING" ("__DATE__") (68030)"BETAVERS
#define VERSTAG   "$VER: SetupAmiTCP "VSTRING" ("__DATE__") 68030/040/060 version"
#else
#define VERS      "SetupAmiTCP "VSTRING" ("__DATE__")"BETAVERS
#define VERSTAG   "$VER: SetupAmiTCP "VSTRING" ("__DATE__") Generic 68'000 version"
#endif
