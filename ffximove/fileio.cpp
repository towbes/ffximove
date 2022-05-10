#include "ffximove.h"
using namespace rapidxml;

void FFXIMOVE::LoadWaypoints(const char* Zonefile)
{
    char buffer[1024];
    sprintf_s(buffer, 1024, "waypoints\\%s", Zonefile);
    std::string ZonefilePath = pSettings->GetInputSettingsPath(buffer);

    if (ZonefilePath == "FILE_NOT_FOUND")
    {
        pOutput->error_f("Could not find profile.  [%s]", Zonefile);
        //LoadDefaultProfile(true);
        return;
    }

    //Reset the map
    WaypointList.clear();

    //Load profile.
    char* bigbuffer = NULL;
    xml_document<>* XMLReader = pSettings->LoadXml(ZonefilePath, bigbuffer);
    if (XMLReader == NULL)
    {
        pOutput->error_f("Could not load profile.  [%s]", ZonefilePath.c_str());
        return;
    }

    xml_node<>* Node = XMLReader->first_node("ffximove");
    if (!Node)
    {
        //not using state
        //mState.CurrentProfile = "NO_FILE";
        delete XMLReader;
        delete[] bigbuffer;
        pOutput->error_f("Profile did not have a ffximove node at root level.  [%s]", ZonefilePath.c_str());
        return;
    }

    //for (Node = Node->first_node(); Node; Node = Node->next_sibling())
    //{
        for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
        {
            if (_stricmp(SubNode->name(), "waypoint") == 0)
            {
                xml_attribute<>* nameAttr = SubNode->first_attribute("name");
                xml_attribute<>* xpos = SubNode->first_attribute("X");
                xml_attribute<>* ypos = SubNode->first_attribute("Y");
                xml_attribute<>* zpos = SubNode->first_attribute("Z");

                //create a new position_t
                position_t temp;
                //value is a string so convert to a float
                temp.x = std::stof(xpos->value());
                temp.y = std::stof(ypos->value());
                temp.z = std::stof(zpos->value());

                //add the id, name and position to the map
                WaypointList[nameAttr->value()] = temp;
            }
        }
    //}

    //mState.CurrentProfile = ProfilePath;
    pOutput->message_f("Loaded profile. [%s] %d", ZonefilePath.c_str(), WaypointList.size());
    delete XMLReader;
    delete bigbuffer;
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