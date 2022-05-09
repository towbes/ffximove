#include "ffximove.h"
using namespace rapidxml;

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