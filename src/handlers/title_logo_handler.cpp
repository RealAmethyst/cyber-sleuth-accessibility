// CUiTitleLogo accessibility handler — pre-title logo sequence.
//
// Announces company logos, disclaimer, and "Press Any Button" as they appear.
// All content is pre-baked textures, so text is hardcoded per language.
//
// State mapping (first state in each group triggers the announcement):
//   5: Bandai Namco, 8: Media.Vision, 11: ha.n.d., 14: Sound Prestige,
//   17: CRI Ware, 20: Disclaimer, 23: 20th Anniversary, 26: Press Any Button

#include "handlers/title_logo_handler.h"
#include "handlers/handler_utils.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// ============================================================
// Disclaimer text per language (from ui_title_logo_caution.img)
// ============================================================
// Language indices: 0=JP, 1=EN, 2=CN, 3=EN_Censor, 4=KR, 5=DE

static const char* s_disclaimerText[] = {
    // 0: JP — TODO: source from Japanese image when available
    "This story and setting of this game is a work of fiction, "
    "and has no relationship with real countries, regions, people, "
    "businesses, organizations or incidents. "
    "In addition, while part of this product takes place in existing "
    "buildings and regions, it does not imply support or backing "
    "from said existing buildings, regions, "
    "or businesses/organizations associated with "
    "the corresponding buildings. "
    "Warning: Transmission or distribution of this game software via the Internet "
    "without the consent of the end-user, or downloading the software via illegal "
    "Internet transmission is strictly prohibited by law. "
    "We appreciate your understanding and cooperation.",

    // 1: EN
    "This story and setting of this game is a work of fiction, "
    "and has no relationship with real countries, regions, people, "
    "businesses, organizations or incidents. "
    "In addition, while part of this product takes place in existing "
    "buildings and regions, it does not imply support or backing "
    "from said existing buildings, regions, "
    "or businesses/organizations associated with "
    "the corresponding buildings. "
    "Warning: Transmission or distribution of this game software via the Internet "
    "without the consent of the end-user, or downloading the software via illegal "
    "Internet transmission is strictly prohibited by law. "
    "We appreciate your understanding and cooperation.",

    // 2: CN (Traditional Chinese)
    "\u904A\u6232\u5167\u5BB9\uFF0E\u8A2D\u5B9A\u7D14\u5C6C\u865B\u69CB\uFF0C"
    "\u8207\u73FE\u5B58\u570B\u5BB6\u3001\u5730\u5340\u3001\u4EBA\u7269\u3001\u4E8B\u4EF6\u3001\u4F01\u696D\u3001\u5718\u9AD4\u7121\u95DC\u3002"
    "\u6B64\u5916\uFF0C\u4F5C\u54C1\u4E2D\u96D6\u6709\u90E8\u5206\u73FE\u5BE6\u5EFA\u7BC9\u7269\u3001\u5730\u5340\u767B\u5834\uFF0C"
    "\u4F46\u4E26\u975E\u6697\u793A\u672C\u4F5C\u9700\u8981\u8A72\u5EFA\u7BC9\u3001"
    "\u5730\u5340\u7684\u76F8\u95DC\u4F01\u696D\u3001\u5718\u9AD4\u4EE5\u53CA\u500B\u4EBA\u8D0A\u52A9\u3002"
    "\u6CE8\u610F\uFF1A\u672A\u7372\u5F97\u904A\u6232\u8EDF\u9AD4\u7248\u6B0A\u6301\u6709\u8005\u7684\u540C\u610F\uFF0C"
    "\u64C5\u81EA\u6563\u4F48\u3001\u50B3\u64AD\u6216\u9055\u6CD5\u9032\u884C\u4E0B\u8F09\u7B49\u884C\u70BA\uFF0C"
    "\u7686\u53D7\u5230\u6CD5\u5F8B\u56B4\u683C\u7981\u6B62\u3002"
    "\u9084\u671B\u5404\u4F4D\u8AD2\u89E3\u4E26\u914D\u5408\u3002",

    // 3: EN_Censor (same as EN)
    "This story and setting of this game is a work of fiction, "
    "and has no relationship with real countries, regions, people, "
    "businesses, organizations or incidents. "
    "In addition, while part of this product takes place in existing "
    "buildings and regions, it does not imply support or backing "
    "from said existing buildings, regions, "
    "or businesses/organizations associated with "
    "the corresponding buildings. "
    "Warning: Transmission or distribution of this game software via the Internet "
    "without the consent of the end-user, or downloading the software via illegal "
    "Internet transmission is strictly prohibited by law. "
    "We appreciate your understanding and cooperation.",

    // 4: KR (Korean)
    "\uC774 \uAC8C\uC784\uC758 \uC2DC\uB098\uB9AC\uC624\uB9AC\uC624\u30FB\uC124\uC815\uC740 \uD53D\uC158\uC774\uBA70, "
    "\uD604\uC2E4\uC758 \uB098\uB77C, \uC9C0\uC5ED, \uC778\uBB3C, \uAE30\uC5C5\u30FB\uB2E8\uCCB4, \uC0AC\uAC74\uACFC\uB294 \uAD00\uACC4\uAC00 \uC5C6\uC2B5\uB2C8\uB2E4. "
    "\uB610\uD55C, \uC774 \uC791\uD488\uC5D0\uC11C\uB294 \uC77C\uBD80 \uD604\uC2E4\uC5D0 \uC874\uC7AC\uD558\uB294 \uAC74\uCD95\uBB3C\uACFC \uC9C0\uC5ED\uC774 "
    "\uB4F1\uC7A5\uD569\uB2C8\uB2E4\uB9CC, \uC2E4\uC874\uD558\uB294 \uAC74\uCD95\uBB3C, \uC9C0\uC5ED, \uD574\uB2F9 \uAC74\uCD95\uBB3C\uC5D0 \uAD00\uB828\uB41C "
    "\uAE30\uC5C5\u30FB\uB2E8\uCCB4 \uADF8\uB9AC\uACE0 \uAC1C\uC778\uC758 \uC9C0\uC9C0\uB098 \uD6C4\uC6D0\uACFC\uB3C4 \uAD00\uB828\uB418\uB294 \uBC14\uAC00 \uC5C6\uC2B5\uB2C8\uB2E4. "
    "\uC8FC\uC758\uC0AC\uD56D: \uAC8C\uC784 \uC18C\uD504\uD2B8\uB97C \uC774\uC6A9\uC790\uC758 \uD5C8\uAC00 \uC5C6\uC774 "
    "\uC778\uD130\uB137\uC744 \uD1B5\uD558\uC5EC \uBC30\uD3EC\uD558\uB294 \uD589\uC704, \uB610\uB294 "
    "\uC704\uBC95\uD558\uAC8C \uBC30\uD3EC\uB418\uB294 \uB370\uC774\uD130\uC784\uC744 \uC54C\uBA74\uC11C\uB3C4 "
    "\uB2E4\uC6B4\uB85C\uB4DC\uD558\uB294 \uD589\uC704\uB294 \uBC95\uB960\uB85C \uAE08\uC9C0\uB418\uC5B4 "
    "\uC788\uC2B5\uB2C8\uB2E4. "
    "\uC5EC\uB7EC\uBD84\uC758 \uC774\uD574\uC640 \uD611\uB825\uC744 \uBD80\uD0C1\uB4DC\uB9BD\uB2C8\uB2E4.",

    // 5: DE (German)
    "Diese Geschichte und der Schauplatz des Spiels sind fiktional "
    "und haben keinen Bezug zu echten L\u00e4ndern, Regionen, Personen, "
    "Unternehmen, Organisationen oder Geschehnissen. "
    "Auch wenn ein Teil dieses Produkts "
    "in existierenden Geb\u00e4uden und Regionen stattfindet, "
    "impliziert dies keine Unterst\u00fctzung von besagten existierenden "
    "Geb\u00e4uden, Regionen oder Unternehmen/Organisationen, "
    "die mit den entsprechenden Geb\u00e4uden in Verbindung stehen. "
    "Warnung: Das \u00dcbermitteln oder der Vertrieb dieser "
    "Spielsoftware \u00fcber das Internet ohne "
    "Einverst\u00e4ndnis des Endbenutzers oder "
    "der Download der Software per illegaler "
    "\u00dcbertragung im Internet ist gesetzlich "
    "streng verboten. Wir wissen dein "
    "Verst\u00e4ndnis und deine Kooperation "
    "zu sch\u00e4tzen.",
};

static constexpr int LANGUAGE_COUNT = 6;

// ============================================================
// Singleton
// ============================================================

TitleLogoHandler* TitleLogoHandler::Get()
{
    static TitleLogoHandler instance;
    return &instance;
}

uintptr_t TitleLogoHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiTitleLogo_Tick;
}

void TitleLogoHandler::OnScreenClosed()
{
    m_lastState = 0xFFFFFFFF;
}

// ============================================================
// State reading
// ============================================================

uint32_t TitleLogoHandler::ReadState(void* thisPtr)
{
    return HandlerUtils::ReadMemory<uint32_t>(thisPtr, Offsets::TitleLogo::STATE);
}

// ============================================================
// State -> announcement mapping
// ============================================================

const char* TitleLogoHandler::GetAnnouncementForState(uint32_t state)
{
    // Each logo uses 3 states: N=setup, N+1=fade-in (visible), N+2=hold.
    // We trigger on N+1 when the content is actually on screen.
    switch (state) {
        case 6:  return "Bandai Namco Entertainment";
        case 9:  return "Media.Vision";
        case 12: return "ha.n.d.";
        case 15: return "Sound Prestige";
        case 18: return "CRIWARE";
        case 24: return "Digimon 20th Adventure";
        // 21 (disclaimer) and 27 (press any button) are language-dependent
        // and handled separately in OnFrameInner
        default: return nullptr;
    }
}

// ============================================================
// OnFrameInner — per-frame logic
// ============================================================

void TitleLogoHandler::OnFrameInner(void* thisPtr)
{
    uint32_t state = ReadState(thisPtr);

    if (state == m_lastState)
        return;

    uint32_t prevState = m_lastState;
    m_lastState = state;

    // Language-dependent screens
    if (state == 21) {
        int lang = GameText_GetLanguage();
        if (lang < 0 || lang >= LANGUAGE_COUNT) lang = 1;  // fallback to EN
        Logger_Log("TitleLogo", "Disclaimer (state %u, lang %d)", state, lang);
        SpeechManager::Get()->Speak(s_disclaimerText[lang], true);
        return;
    }

    // Company logos (same in all languages)
    const char* text = GetAnnouncementForState(state);
    if (text) {
        Logger_Log("TitleLogo", "Logo: %s (state %u)", text, state);
        SpeechManager::Get()->Speak(text, true);
    }
}
