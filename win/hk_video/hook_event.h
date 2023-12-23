#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winuser.h>
#include <dxgi1_4.h>

#include <queue>
#include <memory>

#include "easyhook/easyhook.h"
#include "hk_utils/hook_api.hpp"

constexpr auto kInvalidCursorPos = -1020304050;

#pragma comment(lib, "User32.lib")

namespace tc
{

    class IPCBaseMessage;
    class IPCMouseMessage;
    class IPCKeyboardMessage;

    typedef UINT (WINAPI *FUNC_GetRawInputData)(
            HRAWINPUT hRawInput,
            UINT uiCommand,
            LPVOID pData,
            PUINT pcbSize,
            UINT cbSizeHeader);

    typedef UINT (WINAPI *FUNC_GetRawInputBuffer)(
            PRAWINPUT pData,
            PUINT pcbSize,
            UINT cbSizeHeader);

    typedef BOOL(WINAPI *FUNC_PostMessageA)(
            HWND hWnd,
            UINT Msg,
            WPARAM wParam,
            LPARAM lParam);

    typedef BOOL(WINAPI *FUNC_PostMessageW)(
            HWND hWnd,
            UINT Msg,
            WPARAM wParam,
            LPARAM lParam);

    typedef LRESULT(WINAPI *FUNC_SendMessageA)(
            HWND hWnd,
            UINT Msg,
            WPARAM wParam,
            LPARAM lParam);

    typedef LRESULT(WINAPI *FUNC_SendMessageW)(
            HWND hWnd,
            UINT Msg,
            WPARAM wParam,
            LPARAM lParam);

    typedef BOOL (WINAPI *FUNC_GetCursorPos)(LPPOINT lpPoint);

    typedef BOOL (WINAPI *FUNC_SetCursorPos)(int X, int Y);

    class HookEvent {
    public:

        static HookEvent *Instance() {
            static HookEvent ins;
            return &ins;
        }

        HookEvent();

        ~HookEvent();

        void HookGetRawInputBuffer();

        UINT ProcessHookedGetRawInputData(
                HRAWINPUT hRawInput,
                UINT uiCommand,
                LPVOID pData,
                PUINT pcbSize,
                UINT cbSizeHeader);

        BOOL ProcessHookedGetCursorPos(LPPOINT lpPoint);

        void Init();

        void ReceiveIPCMessages();

        void ProcessMouseMessage(std::shared_ptr<IPCMouseMessage> message);

        void ProcessKeyboardMessage(std::shared_ptr<IPCKeyboardMessage> message);

    private:

        tc::HookApi api_HookGetRawInputBuffer;
        tc::HookApi api_HookGetRawInputData;
        tc::HookApi api_HookPostMessageA;
        tc::HookApi api_HookPostMessageW;
        tc::HookApi api_HookSendMessageA;
        tc::HookApi api_HookSendMessageW;
        tc::HookApi api_HookGetCursorPos;
        tc::HookApi api_HookSetCursorPos;

        // messages
        std::queue<std::shared_ptr<IPCBaseMessage>> messages;

        POINT cursor_position{kInvalidCursorPos, kInvalidCursorPos};
    };

    static FUNC_GetRawInputBuffer origin_GetRawInputBuffer;
    static FUNC_GetRawInputData origin_GetRawInputData;
    static FUNC_PostMessageA origin_PostMessageA;
    static FUNC_PostMessageW origin_PostMessageW;
    static FUNC_SendMessageA origin_SendMessageA;
    static FUNC_SendMessageW origin_SendMessageW;
    static FUNC_GetCursorPos origin_GetCursorPos;
    static FUNC_SetCursorPos origin_SetCursorPos;
}