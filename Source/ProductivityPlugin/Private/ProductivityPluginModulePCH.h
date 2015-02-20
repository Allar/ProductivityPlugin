// Some copyright should be here...
#pragma once

#include "Core.h"
#include "Engine.h"
#include "Networking.h"
#include "SlateBasics.h"
#include "SlateCore.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

#include "ProductivityTypes.h"
#include "ProductivitySettings.h"
#include "ProductivityPluginModule.h"
#include "InstancedMeshWrapper.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

DECLARE_LOG_CATEGORY_EXTERN(LogProductivityPlugin, VeryVerbose, All);

/* Private constants
*****************************************************************************/

/**
* Defines the default IP endpoint for the Slate Remote server running in the Editor.
*/
#define PRODUCTIVITY_SERVER_DEFAULT_EDITOR_ENDPOINT FIPv4Endpoint(FIPv4Address(127, 0, 0, 1), 51234)

/**
* Defines the protocol version of the UDP message transport.
*/
#define PRODUCTIVITY_SERVER_PROTOCOL_VERSION 1
