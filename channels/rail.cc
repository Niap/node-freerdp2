#include "rail.h"

static UINT node_rail_server_handshake(RailClientContext* context,
	RAIL_HANDSHAKE_ORDER* handshake)
{
	RAIL_EXEC_ORDER exec;
	RAIL_SYSPARAM_ORDER sysparam;
	RAIL_HANDSHAKE_ORDER clientHandshake;
	RAIL_CLIENT_STATUS_ORDER clientStatus;
	nodeContext* nctx = (nodeContext*)context->custom;
	rdpSettings* settings = nctx->context.settings;
	clientHandshake.buildNumber = 0x00001DB0;
	context->ClientHandshake(context, &clientHandshake);
	ZeroMemory(&clientStatus, sizeof(RAIL_CLIENT_STATUS_ORDER));
	clientStatus.flags = RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE;
	context->ClientInformation(context, &clientStatus);

	if (settings->RemoteAppLanguageBarSupported)
	{
		RAIL_LANGBAR_INFO_ORDER langBarInfo;
		langBarInfo.languageBarStatus = 0x00000008; /* TF_SFT_HIDDEN */
		context->ClientLanguageBarInfo(context, &langBarInfo);
	}

	ZeroMemory(&sysparam, sizeof(RAIL_SYSPARAM_ORDER));
	sysparam.params = 0;
	sysparam.params |= SPI_MASK_SET_HIGH_CONTRAST;
	sysparam.highContrast.colorScheme.string = NULL;
	sysparam.highContrast.colorScheme.length = 0;
	sysparam.highContrast.flags = 0x7E;
	sysparam.params |= SPI_MASK_SET_MOUSE_BUTTON_SWAP;
	sysparam.mouseButtonSwap = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_PREF;
	sysparam.keyboardPref = FALSE;
	sysparam.params |= SPI_MASK_SET_DRAG_FULL_WINDOWS;
	sysparam.dragFullWindows = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_CUES;
	sysparam.keyboardCues = FALSE;
	sysparam.params |= SPI_MASK_SET_WORK_AREA;
	sysparam.workArea.left = 0;
	sysparam.workArea.top = 0;
	sysparam.workArea.right = settings->DesktopWidth;
	sysparam.workArea.bottom = settings->DesktopHeight;
	sysparam.dragFullWindows = FALSE;
	context->ClientSystemParam(context, &sysparam);
	ZeroMemory(&exec, sizeof(RAIL_EXEC_ORDER));
	exec.RemoteApplicationProgram = settings->RemoteApplicationProgram;
	exec.RemoteApplicationWorkingDir = settings->ShellWorkingDirectory;
	exec.RemoteApplicationArguments = settings->RemoteApplicationCmdLine;
	context->ClientExecute(context, &exec);
	return CHANNEL_RC_OK;
}

void node_rail_init(nodeContext* nctx, RailClientContext* rail)
{
	nctx->rail = rail;
	rail->custom = (void*)nctx;
	rail->ServerHandshake = reinterpret_cast<pcRailServerHandshake>(node_rail_server_handshake);
}

void node_rail_uninit(nodeContext* nctx, RailClientContext* rail)
{
	nctx->rail = NULL;
	rail->custom = NULL;
}
