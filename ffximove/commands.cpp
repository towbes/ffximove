#include "ffximove.h"

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
            p_Follow->DirX = 0;
            p_Follow->DirZ = 0;
            p_Follow->Autorun = 0;
            s_last_run_state = false;
        }
        //wp commands
        if (count >= 3 && args[1] == "wp") {
            //fm save <name>
            if (count >= 3 && args[2] == "save") {
                this->m_AshitaCore->GetChatManager()->Write("Save waypoint");
                uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
                float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
                float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);
                float my_pos_y = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalY(myindex);
                FFXIMOVE::SaveWaypoint(my_pos_x, my_pos_z, my_pos_y, args[2].c_str());
            }
            else if (count >= 3 && args[2] == "load") {
                //this->m_AshitaCore->GetChatManager()->Write("load waypoints");
                int zone = m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0);
                FFXIMOVE::LoadWaypoints(to_string(zone).c_str());
            }
            else if (count >= 3 && args[2] == "list") {
                this->m_AshitaCore->GetChatManager()->Writef("List size: %d", WaypointList.size());
                int i = 0;
                for (const auto& w : WaypointList) {
                    this->m_AshitaCore->GetChatManager()->Writef("%d: %s", to_string(i), w.first);
                    //this->m_AshitaCore->GetChatManager()->Writef("%d: %s(%.5f,%.5f) Y:%.5f", to_string(i), w.first, w.second.x, w.second.z, w.second.y);
                    i++;
                }
            }
        }

        else if (count >= 2 && args[1] == "loadnav") {
            this->m_AshitaCore->GetChatManager()->Write("Loading navmesh");
            std::string install = m_AshitaCore->GetAshitaInstallPathA();
            int zone = m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0);
            if (m_navMesh == nullptr)
            {
                m_navMesh = new CNavMesh(zone);
            }

            char file[1024];
            sprintf_s(file, 1024, "%s\\config\\navmeshes\\%s.nav", install.c_str(), std::to_string(zone).c_str());

            if (!m_navMesh->load(file))
            {
                this->m_AshitaCore->GetChatManager()->Write("Failed to load navmesh");
                delete m_navMesh;
                m_navMesh = nullptr;
            }
            else {
                this->m_AshitaCore->GetChatManager()->Write("Navmesh loaded");
            }
        }
        else if (count >= 2 && args[1] == "findpath") {
            this->m_AshitaCore->GetChatManager()->Write("Finding Path");

            //Get character positions
            uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
            float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
            float my_pos_y = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalY(myindex);
            float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);

            position_t startPosition;

            startPosition.x = my_pos_x;
            startPosition.y = my_pos_y;
            startPosition.z = my_pos_z;

            this->m_AshitaCore->GetChatManager()->Writef("Setting start position %.5f %.5f %.5f", startPosition.x, startPosition.y, startPosition.z);

            position_t endPosition;

            if (args[2] == "target") {
                int targindex = m_AshitaCore->GetDataManager()->GetTarget()->GetTargetIndex();

                float tar_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(targindex);
                float tar_pos_y = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalY(targindex);
                float tar_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(targindex);

                endPosition.x = tar_pos_x;
                endPosition.y = tar_pos_y;
                endPosition.z = tar_pos_z;

            }
            else {
                endPosition.x = std::stof(args[2].c_str());
                endPosition.y = std::stof(args[3].c_str());
                endPosition.z = std::stof(args[4].c_str());
            }
            



            this->m_AshitaCore->GetChatManager()->Writef("Setting end position %.5f %.5f %.5f", endPosition.x, endPosition.y, endPosition.z);


            this->m_AshitaCore->GetChatManager()->Write("Finding a path");
            m_points = m_navMesh->findPath(startPosition, endPosition);
            if (!m_points.size() > 0) {
                this->m_AshitaCore->GetChatManager()->Write("No path found");
            }
            else {
                this->m_AshitaCore->GetChatManager()->Writef("Found a path, %d", m_points.size());
                m_currentPoint = 0;
                c_run = true;
            }
        }
        // Return true here to block this command from going to the client.
        return true;
    }

    // Return false here to allow unhandled commands to continue to be processed.
    return false;
}