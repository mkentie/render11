#pragma once

#define PREAMBLE PROJECTNAME L": "

//Don't use GError because that popup window calls Exit(), meaning the log won't be written
#define LOGMESSAGE(str) GLog->Log(PREAMBLE str);
#define LOGMESSAGEF(str, ...) GLog->Logf(PREAMBLE str, __VA_ARGS__);
#define LOGWARNING(str) GWarn->Log(PREAMBLE str);
#define LOGWARNINGF(str, ...) GWarn->Logf(PREAMBLE str, __VA_ARGS__);



#ifdef _DEBUG
template<class c> void SetResourceName(const ComPtr<c>& pResource, const char* const pszName)
{
    assert(pResource);
    pResource->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(pszName), pszName);
}

template<class c> void SetResourceNameW(const ComPtr<c>& pResource, const wchar_t* const pszName)
{
    assert(pResource);
    char szBuf[1024];
    sprintf_s(szBuf, _countof(szBuf), "%S", pszName);
    pResource->SetPrivateData(WKPDID_D3DDebugObjectName, 1024, szBuf);
}
#else
template<class c> void SetResourceName(const ComPtr<c>&, const char* const){}
template<class c> void SetResourceNameW(const ComPtr<c>&, const wchar_t* const){}
#endif

//Exception with wchar_t message buffer
class WException : public std::exception
{
public:
    explicit WException(const wchar_t* const pszText)
    {
        wcscpy_s(m_szBuf, pszText);
    }

    const wchar_t* wwhat() const { return m_szBuf; }
protected:
    wchar_t m_szBuf[1024];
};

#define ThrowIfFail(hResult, pszMsg, ...) ThrowIfFailImpl(hResult, __FUNCTIONW__, pszMsg, __VA_ARGS__)

inline HRESULT ThrowIfFailImpl(const HRESULT hResult, const wchar_t* const pszFunction, const wchar_t* const pszMsg, ...)
{
    if (FAILED(hResult))
    {
        //Expand variadic params
        va_list ap;
        va_start(ap, pszMsg);
        wchar_t szBuf1[1024];
        vswprintf_s(szBuf1, pszMsg, ap);

        const _com_error Err(hResult);
        wchar_t szBuf2[1024];
        swprintf_s(szBuf2, L"Error in %s: %s (%x - %s)", pszFunction, szBuf1, Err.Error(), Err.ErrorMessage());
        throw WException(szBuf2);
    }

    return hResult;
}

//Operator new/delete for SSE aligned data
class XMMAligned
{
public:
    void* operator new(const size_t s)
    {
        return _aligned_malloc(s, std::alignment_of<__m128>::value);
    }

        void operator delete(void* const p)
    {
        _aligned_free(p);
    }
};
