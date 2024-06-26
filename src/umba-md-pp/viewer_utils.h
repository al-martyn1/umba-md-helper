#pragma once

#include "app_config.h"
#include "document.h"

//
#include "umba/umba.h"
#include "umba/filename.h"
#include "umba/filesys.h"
//
#include <string>

    


//std::size_t h1 = std::hash<std::string>{}(s.first_name);

template<typename StringType>
StringType makeFileNameWithReducedPath(const StringType &inputFilename, const std::size_t numPathLevels=2u)
{
    StringType tmpInputFileName;
    tmpInputFileName = umba::filename::getFileName(inputFilename);
    StringType tmpPath = umba::filename::getPath(inputFilename);
    for(std::size_t i=0u; i!=numPathLevels; ++i)
    {
        auto tmpPathPart = umba::filename::getFileName(tmpPath);
        if (tmpPathPart.empty() || tmpPathPart==tmpPath)
            break;
        tmpPath = umba::filename::getPath(tmpPath);
        tmpInputFileName = umba::filename::appendPath(tmpPathPart, tmpInputFileName);
    }

    return tmpInputFileName;
}

//----------------------------------------------------------------------------
inline
std::string generateTempSubfolderNameByInputFileName(const std::string &name)
{
    std::size_t h = std::hash<std::string>{}(name);

    std::string nameNoPath = makeFileNameWithReducedPath(name);
    //umba::filename::getFileName(name);
    for(auto &ch: nameNoPath)
    {
        if (ch=='.' || ch=='\\' || ch=='/' || (ch>=0 && ch<=' '))
            ch = '_';
    }

    return std::to_string(h) + "_" + nameNoPath;
}

//----------------------------------------------------------------------------
inline
std::wstring generateTempSubfolderNameByInputFileName(const std::wstring &name)
{
    std::size_t h = std::hash<std::wstring>{}(name);

    std::wstring nameNoPath = makeFileNameWithReducedPath(name);
    //umba::filename::getFileName(name);
    for(auto &ch: nameNoPath)
    {
        if (ch==L'.' || ch==L'\\' || ch==L'/' || (ch>=0 && ch<=L' '))
            ch = L'_';
    }

    return std::to_wstring(h) + L"_" + nameNoPath;
}

//----------------------------------------------------------------------------
template<typename StringType> inline
bool createTempFolder(StringType &finalPath, const StringType &inputFileName, const StringType &appName=umba::string_plus::make_string<StringType>("umba-md-pp-view"))
{
    StringType tempRoot = umba::filesys::getTempFolderPath<StringType>();
    umba::filesys::createDirectory(tempRoot);

    StringType umbaMdPpViewerTempRoot = umba::filename::appendPath(tempRoot, umba::string_plus::make_string<StringType>(".") + appName);
    umba::filesys::createDirectory(umbaMdPpViewerTempRoot);

    StringType generatedSimpleFolderName = generateTempSubfolderNameByInputFileName(inputFileName);
    StringType curFileTempRoot = umba::filename::appendPath(umbaMdPpViewerTempRoot, generatedSimpleFolderName);
    finalPath = curFileTempRoot;

    if (!umba::filesys::createDirectory(curFileTempRoot) && !umba::filesys::isLastErrorAlreadyExists())
    {
        return false;
    }

    if (umba::filesys::isPathDirectory(curFileTempRoot))
    {
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
inline
marty_cpp::ELinefeedType getConfigsLinefeed()
{
    #if defined(WIN32) || defined(_WIN32)
        return marty_cpp::ELinefeedType::crlf;
    #else
        return marty_cpp::ELinefeedType::lf;
    #endif
}

//----------------------------------------------------------------------------
/*
PROJECT_BRIEF          = ""
# OUTPUT_LANGUAGE        = English
OUTPUT_LANGUAGE        = Russian
INPUT                  = Имя файла или каталога

EXCLUDE                =
EXCLUDE_PATTERNS       =
OUTPUT_DIRECTORY       = doxy

// Вроде надо бы, но хз, что сюда добавить
IMAGE_PATH             =

COMPACT_RTF            = NO

RTF_STYLESHEET_FILE    =

AUTOLINK_SUPPORT       = YES

BRIEF_MEMBER_DESC      = YES
REPEAT_BRIEF           = YES
ALWAYS_DETAILED_SEC    = YES
FULL_PATH_NAMES        = YES
STRIP_FROM_PATH        =
STRIP_FROM_INC_PATH    =
SHORT_NAMES            = NO
JAVADOC_AUTOBRIEF      = NO
QT_AUTOBRIEF           = NO
INHERIT_DOCS           = YES
SEPARATE_MEMBER_PAGES  = NO
IDL_PROPERTY_SUPPORT   = YES
EXTRACT_ALL            = YES

*/
template<typename FilenameStringType> inline
std::string generateDoxyfile(const AppConfig<FilenameStringType> &appCfg, const Document &doc)
{
    std::vector<std::string> lines;

    lines.emplace_back("DOXYFILE_ENCODING      = UTF-8");
    lines.emplace_back("INPUT_ENCODING         = UTF-8");
    lines.emplace_back("MARKDOWN_SUPPORT       = YES");
    lines.emplace_back("TAB_SIZE               = 4");
    lines.emplace_back("RECURSIVE              = NO");
    lines.emplace_back("GENERATE_RTF           = YES");
    lines.emplace_back("RTF_HYPERLINKS         = YES");
    
    lines.emplace_back("EXCLUDE                = doxy");
    lines.emplace_back("FILE_PATTERNS          = *.md");
    lines.emplace_back("RTF_OUTPUT             = rtf");
    lines.emplace_back("OUTPUT_DIRECTORY       = doxy");
    lines.emplace_back("RTF_EXTENSIONS_FILE    = doxygen_rtf.cfg");
    lines.emplace_back("INPUT                  = document.md");

    std::string str = doc.getDocumentTitleAny();
    if (!str.empty())
    {
        lines.emplace_back("PROJECT_NAME           = " + str);
    }

    str = doc.getMetaTagValueAsSingleLineText(appCfg, "version", ",");
    if (!str.empty())
    {
        lines.emplace_back("PROJECT_NUMBER         = " + str);
    }

    // English/Russian
    str = doc.getDocumentLanguage(appCfg);
    if (!str.empty())
        str = findLangTagByString(str);
    if (!str.empty())
        str = findGoxygenLanguageByLangTag(str);
    if (str.empty())
        str = "English";
    lines.emplace_back("OUTPUT_LANGUAGE        = " + str);
    

    return marty_cpp::mergeLines(lines, getConfigsLinefeed(), true  /* addTrailingNewLine */ );
}

//----------------------------------------------------------------------------
template<typename FilenameStringType> inline
std::string generateDoxygenRtfCfg(const AppConfig<FilenameStringType> &appCfg, const Document &doc)
{
    std::vector<std::string> lines;

    std::string str = doc.getDocumentTitleAny();
    if (!str.empty())
    {
        lines.emplace_back("Title = " + str);
    }

    str = doc.getMetaTagValueAsSingleLineText(appCfg, "version", ",");
    if (!str.empty())
    {
        lines.emplace_back("Version = " + str);
    }

    str = doc.getMetaTagValueAsSingleLineText(appCfg, "author", ",");
    if (!str.empty())
    {
        lines.emplace_back("Author = " + str);
    }

    return marty_cpp::mergeLines(lines, getConfigsLinefeed(), true  /* addTrailingNewLine */ );
}

//----------------------------------------------------------------------------
inline
std::string generateFinalFilenameFromTitle(const std::string &titleStr, bool bTransliterate)
{
    // https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names

    static const std::string restrictedChars = "^*\"/\\<>:|?.";
    // ^ / * " / \ < > : | ?

    std::string resFilename; resFilename.reserve(titleStr.size());
    for(auto ch : titleStr)
    {
        if ((unsigned)(int)ch<=(unsigned)' ' || restrictedChars.find(ch)!=restrictedChars.npos)
        {
            ch = '_';
        }

        resFilename.append(1,ch);
    }

    if (bTransliterate)
    {
        resFilename = umba::transliterate(resFilename);
    }


    return resFilename;
}

//----------------------------------------------------------------------------
inline
std::wstring generateFinalFilenameFromTitle(const std::wstring &titleStr, bool bTransliterate)
{
    // https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names

    static const std::wstring restrictedChars = L"^*\"/\\<>:|?.";
    // ^ / * " / \ < > : | ?

    std::wstring resFilename; resFilename.reserve(titleStr.size());
    for(auto ch : titleStr)
    {
        if ((unsigned)(int)ch<=(unsigned)' ' || restrictedChars.find(ch)!=restrictedChars.npos)
        {
            ch = '_';
        }

        resFilename.append(1,ch);
    }

    if (bTransliterate)
    {
        
        resFilename = umba::fromUtf8(umba::transliterate(umba::toUtf8(resFilename)));
    }

    return resFilename;
}

//----------------------------------------------------------------------------
inline
bool isWindows32OnWindows64()
{
    #if !defined(WIN32) && !defined(_WIN32)

        return false; // not a windows at all

    #else

        //#if defined(WIN64) || defined(_WIN64)

        //    return false; // 64хбитное приложение, нет проблем

        //#else

            // https://stackoverflow.com/questions/14184137/how-can-i-determine-whether-a-process-is-32-or-64-bit

            typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

            BOOL bIsWow64 = FALSE;

            auto hKernel = GetModuleHandle(TEXT("kernel32"));
            if (hKernel)
            {
	            LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(hKernel,"IsWow64Process");
                if (fnIsWow64Process)
                {
                    if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
                    {
                        //handle error
                    }                
                }
            }

            return bIsWow64 ? true : false;

        //#endif

    #endif
}

//----------------------------------------------------------------------------
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

inline
std::wstring findDoxygenExecutableName()
{
    #if !defined(WIN32) && !defined(_WIN32)

        return L"doxygen";

    #else

        // https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-functions

        std::wstring doxygen = L"doxygen.exe";


        // Doxygen можно поискать тут:
        // Компьютер\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\doxygen_is1
        // "Inno Setup: App Path"
        // "InstallLocation"

        // KEY_WOW64_64KEY

        REGSAM samDesired = KEY_READ;
        if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
        {
            samDesired |= KEY_WOW64_64KEY;
        }

        HKEY hk = 0;
        LSTATUS status = RegOpenKeyExW( HKEY_LOCAL_MACHINE
                                      //, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Conan Package Manager_is1"
                                      , L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\doxygen_is1"
                                      //, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
                                      //, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
                                      //, L"SOFTWARE\\Microsoft\\Windows"
                                      //, L"SOFTWARE\\Microsoft"
                                      //, L"SOFTWARE"
                                      , 0 // ulOptions
                                      , samDesired
                                      , &hk
                                      );
        if (status==ERROR_SUCCESS)
        {
            wchar_t buf[1024];
            DWORD type;
            DWORD cbData = sizeof(buf); // in bytes
            status = RegQueryValueExW( hk
                                     , L"InstallLocation"
                                     , 0 // reserved
                                     , &type
                                     , (LPBYTE)&buf[0]
                                     , &cbData
                                     );

            if (status==ERROR_SUCCESS && type==REG_SZ)
            {
                if (cbData>(sizeof(buf)-1))
                    cbData = sizeof(buf)-1;

                std::size_t numChars = cbData/sizeof(wchar_t);
                if (numChars>0)
                {
                    if (buf[numChars-1]==0)
                        --numChars;
                }

                if (numChars>0)
                {
	                doxygen = std::wstring(buf, numChars);
	                doxygen = umba::filename::appendPath(doxygen, std::wstring(L"bin"));
                    doxygen = umba::filename::appendPath(doxygen, std::wstring(L"doxygen.exe"));
                }

            }
        
            RegCloseKey(hk);
        }

        return doxygen;

    #endif
}

//----------------------------------------------------------------------------
inline
void showErrorMessageBox(std::string str)
{

	#if defined(WIN32) || defined(_WIN32)
	
	    MessageBoxA( 0 // hwnd
                   , str.c_str()
                   , "Umba Markdown PP Viewer"
                   , MB_OK | MB_ICONERROR
                   );
	
	#endif

}

//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)



//------------------------------
inline
HKEY regCreateKeyHelper(HKEY hKeyRoot, const std::wstring &path, REGSAM samDesired)
{
    if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    {
        samDesired |= KEY_WOW64_64KEY;
    }
    
    HKEY hKeyRes = 0;
    DWORD dwDisposition = 0;

    LSTATUS status = RegCreateKeyExW( hKeyRoot
                                    , path.c_str()
                                    , 0 // reserved
                                    , 0 // lpClass - The user-defined class type of this key. This parameter may be ignored. This parameter can be NULL.
                                    , REG_OPTION_NON_VOLATILE // default, 0
                                    , samDesired
                                    , 0 // lpSecurityAttributes
                                    , &hKeyRes
                                    , &dwDisposition
                                    );
    if (status!=ERROR_SUCCESS)
    {
        return 0;
    }

    return hKeyRes;
}

//------------------------------
inline
bool regSetValue(HKEY hKey, const std::wstring &varName, const std::wstring &value)
{
    LSTATUS status = RegSetValueW(hKey, varName.c_str(), REG_SZ, (LPCWSTR)value.c_str(), (DWORD)(value.size()+1)*sizeof(wchar_t));
    return status==ERROR_SUCCESS;
}

//------------------------------
#define UMBA_PP_VIEWER_USE_HKCU

//------------------------------
inline
HKEY regGetShellExtentionsRoot()
{
    #ifdef UMBA_PP_VIEWER_USE_HKCU

       return HKEY_CURRENT_USER;

    #else

       return HKEY_CLASSES_ROOT;

    #endif
}

//------------------------------
inline
std::wstring regShellExtentionHandlersRootPath()
{
    std::wstring regPath;

    #ifdef UMBA_PP_VIEWER_USE_HKCU

       regPath.append(L"Software");
       regPath.append(L"\\Classes");

    #else

    #endif

    return regPath;
}

inline
bool regShellExtentionHandlerApplication(const std::wstring &appNameId, const std::wstring &shellVerb, const std::wstring &appCommand)
{

    // Компьютер\HKEY_CLASSES_ROOT\md__auto_file
    //     shell
    //       open
    //         command
    //           default value: "F:\_github\umba-tools\umba-md-pp\.out\msvc2019\x86\Debug\umba-md-pp-view.exe" "%1"
    //  
    // HKEY_CLASSES_ROOT\.md_
    //     default value md__auto_file
    //  
    // The nameless key is the default one - https://learn.microsoft.com/en-us/dotnet/api/microsoft.win32.registry.setvalue?view=net-8.0&redirectedfrom=MSDN#overloads

    HKEY hRootKey = regGetShellExtentionsRoot();

    std::wstring regPath = regShellExtentionHandlersRootPath();

    if (!regPath.empty())
        regPath.append(L"\\");

    regPath.append(appNameId);
    regPath.append(L"\\shell");
    regPath.append(L"\\");
    regPath.append(shellVerb);
    regPath.append(L"\\command");

    HKEY hKey = regCreateKeyHelper(hRootKey, regPath,  /* KEY_READ| */ KEY_WRITE);

    if (!hKey)
        return false;

    bool res = regSetValue(hKey, L"" /* varName */ , appCommand);

    RegCloseKey(hKey);

    return res;
}

//------------------------------
inline
bool regShellExtentionHandlerForExt(const std::wstring &appNameId, std::wstring ext)
{

    // HKEY_CLASSES_ROOT\.md_
    //     default value md__auto_file

    if (ext.empty())
        return false;

    if (ext.front()!=L'.')
    {
        ext = L"." + ext;
    }

    HKEY hRootKey = regGetShellExtentionsRoot();

    std::wstring regPath = regShellExtentionHandlersRootPath();

    if (!regPath.empty())
        regPath.append(L"\\");

    regPath.append(ext);

    HKEY hKey = regCreateKeyHelper(hRootKey, regPath,  /* KEY_READ| */ KEY_WRITE);

    if (!hKey)
        return false;

    bool res = regSetValue(hKey, L"" /* varName */ , appNameId);

    RegCloseKey(hKey);

    return res;
}

//------------------------------
inline
bool regShellExtentionHandlerForExtList(const std::wstring &appNameId, const std::vector<std::wstring> &extList)
{
    bool res = true;

    for(auto ext: extList)
    {
        if (!regShellExtentionHandlerForExt(appNameId, ext))
            res = false;
    }

    return res;
}

//------------------------------
inline
bool regShellExtentionHandlerForExtList(const std::wstring &appNameId, const std::wstring &extCommaList)
{
    auto extList = marty_cpp::splitToLinesSimple(extCommaList, false, ',');
    for(auto &ext: extList)
    {
        umba::string_plus::trim(ext);
    }

    return regShellExtentionHandlerForExtList(appNameId, extList);
}

#endif





/*
RTF Ext

Title           = 
Author
Version

Manager
Subject
Comments
Keywords
*/

