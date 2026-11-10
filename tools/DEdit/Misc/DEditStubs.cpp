// Globals that the shared engine code references but that only exist in the
// game/server builds. DEdit never calls through them.
#include "bdefs.h"

class ILTServer;
ILTServer *g_pLTServer = NULL;
