#include "ffximove.h"

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
        //this->m_AshitaCore->GetChatManager()->Write("Trying to run");
        uint16_t myindex = m_AshitaCore->GetDataManager()->GetParty()->GetMemberTargetIndex(0);
        float my_pos_x = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalX(myindex);
        float my_pos_z = m_AshitaCore->GetDataManager()->GetEntity()->GetLocalZ(myindex);


        //if there are points in m_points, follow them
        if (m_points.size() > 0 && m_currentPoint < m_points.size()) {
            //this->m_AshitaCore->GetChatManager()->Write("Setting m_point");
            s_tarpos_x = m_points[m_currentPoint].x;
            s_tarpos_z = m_points[m_currentPoint].z;
        }
        else {
            c_run = false;
            //this->m_AshitaCore->GetChatManager()->Write("Done running");
            p_Follow->DirX = 0;
            p_Follow->DirZ = 0;
            p_Follow->Autorun = 0;
            s_last_run_state = false;
        }

        s_vector_x = s_tarpos_x - my_pos_x;
        s_vector_z = s_tarpos_z - my_pos_z;

        double distance = sqrt(pow(s_vector_x, 2) + pow(s_vector_z, 2));

        if ((distance > c_tolerance) && (distance < c_maxdist))
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
            m_currentPoint++;
        }
    }
}