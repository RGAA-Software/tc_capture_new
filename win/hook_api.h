//
// Created by hy on 2024/3/1.
//

#ifndef TC_APPLICATION_HOOK_API_H
#define TC_APPLICATION_HOOK_API_H

#include <Windows.h>

// hook api

typedef UINT (WINAPI *GetRawInputData_t)(
        HRAWINPUT hRawInput,
        UINT uiCommand,
        LPVOID pData,
        PUINT pcbSize,
        UINT cbSizeHeader);

typedef UINT (WINAPI *GetRawInputBuffer_t)(
        PRAWINPUT pData,
        PUINT pcbSize,
        UINT cbSizeHeader);

typedef BOOL(WINAPI *PostMessageA_t)(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

typedef	BOOL(WINAPI *PostMessageW_t)(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

typedef LRESULT(WINAPI *SendMessageA_t)(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

typedef LRESULT(WINAPI* SendMessageW_t)(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

typedef BOOL (WINAPI* GetCursorPos_t)(LPPOINT lpPoint);
typedef BOOL (WINAPI* SetCursorPos_t)(int X, int Y);

// hook api

#endif //TC_APPLICATION_HOOK_API_H
