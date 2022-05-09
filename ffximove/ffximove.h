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

#ifndef __ASHITA_FFXIMOVE_H_INCLUDED__
#define __ASHITA_FFXIMOVE_H_INCLUDED__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/**
 * Main Ashita ADK Header
 *
 * This is the main header to include while creating Ashita v3 plugins. This header includes
 * various things that are useful to plugins as well as includes all the other ADK files in
 * a single header. Update this path to match where you have Ashita v3 installed to.
 */
#include "Ashita.h"
//From thorny's common headers
//https://github.com/ThornyFFXI/common/tree/master/thirdparty
#include "thirdparty/rapidxml.hpp"
#include "navmesh.h"
//#include "Settings.h"
//#include "Output.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <psapi.h>
#include <stdint.h>


/**
 * Plugin Information
 *
 * This object holds information specific to this plugin. The CreatePluginInfo exported function
 * uses this object to create information about the plugin which is used internally with Ashita.
 *
 * Please note: Ashita maintains the pointer of this object! Your plugin should never try to delete
 *              this object manually!
 */
extern plugininfo_t* g_PluginInfo;


/**
   Structs from multisend code
   https://git.ashitaxi.com/Plugins/MultiSend/src/branch/master/src/MultiSend/MultiSend.h
*/
struct sFollow
{ // 42 bytes
    unsigned int    _unknown_0_4;       //  0- 3    Looks like a pointer
    unsigned short  TargetIndex;        //  4- 5+2
    unsigned int    TargetID;           //  8-11
    float           DirX;               // 12-15
    float           DirY;               // 16-19
    float           DirZ;               // 20-23
    unsigned char   _unknown_24_27[4];  // 24-27
    unsigned int    _unknown_28_31;     // 28-31    Same as _unknown_0_4
    unsigned int    FollowIndex;        // 32-33+2
    unsigned int    FollowID;           // 36-39    Once set will overwrite DirX, DirZ and DirY with directional values
    unsigned char   _zero_40_40;        // 40-40
    unsigned char   Autorun;            // 41-41
};

/**
 * Main Plugin Class Instance
 *
 * The main class the plugin will expose back to Ashita when loaded.
 * This class must inherit from IPlugin in order to be valid!
 *
 * See FFXIMOVE.cpp for more information about the functions in this class.
 */
class FFXIMOVE : IPlugin
{
private:
    IAshitaCore*        m_AshitaCore;
    ILogManager*        m_LogManager;
    uint32_t            m_PluginId;
    IDirect3DDevice8*   m_Direct3DDevice;

    //Settings helper ashita4 only
    //SettingsHelper* pSettings;
    //OutputHelpers* pOutput;

    //Multisend follow struct
    //https://git.ashitaxi.com/Plugins/MultiSend/src/branch/master/src/MultiSend/MultiSend.h
    sFollow* p_Follow;

    //Navmesh
    CNavMesh* m_navMesh; // zones navmesh for finding paths

    //State tracking
    std::string			s_name;
    uint32_t			s_position;
    bool				s_last_run_state;
    float				s_vector_x;
    float				s_vector_z;
    float               s_tarpos_x;
    float               s_tarpos_z;


    //Configuration
    float				c_maxdist;
    volatile bool		c_attemptzone;
    volatile bool		c_run;
    volatile bool		c_debug;
    volatile bool		c_safemode;


public:
    // Constructor and Deconstructor
    FFXIMOVE(void);
    ~FFXIMOVE(void);

public:
    // Plugin Information Callback
    plugininfo_t GetPluginInfo(void);

public:
    // Main Initialization and Cleanup Callbacks
    bool Initialize(IAshitaCore* core, ILogManager* log, uint32_t id) override;
    void Release(void) override;

    // Chat Manager Callbacks
    bool HandleCommand(const char* command, int32_t type) override;
    bool HandleIncomingText(int16_t mode, const char* message, int16_t* modifiedMode, char* modifiedMessage, bool blocked) override;
    bool HandleOutgoingText(int32_t type, const char* message, int32_t* modifiedType, char* modifiedMessage, bool blocked) override;

    // Packet Manager Callbacks
    bool HandleIncomingPacket(uint16_t id, uint32_t size, void* data, void* modified, bool blocked) override;
    bool HandleOutgoingPacket(uint16_t id, uint32_t size, void* data, void* modified, bool blocked) override;

    // Direct3D Related Callbacks
    bool Direct3DInitialize(IDirect3DDevice8* device) override;
    void Direct3DRelease(void) override;
    void Direct3DPreRender(void) override;
    void Direct3DRender(void) override;
    bool Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) override;
    bool Direct3DDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
    bool Direct3DDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
    bool Direct3DDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
    bool Direct3DDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
    bool Direct3DSetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;

    //fileio.cpp
    //https://github.com/ThornyFFXI/Lootwhore/blob/6cad6d55c9a0e5c7bbb378404d196de8a9e67aff/Lootwhore.h
    void SaveWaypoint(float x_pos, float z_pos, float y_pos, const char* Name);

};

#endif // __ASHITA_FFXIMOVE_H_INCLUDED__