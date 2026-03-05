// CUiFirstSequence accessibility handler — new game naming/gender screen.
//
// This class manages the entire new-game flow in Cyber Sleuth:
//   - Gender selection (Male/Female characters side by side)
//   - Name entry form (Gender field, Name field, Log In button)
//
// 31-state machine at this+0x108:
//   State 10: Gender selection interactive (Male/Female picker)
//   State 6:  Login form interactive (cursor at 0x160: 0=gender, 1=name, 2=login)
//   State 12-13: Name entry (OS keyboard active)
//   State 25-26: Login confirmation dialog

#include "handlers/player_setting_handler.h"
#include "handlers/handler_utils.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// CUiFirstSequence offsets
static constexpr uintptr_t STATE_OFFSET = 0x108;
static constexpr uintptr_t GENDER_OFFSET = 0x164;  // int32: 1=male, 2=female
static constexpr uintptr_t CURSOR_OFFSET = 0x160;  // int32: 0=gender, 1=name, 2=login

// Interactive states
static constexpr int STATE_GENDER_SELECT = 10;
static constexpr int STATE_LOGIN_FORM = 6;

// common_message row IDs
static constexpr int GENDER_TEXT_BASE = 1499;  // gender value 1 → ID 1500 "Male", 2 → ID 1501 "Female"

PlayerSettingHandler* PlayerSettingHandler::Get()
{
    static PlayerSettingHandler instance;
    return &instance;
}

uintptr_t PlayerSettingHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiFirstSequence_Tick;
}

void PlayerSettingHandler::OnScreenClosed()
{
    if (m_screenOpened) {
        Logger_Log("FirstSeq", "Screen closed (tick stopped)");
    }
    m_lastState = -1;
    m_lastGender = -1;
    m_lastCursor = -1;
    m_screenOpened = false;
}

static std::string GetGenderText(int gender)
{
    std::string text = GameText_Lookup("common_message", GENDER_TEXT_BASE + gender);
    if (text.empty()) {
        text = (gender == 1) ? "Male" : "Female";
    }
    return text;
}

// Login form field labels: Gender, Name, Log In
// common_message 1503 = "Gender", 1504 = "Name", 1505 = "Log In" (need to verify)
// For now, use hardcoded fallbacks with GameText attempts
static std::string GetFieldLabel(int cursor)
{
    // Try common_message IDs around the gender area
    // 1503, 1504, 1505 are likely candidates for field labels
    static constexpr int FIELD_LABEL_BASE = 1503;
    std::string text = GameText_Lookup("common_message", FIELD_LABEL_BASE + cursor);
    if (!text.empty()) return text;

    // Fallback
    switch (cursor) {
    case 0: return "Gender";
    case 1: return "Name";
    case 2: return "Log In";
    default: return "";
    }
}

void PlayerSettingHandler::OnFrameInner(void* thisPtr)
{
    int32_t state = HandlerUtils::ReadMemory<int32_t>(thisPtr, STATE_OFFSET);
    int32_t gender = HandlerUtils::ReadMemory<int32_t>(thisPtr, GENDER_OFFSET);
    int32_t cursor = HandlerUtils::ReadMemory<int32_t>(thisPtr, CURSOR_OFFSET);

    if (!m_screenOpened && state > 0) {
        Logger_Log("FirstSeq", "Screen active, initial state=%d gender=%d", state, gender);
        m_screenOpened = true;
    }

    // State transitions
    if (state != m_lastState) {
        Logger_Log("FirstSeq", "State %d -> %d (gender=%d cursor=%d)", m_lastState, state, gender, cursor);

        // Announce screen context on entering interactive states
        if (state == STATE_GENDER_SELECT) {
            // Entering gender selection — announce screen + current gender
            std::string genderText = GetGenderText(gender);
            SpeechManager::Get()->Speak("Select Gender", true);
            SpeechManager::Get()->Speak(genderText, false);
            Logger_Log("FirstSeq", "Gender selection screen: %s", genderText.c_str());
            m_lastGender = gender;
        }
        else if (state == STATE_LOGIN_FORM) {
            // Entering login form — announce current field
            std::string fieldLabel = GetFieldLabel(cursor);
            if (cursor == 0) {
                // Gender field — include current value
                std::string genderText = GetGenderText(gender);
                fieldLabel += ": " + genderText;
            }
            SpeechManager::Get()->Speak(fieldLabel, true);
            Logger_Log("FirstSeq", "Login form, field: %s (cursor=%d)", fieldLabel.c_str(), cursor);
            m_lastCursor = cursor;
        }

        m_lastState = state;
    }

    // Track gender changes during gender selection (state 10)
    if (state == STATE_GENDER_SELECT && gender != m_lastGender && m_lastGender >= 0) {
        std::string genderText = GetGenderText(gender);
        Logger_Log("FirstSeq", "Gender changed: %d -> %d (%s)", m_lastGender, gender, genderText.c_str());
        SpeechManager::Get()->Speak(genderText, true);
    }
    m_lastGender = gender;

    // Track cursor changes on login form (state 6)
    if (state == STATE_LOGIN_FORM && cursor != m_lastCursor && m_lastCursor >= 0) {
        std::string fieldLabel = GetFieldLabel(cursor);
        if (cursor == 0) {
            // Gender field — include current value
            std::string genderText = GetGenderText(gender);
            fieldLabel += ": " + genderText;
        }
        Logger_Log("FirstSeq", "Cursor moved: %d -> %d (%s)", m_lastCursor, cursor, fieldLabel.c_str());
        SpeechManager::Get()->Speak(fieldLabel, true);
    }
    m_lastCursor = cursor;
}
