#pragma once 
#include <map>
#include "Windows.h"

class VirtualMethodTableHooker
{
private:
    std::map<DWORD, DWORD*> hooks;
    DWORD** vmt;
public:

    VirtualMethodTableHooker()
    {
        vmt = nullptr;
    }

    void SetVmt(void* vmtToSet)
    {
        vmt = reinterpret_cast<DWORD**>(vmtToSet);
    }

    template<class Fn>
    Fn Hook(DWORD index, Fn hook)
    {
        if (!vmt || index > GetNumberOfFunctions() || index < 0)
            return nullptr;

        auto original = vmt[index];

        DWORD old;
        VirtualProtect(vmt, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old);
        vmt[index] = reinterpret_cast<DWORD*>(hook);
        VirtualProtect(vmt, sizeof(DWORD), old, &old);

        hooks.insert(std::make_pair(index, original));

        return reinterpret_cast<Fn>(original);
    }

    void Unhook(DWORD index)
    {
        auto it = hooks.find(index);
        if (it != hooks.end())
        {
            auto original = static_cast<DWORD*>(it->second);

            DWORD old;
            VirtualProtect(vmt, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old);
            vmt[index] = reinterpret_cast<DWORD*>(original);
            VirtualProtect(vmt, sizeof(DWORD), old, &old);

            hooks.erase(it);
        }
    }

    void UnhookAll()
    {
        for (auto it = hooks.begin(); it != hooks.end();)
        {
            auto original = it->second;

            DWORD old;
            VirtualProtect(vmt, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old);
            vmt[it->first] = original;
            VirtualProtect(vmt, sizeof(DWORD), old, &old);

            hooks.erase(it++);
        }
    }

    DWORD GetNumberOfFunctions() {
        if (!vmt)
            return 0;
        DWORD index;
        for (index = 0; vmt[index]; index++)
        {
            if (IsBadCodePtr(reinterpret_cast<FARPROC>(vmt[index])))
                break;
        }
        return index;
    }

    ~VirtualMethodTableHooker()
    {
        UnhookAll();
    }
};