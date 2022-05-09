/**
 * Ashita - Copyright (c) 2014 - 2016 atom0s [atom0s@live.com]
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * By using Ashita, you agree to the above license and its terms.
 *
 *      Attribution - You must give appropriate credit, provide a link to the license and indicate if changes were
 *                    made. You must do so in any reasonable manner, but not in any way that suggests the licensor
 *                    endorses you or your use.
 *
 *   Non-Commercial - You may not use the material (Ashita) for commercial purposes.
 *
 *   No-Derivatives - If you remix, transform, or build upon the material (Ashita), you may not distribute the
 *                    modified material. You are, however, allowed to submit the modified works back to the original
 *                    Ashita project in attempt to have it added to the original project.
 *
 * You may not apply legal terms or technological measures that legally restrict others
 * from doing anything the license permits.
 *
 * No warranties are given.
 */

#include "FFXIMOVE.h"
#pragma comment(lib, "psapi.lib")
#include <psapi.h>
#include <stdint.h>

/**
 * Constructor and Deconstructor
 */
FFXIMOVE::FFXIMOVE(void)
    : m_AshitaCore(nullptr)
    , m_LogManager(nullptr)
    , m_PluginId(0)
    , m_Direct3DDevice(nullptr)
{ }
FFXIMOVE::~FFXIMOVE(void)
{ }


/**
 * Returns the plugins information structure.
 *
 * @returns {plugininfo_t} The plugin information structure of the plugin.
 */
plugininfo_t FFXIMOVE::GetPluginInfo(void)
{
    return *g_PluginInfo;
}

/**
 * Invoked when the plugin is loaded, allowing it to prepare for usage.
 *
 * @param {IAshitaCore*} core - The Ashita core object to interact with the various Ashita managers.
 * @param {ILogManager*} log - The log manager used to interact with the current log file.
 * @param {uint32_t} id - The plugins id, or its module base, that identifies it other than its name.
 * @returns {bool} True on success, false otherwise. (If false, the plugin will not be loaded.)
 *
 * @notes
 *
 *      Plugins must return true here or they will be considered invalid and unload immediately.
 *      Returning false means that the plugin failed to initialize and should not be used.
 */
bool FFXIMOVE::Initialize(IAshitaCore* core, ILogManager* log, uint32_t id)
{
    // Store the variables for later usage..
    this->m_AshitaCore = core;
    this->m_LogManager = log;
    this->m_PluginId = id;


    //Load the follow address
    //https://git.ashitaxi.com/Plugins/MultiSend/src/branch/master/src/MultiSend/Main.cpp
    DWORD Pointer = NULL;
    MODULEINFO mod = { 0 };
    if (!::GetModuleInformation(::GetCurrentProcess(), ::GetModuleHandle("FFXiMain.dll"), &mod, sizeof(MODULEINFO)))
        return false;

    Pointer = Ashita::Memory::FindPattern((uintptr_t)mod.lpBaseOfDll, (uintptr_t)mod.SizeOfImage,
        "8BCFE8????FFFF8B0D????????E8????????8BE885ED750CB9",
        0, 0);
    if (Pointer == NULL) return false;
    Pointer += 25;
    p_Follow = (sFollow*)(*((DWORD*)Pointer));

    //settings and output helper ashita4 only
    //pOutput = new OutputHelpers(core, log, "ffximove");
    //pSettings = new SettingsHelper(core, pOutput, "ffximove");

    //initialize target x and z to 0
    s_tarpos_x = 0;
    s_tarpos_z = 0;

    //Default settings.
    c_run = false;
    c_attemptzone = false;
    c_debug = false;
    c_safemode = false;
    c_maxdist = 5000.0f;

    return true;
}

/**
 * Invoked when the plugin is being unloaded, allowing it to cleanup its resources.
 *
 * @notes
 *
 *      Plugins should use this function to cleanup non-Direct3D related objects.
 *          - Delete created font objects.
 *          - Delete created Gui objects.
 *          - Internal memory allocations.
 */
void FFXIMOVE::Release(void)
{ }

/**
 * Invoked when a command is being processed by the game client.
 *
 * Note:
 *      Please note, this handles all things done via the game in terms of commands
 *      and chat. All / commands as well as normal chat you type, macros, etc. will
 *      be processed through here. You should only use this to handle / commands.
 *
 *      If you wish to handle other bits of outgoing text before the client sees it,
 *      then use the HandleOutgoingText callback instead.
 *
 * @param {const char*} command         - The raw command string being processed.
 * @param {uint32_t} type               - The type of the command being processed. (See Ashita::CommandInputType enumeration.)
 * @returns {bool} True if handled and should be blocked, false otherwise.
 *
 * @notes
 *
 *      Plugins can handle and block input from being sent to the game in this function. Returning
 *      true will block the current command from happening at all within the client. Plugins should
 *      only return true when the command passed to this function is being handled by the plugin.
 */
bool FFXIMOVE::HandleCommand(const char* command, int32_t type)
{
    UNREFERENCED_PARAMETER(type);

    // Split the command into args..
    std::vector<std::string> args;
    auto count = Ashita::Commands::GetCommandArgs(command, &args);

    // Handle the example slash command..
    HANDLECOMMAND("/fm")
    {
        // Check for a parameter..
        if (count >= 2 && args[1] == "run") {
            this->m_AshitaCore->GetChatManager()->Write("Starting run");
            //uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
            //float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
            //float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);
            s_tarpos_x = std::atoi(args[2].c_str());
            s_tarpos_z = std::atoi(args[3].c_str());
            c_run = true;
        }
        else if (count >= 2 && args[1] == "stop") {
            this->m_AshitaCore->GetChatManager()->Write("Stopping");
            c_run = false;
        }
        else if (count >= 3 && args[1] == "save") {
            this->m_AshitaCore->GetChatManager()->Write("Save waypoint");
            uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
            float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
            float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);
            float my_pos_y = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalY(myindex);
            FFXIMOVE::SaveWaypoint(my_pos_x, my_pos_z, my_pos_y, args[2].c_str());
        }
        // Return true here to block this command from going to the client.
        return true;
    }

    // Return false here to allow unhandled commands to continue to be processed.
    return false;
}

/**
 * Invoked when incoming text being sent to the chat log is being processed.
 *
 * @param {int16_t} mode                - The mode of the message being added to the chatlog.
 * @param {const char*} message         - The raw message being added to the chat log.
 * @param {int16_t*} modifiedMode       - The modified mode, if any, that has been altered by other plugins/addons.
 * @param {char*} modifiedMessage       - The modified message, if any, that has been altered by other plugins/addons.
 * @param {bool} blocked                - Flag if this message has been blocked already by another plugin. (Once blocked, other plugins cannot restore it.)
 * @returns {bool} True if handled and should be blocked, false otherwise.
 *
 * @notes
 *
 *      Plugins can block the incoming text line by returning true in this function.
 *      Plugins can override the incoming text mode by setting the value of modifiedMode.
 *      Plugins can override the incoming text by writing a new message to the modifiedMessage buffer.
 *      Plugins can check if other plugins have blocked the current message by checking of the blocked param is true.
 */
bool FFXIMOVE::HandleIncomingText(int16_t mode, const char* message, int16_t* modifiedMode, char* modifiedMessage, bool blocked)
{
    UNREFERENCED_PARAMETER(mode);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(modifiedMode);
    UNREFERENCED_PARAMETER(modifiedMessage);
    UNREFERENCED_PARAMETER(blocked);

    return false;
}

/**
 * Invoked when outgoing text has not been handled by other plugins/addons.
 * Invoked after HandleCommand if nothing else processed the data.
 *
 * @param {int16_t} mode                - The type of the text that is being sent. (See Ashita::CommandInputType enumeration.)
 * @param {const char*} message         - The raw message being sent.
 * @param {int16_t*} modifiedMode       - The modified mode, if any, that has been altered by other plugins/addons.
 * @param {char*} modifiedMessage       - The modified message, if any, that has been altered by other plugins/addons.
 * @param {bool} blocked                - Flag if this message has been blocked already by another plugin. (Once blocked, other plugins cannot restore it.)
 * @returns {bool} True if handled and should be blocked, false otherwise.
 *
 * @notes
 *
 *      Plugins can block the outgoing text line by returning true in this function.
 *      Plugins can override the outgoing text mode by setting the value of modifiedMode.
 *      Plugins can override the outgoing text by writing a new message to the modifiedMessage buffer.
 *      Plugins can check if other plugins have blocked the current message by checking of the blocked param is true.
 *
 *      Plugins can use this event as a last-ditch effort to handle outgoing text being sent from the client.
 *      This event can be used for overriding things like custom token parsing for things such as:
 *          %player_name% to be parsed out and replaced with the actual player name.
 */
bool FFXIMOVE::HandleOutgoingText(int32_t type, const char* message, int32_t* modifiedType, char* modifiedMessage, bool blocked)
{
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(modifiedType);
    UNREFERENCED_PARAMETER(modifiedMessage);
    UNREFERENCED_PARAMETER(blocked);

    return false;
}

/**
 * Invoked when an incoming packet is being handled.
 *
 * @param {uint16_t} id                 - The id of the packet.
 * @param {uint32_t} size               - The size of the packet data.
 * @param {void*} data                  - The raw data of the packet.
 * @param {void*} modified              - The modified data, if any, that has been altered by other plugins/addons.
 * @param {bool} blocked                - Flag if this message has been blocked already by another plugin. (Once blocked, other plugins cannot restore it.)
 * @returns {bool} True if handled and should be blocked, false otherwise.
 *
 * @notes
 *
 *      Plugins can block the packet by returning true in this function.
 *      Plugins can alter the data of the packet by editing the modified data parameter.
 *      Plugins can check if other plugins have blocked the current packet by checking if the blocked param is true.
 *
 *      Please note; altering packets incorrectly or altering the flow of packets incorrectly can have adverse affects
 *      and possibly lead to players being banned. Edit with caution!
 */
bool FFXIMOVE::HandleIncomingPacket(uint16_t id, uint32_t size, void* data, void* modified, bool blocked)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(modified);
    UNREFERENCED_PARAMETER(blocked);

    return false;
}

/**
 * Invoked when an outgoing packet is being handled.
 *
 * @param {uint16_t} id                 - The id of the packet.
 * @param {uint32_t} size               - The size of the packet data.
 * @param {void*} data                  - The raw data of the packet.
 * @param {void*} modified              - The modified data, if any, that has been altered by other plugins/addons.
 * @param {bool} blocked                - Flag if this message has been blocked already by another plugin. (Once blocked, other plugins cannot restore it.)
 * @returns {bool} True if handled and should be blocked, false otherwise.
 *
 * @notes
 *
 *      Plugins can block the packet by returning true in this function.
 *      Plugins can alter the data of the packet by editing the modified data parameter.
 *      Plugins can check if other plugins have blocked the current packet by checking if the blocked param is true.
 *
 *      Please note; altering packets incorrectly or altering the flow of packets incorrectly can have adverse affects
 *      and possibly lead to players being banned. Edit with caution!
 */
bool FFXIMOVE::HandleOutgoingPacket(uint16_t id, uint32_t size, void* data, void* modified, bool blocked)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(modified);
    UNREFERENCED_PARAMETER(blocked);

    return false;
}

/**
 * Invoked when the plugin is being initialized for Direct3D rendering.
 *
 * Note:
 *      Plugins must return true with this function in order to have the other Direct3D
 *      functions invoked. Returning false is ideal here if you do not need to use the
 *      Direct3D functions within your plugin. This can help with overall performance.
 *
 * @param {IDirect3DDevice8*} device    - The Direct3D device pointer currently being used by the game.
 * @return {bool} True if the plugin should handle the other Direct3D messages, false otherwise.
 */
bool FFXIMOVE::Direct3DInitialize(IDirect3DDevice8* device)
{
    // Store the device for later usage..
    this->m_Direct3DDevice = device;

    /**
     * Returning true here for the sake of the example plugin! If you do not need
     * to make use of any of the Direct3D calls, it is recommended to return false
     * here to save on performance!
     */

    return true;
}

/**
 * Invoked when the plugin is being unloaded and is able to cleanup its Direct3D related resources.
 *
 * @notes
 *
 *      Plugins should use this function to cleanup Direct3D related objects.
 *          - Index Buffers, Vertex Buffers
 *          - Textures
 */
void FFXIMOVE::Direct3DRelease(void)
{ }

/**
 * Invoked when the Direct3D device is beginning to render. (BeginScene)
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called after BeginScene is called.
 */
void FFXIMOVE::Direct3DPreRender(void)
{ 
    if (c_run)
    {
        uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
        float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
        float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);
        s_vector_x = s_tarpos_x - my_pos_x;
        s_vector_z = s_tarpos_z - my_pos_z;
        double distance = sqrt(pow(s_vector_x, 2) + pow(s_vector_z, 2));

        if ((distance > 0.4f) && (distance < c_maxdist))
        {
            p_Follow->FollowID = 0;
            p_Follow->FollowIndex = 0;
            p_Follow->DirX = s_vector_x;
            p_Follow->DirY = 0;
            p_Follow->DirZ = s_vector_z;
            p_Follow->Autorun = 1;
            s_last_run_state = true;
        }
        else if (s_last_run_state)
        {
            p_Follow->Autorun = 0;
            c_run = false;
            s_last_run_state = false;
        }
    }
}

/**
 * Invoked when the Direct3D device is ending its rendering. (EndScene)
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before EndScene is called.
 */
void FFXIMOVE::Direct3DRender(void)
{ }

/**
 * Invoked when the Direct3D device is presenting the scene. (Present)
 *
 * @param {RECT*} pSourceRect               - The source rect being rendered into.
 * @param {RECT*} pDestRect                 - The destination rect being rendered from.
 * @param {HWND} hDestWindowOverride        - The window handle, if any, to override the rendering into.
 * @param {RGNDATA*} pDirtyRegion           - The dirty region data.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before Present is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
    UNREFERENCED_PARAMETER(pSourceRect);
    UNREFERENCED_PARAMETER(pDestRect);
    UNREFERENCED_PARAMETER(hDestWindowOverride);
    UNREFERENCED_PARAMETER(pDirtyRegion);

    return false;
}

/**
 * Invoked when the Direct3D device is drawing a primitive to the scene. (DrawPrimitive)
 *
 * @param {D3DPRIMITIVETYPE} PrimitiveType  - The type of primitive being rendered.
 * @param {UINT} StartVertex                - Index of the first vertex to load.
 * @param {UINT} PrimitiveCount             - Number of primitives to render.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before DrawPrimitive is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(StartVertex);
    UNREFERENCED_PARAMETER(PrimitiveType);

    return false;
}

/**
 * Invoked when the Direct3D device is drawing a primitive to the scene. (DrawIndexedPrimitive)
 *
 * @param {D3DPRIMITIVETYPE} PrimitiveType  - The type of primitive being rendered.
 * @param {UINT} minIndex                   - Minimum vertex index for vertices used during this call.
 * @param {UINT} numVertices                - Number of vertices used during this call
 * @param {UINT} startIndex                 - Index of the first index to use when accesssing the vertex buffer.
 * @param {UINT} primCount                  - Number of primitives to render.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before DrawIndexedPrimitive is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(minIndex);
    UNREFERENCED_PARAMETER(NumVertices);
    UNREFERENCED_PARAMETER(startIndex);
    UNREFERENCED_PARAMETER(primCount);

    return false;
}

/**
 * Invoked when the Direct3D device is drawing a primitive to the scene. (DrawPrimitiveUP)
 *
 * @param {D3DPRIMITIVETYPE} PrimitiveType  - The type of primitive being rendered.
 * @param {UINT} PrimitiveCount             - Number of primitives to render.
 * @param {void*} pVertexStreamZeroData     - User memory pointer to the vertex data.
 * @param {UINT} VertexStreamZeroStride     - The number of bytes of data for each vertex.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before DrawPrimitiveUP is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(PrimitiveCount);
    UNREFERENCED_PARAMETER(pVertexStreamZeroData);
    UNREFERENCED_PARAMETER(VertexStreamZeroStride);

    return false;
}

/**
 * Invoked when the Direct3D device is drawing a primitive to the scene. (DrawIndexedPrimitiveUP)
 *
 * @param {D3DPRIMITIVETYPE} PrimitiveType  - The type of primitive being rendered.
 * @param {UINT} MinVertexIndex             - Minimum vertex index.
 * @param {UINT} NumVertexIndices           - Number of vertices used during this call.
 * @param {UINT} PrimitiveCount             - Number of primitives to render.
 * @param {void*} pIndexData                - User memory pointer to the index data.
 * @param {D3DFORMAT} IndexDataFormat       - The format of the index data.
 * @param {void*} pVertexStreamZeroData     - User memory pointer to the vertex data.
 * @param {UINT} VertexStreamZeroStride     - The number of bytes of data for each vertex.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before DrawIndexedPrimitiveUP is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(MinVertexIndex);
    UNREFERENCED_PARAMETER(NumVertexIndices);
    UNREFERENCED_PARAMETER(PrimitiveCount);
    UNREFERENCED_PARAMETER(pIndexData);
    UNREFERENCED_PARAMETER(IndexDataFormat);
    UNREFERENCED_PARAMETER(pVertexStreamZeroData);
    UNREFERENCED_PARAMETER(VertexStreamZeroStride);

    return false;
}

/**
 * Invoked when the Direct3D device is setting a render state. (SetRenderState)
 *
 * @param {D3DRENDERSTATETYPE} state        - The render state to alter.
 * @param {DWORD} value                     - The new value for the render state.
 * @returns {bool} True if the call should be blocked, false otherwise.
 *
 * @notes
 *
 *      This will only be called if you returned true inside of Direct3DInitialize!
 *      This function is called before SetRenderState is called.
 *
 *      Plugins can block the call from happening by returning true.
 */
bool FFXIMOVE::Direct3DSetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
    UNREFERENCED_PARAMETER(State);
    UNREFERENCED_PARAMETER(Value);

    return false;
}

/**
 * Returns the interface version this plugin was compiled with.
 *
 * @returns {double} The Ashita interface version.
 */
__declspec(dllexport) double __stdcall GetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

/**
 * Creates and populates the plugins information structure.
 *
 * @returns {double} The Ashita interface version.
 */
__declspec(dllexport) void __stdcall CreatePluginInfo(plugininfo_t* info)
{
    // Store the pointer from Ashita..
    g_PluginInfo = info;

    // Populate the structure with our plugins information..
    strcpy_s(info->Author, sizeof(info->Author), "atom0s");
    strcpy_s(info->Name, sizeof(info->Name), "FFXIMOVE");
    info->InterfaceVersion = ASHITA_INTERFACE_VERSION;
    info->PluginVersion = 3.0f;
    info->Priority = 0;
}

/**
 * Creates an instance of the plugins main class.
 *
 * @returns {IPlugin*} The plugin base class instance created by this plugin.
 */
__declspec(dllexport) IPlugin* __stdcall CreatePlugin(void)
{
    return (IPlugin*)new FFXIMOVE;
}

void FFXIMOVE::SaveWaypoint(float x_pos, float z_pos, float y_pos, const char* Name)
{
    std::string install = m_AshitaCore->GetAshitaInstallPathA();
    int zone = m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0);
    char buffer[1024];
    sprintf_s(buffer, 1024, "%s\\config\\ffximove\\waypoints\\%s.xml", install.c_str(), std::to_string(zone).c_str());

    //Ensure directories exist, making them if not.
    std::string makeDirectory(buffer);
    size_t nextDirectory = makeDirectory.find("\\");
    nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
    while (nextDirectory != std::string::npos)
    {
        std::string currentDirectory = makeDirectory.substr(0, nextDirectory + 1);
        if ((!CreateDirectory(currentDirectory.c_str(), NULL)) && (ERROR_ALREADY_EXISTS != GetLastError()))
        {
            this->m_AshitaCore->GetChatManager()->Write("Could not find or create folder.");
            return;
        }
        nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
    }

    std::ofstream outstream(buffer, std::ofstream::out);
    if (!outstream.is_open())
    {
        this->m_AshitaCore->GetChatManager()->Write(buffer);
        return;
    }

    outstream << "<ffximove>\n";
    outstream << std::setprecision(6) << "\t<waypoint name=\"" << Name << "\" X=\"" << x_pos << "\" Y=\"" << y_pos \
        << "\" Z=\"" << z_pos << "\" />\n";
    outstream << "</ffximove>";
    outstream.close();
    this->m_AshitaCore->GetChatManager()->Write("Successfully wrote the file");

    /** Ashita4 code only for use with settings.h and output.h
    std::string Path = m_AshitaCore->GetAshitaInstallPathA();
    int zone = m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0);
    char buffer[1024];
    sprintf_s(buffer, 1024, "ffximove\\waypoints\\%s", std::to_string(zone));
    Path = pSettings->GetInputWritePath(buffer);
    pSettings->CreateDirectories(Path.c_str());

    std::ofstream outstream(Path.c_str());
    if (!outstream.is_open())
    {
        pOutput->error_f("Failed to write settings file.  [%s]", Path.c_str());
        return;
    }

    outstream << "<ffximove>\n";
    outstream << std::setprecision(6) << "\n\t<waypoint name=\"" << Name << "\" X=\"" << x_pos << "\" Y=\"" << y_pos \
        << "\" Z=\"" << z_pos << "\" />\n";
    outstream << "</ffximove>";
    outstream.close();
    pOutput->message_f("Wrote settings XML. [$H%s$R]", Path.c_str());
    */
}