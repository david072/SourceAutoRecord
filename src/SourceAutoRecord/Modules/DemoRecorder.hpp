#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Timer.hpp"

#include "Cheats.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Engine {

bool* m_bLoadgame;

namespace DemoRecorder {

    VMT s_ClientDemoRecorder;

    using _GetRecordingTick = int(__CALL*)(void* thisptr);
    _GetRecordingTick GetRecordingTick;

    char* m_szDemoBaseName;
    int* m_nDemoNumber;
    bool* m_bRecording;

    std::string CurrentDemo;
    bool IsRecordingDemo;

    int GetTick()
    {
        return GetRecordingTick(s_ClientDemoRecorder->GetThisPtr());
    }

    // CDemoRecorder::SetSignonState
    DETOUR(SetSignonState, int state)
    {
        if (state == SignonState::Full && *m_bRecording) {
            IsRecordingDemo = true;
            CurrentDemo = std::string(m_szDemoBaseName);
            if (*m_nDemoNumber > 1) {
                CurrentDemo += std::string("_") + std::to_string(*m_nDemoNumber);
            }
        }
        return Original::SetSignonState(thisptr, state);
    }

    // CDemoRecorder::StopRecording
    DETOUR(StopRecording)
    {
        const int LastDemoNumber = *m_nDemoNumber;

        // This function does:
        //   m_bRecording = false
        //   m_nDemoNumber = 0
        auto result = Original::StopRecording(thisptr);

        if (IsRecordingDemo && sar_autorecord.GetBool()) {
            *m_nDemoNumber = LastDemoNumber;

            // Tell recorder to keep recording
            if (*m_bLoadgame) {
                *m_bRecording = true;
                (*m_nDemoNumber)++;
                Console::DevMsg("SAR: Recording!");
            }
        } else {
            IsRecordingDemo = false;
        }

        return result;
    }

    void Hook(void* demorecorder)
    {
        if (SAR::NewVMT(demorecorder, s_ClientDemoRecorder)) {
            HOOK(s_ClientDemoRecorder, SetSignonState);
            HOOK(s_ClientDemoRecorder, StopRecording);

            GetRecordingTick = s_ClientDemoRecorder->GetOriginalFunction<_GetRecordingTick>(Offsets::GetRecordingTick);
            m_szDemoBaseName = reinterpret_cast<char*>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
            m_nDemoNumber = reinterpret_cast<int*>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
            m_bRecording = reinterpret_cast<bool*>((uintptr_t)demorecorder + Offsets::m_bRecording);
        }
    }
    void Unhook()
    {
        UNHOOK(s_ClientDemoRecorder, SetSignonState);
        UNHOOK(s_ClientDemoRecorder, StopRecording);
        SAR::DeleteVMT(s_ClientDemoRecorder);
    }
}
}