#pragma once


#include "snippet_options.h"
//
#include "encoding/encoding.h"
//
#include "umba/program_location.h"
#include "umba/enum_helpers.h"
#include "umba/flag_helpers.h"
#include "umba/string_plus.h"
//
//#include "umba/regex_helpers.h"
//
#include "umba/filename.h"
#include "umba/filesys.h"
//
#include "marty_cpp/src_normalization.h"

//
#include <string>
#include <vector>
#include <map>
#include <unordered_map>



//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
struct LangOptions
{
    std::string listingTag;
    std::string cutPrefix ;

};

//----------------------------------------------------------------------------
struct AppConfig
{

    #if defined(WIN32) || defined(_WIN32)
        marty_cpp::ELinefeedType outputLinefeed = marty_cpp::ELinefeedType::crlf;
    #else
        marty_cpp::ELinefeedType outputLinefeed = marty_cpp::ELinefeedType::lf;
    #endif

    std::vector<std::string>                              samplesPaths;
    std::unordered_map<std::string, std::string>          extToLang  ;
    std::unordered_map<std::string, LangOptions>          langOptions;
    std::unordered_set<SnippetOptions>                    snippetOptions;


    bool updateSnippetOptions(const std::string &opts)
    {
        return deserializeSnippetOptions(opts, snippetOptions);
    }

    // umba::string_plus::trim(lineCopy);
    // if (!umba::string_plus::starts_with_and_strip(lineCopy, tagPrefix))
    //     return false;

    // std::unordered_set<SnippetOptions>                    snippetOptions;

    void addSamplesPaths( const std::vector<std::string> &pl )
    {
        //std::vector<FilenameStringType> &dirs = m_lookupMap[lvl].lookupDirs;
        samplesPaths.insert( samplesPaths.end(), pl.begin(), pl.end() );
    }

    //! ��������� ���� ������ ��������
    /*! ������������ �������� ���� ������:
        - Win32 - ';' (����� � �������, semicolon)
        - Linux - ':' (���������, colon)
     */
    void addSamplesPaths( const std::string &pl )
    {
        addSamplesPaths( umba::filename::splitPathList( pl ) );
    }

    static
    std::string autoEncodeToUtf(const std::string &text)
    {
        size_t bomSize = 0;
        //const charDataPtr = 
        encoding::EncodingsApi* pEncodingsApi = encoding::getEncodingsApi();
        std::string detectRes = pEncodingsApi->detect( text.data(), text.size(), bomSize );
        auto cpId = pEncodingsApi->getCodePageByName(detectRes);
        std::string utfText = pEncodingsApi->convert( text.data()+bomSize, text.size()-bomSize, cpId, encoding::EncodingsApi::cpid_UTF8 );
        return utfText;
    }


    bool findSamplesFile(const std::string &lookFor, std::string &foundFullFilename, std::string &foundFileText) const
    {
         for(auto path: samplesPaths)
         {
             auto fullName = umba::filename::appendPath(path, lookFor);
             std::string orgText;
             if (umba::filesys::readFile(fullName, orgText))
             {
                 orgText           = autoEncodeToUtf(orgText);
                 foundFileText     = marty_cpp::normalizeCrLfToLf(orgText);
                 foundFullFilename = fullName;
                 return true;
             }
         }

         return false;
    }

    static
    std::string normalizeExt(std::string ext)
    {
        while(!ext.empty() && ext.front()=='.')
        {
            ext.erase(0,1);
        }

        return marty_cpp::toLower(ext);
    }
    

    //----------------------------------------------------------------------------
    // Option helpers for --add-lang-file-extention=cpp:cpp,cxx,c++,cc,h,hpp,h++

    bool addLangExtentions(std::string lang, const std::vector<std::string> &extList)
    {
        if (lang.empty())
            return false;

        //lang = marty_cpp::toLower(lang);

        std::size_t cnt = 0;
        for(auto ext: extList)
        {
            ext = normalizeExt(ext);
            if (ext.empty())
                continue;

            extToLang[ext] = lang;

            ++cnt;
        }

        return cnt>0;
    }

    bool addLangExtentions(const std::string &lang, const std::string &extList)
    {
        std::vector<std::string> extListVec = marty_cpp::splitToLinesSimple(extList, false, ',');
        return addLangExtentions(lang, extListVec);
    }

    bool addLangExtentions(const std::string &langAndExts)
    {
        std::vector<std::string> langExtsPair = marty_cpp::splitToLinesSimple(langAndExts, false, ':');
        if (langExtsPair.size()<2)
            return false;
        return addLangExtentions(langExtsPair[0], langExtsPair[1]);
    }

    std::string getLangByExt(std::string ext) const
    {
        ext = normalizeExt(ext);

        // if (ext.empty())
        //     continue;

        std::unordered_map<std::string, std::string>::const_iterator it = extToLang.find(ext);
        if (it==extToLang.end())
        {
            return std::string();
        }

        return it->second;
    }

    std::string getLangByFilename(const std::string &name) const
    {
        return getLangByExt(umba::filename::getFileExtention(name));
    }

    //----------------------------------------------------------------------------


    //----------------------------------------------------------------------------
    // --set-lang-cut-prefix=nut,//!#
    bool setLangCutPrefix(const std::string &lang, const std::string &cutPrefix)
    {
        langOptions[lang].cutPrefix = cutPrefix;
        return true;
    }

    bool setLangCutPrefix(const std::string &langPrefixPair)
    {   
        std::string lang, cutPrefix;
        if (!umba::string_plus::split_to_pair(langPrefixPair, lang, cutPrefix, ':'))
        {
            return false;
        }

        return setLangCutPrefix(lang, cutPrefix);
    }

    std::string getLangCutPrefix(const std::string &lang) const
    {
        std::unordered_map<std::string, LangOptions>::const_iterator it = langOptions.find(lang);
        if (it==langOptions.end())
        {
            return std::string();
        }
        return it->second.cutPrefix;
    }


    //----------------------------------------------------------------------------
    // --set-lang-listing-tag=nut,sq
    bool setLangListingTag(const std::string &lang, const std::string &listingTag)
    {
        langOptions[lang].listingTag = listingTag;
        return true;
    }

    bool setLangListingTag(const std::string &langTagPair)
    {   
        std::string lang, listingTag;
        if (!umba::string_plus::split_to_pair(langTagPair, lang, listingTag, ':'))
        {
            return false;
        }

        return setLangListingTag(lang, listingTag);
    }

    std::string getLangListingTag(const std::string &lang) const
    {
        std::unordered_map<std::string, LangOptions>::const_iterator it = langOptions.find(lang);
        if (it==langOptions.end())
        {
            return std::string();
        }
        return it->second.listingTag;
    }



// bool hasEq = umba::string_plus::split_to_pair(cmdLineArg, opt, optArg, '=');
    //std::unordered_map<std::string, LangOptions>          langOptions;
// struct LangOptions
// {
//     std::string listingTag;
//     std::string cutPrefix ;
//  
// };

//std::vector<StringType> splitToLinesSimple(const StringType &str, bool addEmptyLineAfterLastLf = true, typename StringType::value_type lfChar=(typename StringType::value_type)'\n')


    //std::vector<std::string> lines = marty_utext::splitToLinesSimple(normUtext, true /* addEmptyLineAfterLastLf */ );

// inline char    toLower( char ch )     { return isUpper(ch) ? ch-'A'+'a' : ch; }
// inline char    toUpper( char ch )     { return isLower(ch) ? ch-'a'+'A' : ch; }
//  
// inline wchar_t toLower( wchar_t ch )  { return (wchar_t)(isUpper(ch) ? ch-L'A'+L'a' : ch); }
// inline wchar_t toUpper( wchar_t ch )  { return (wchar_t)(isLower(ch) ? ch-L'a'+L'A' : ch); }

// --cut-options=lineno,notrim,notag
// --set-lang-cut-prefix=nut,//!#
// --set-lang-code-suffix=nut,nut

    // std::unordered_map<std::string, std::string>          extToLang  ;
    // std::unordered_map<std::string, LangOptions>          langOptions;




}; // struct AppConfig




