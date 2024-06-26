/*! \file
    \brief Утилита umba-md-pp - препроцессор для маркдауна
 */

// Должна быть первой
#include "umba/umba.h"
//---

//#-sort
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"
//#+sort

#include "umba/debug_helpers.h"
#include "umba/time_service.h"

#if defined(_WIN32) || defined(WIN32)
    #include "umba/winconhelpers.h"
#endif

#include <iostream>
#include <iomanip>
#include <string>
// #include <filesystem>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"
#include "umba/info_log.h"
//
#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/sort_includes.h"
#include "marty_cpp/enums.h"
#include "marty_cpp/src_normalization.h"

//
#include "marty_utf/utf.h"

//
#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"
#include "umba/time_service.h"

#include "umba/filesys.h"
#include "umba/filename.h"
#include "umba/format_message.h"
#include "umba/id_gen.h"

#include "snippet_options.h"

#include "encoding/encoding.h"
#include "utils.h"
#include "batch_utils.h"

//
#include "marty_tr/marty_tr.h"
#include "tr/tr.h"
//

#include "marty_yaml_toml_json/json_utils.h"
#include "marty_yaml_toml_json/yaml_json.h"
#include "marty_yaml_toml_json/yaml_utils.h"



#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


#include "app_config.h"
#include "viewer_utils.h"




umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter umbaLogStreamErr(&cerrWriter);
umba::SimpleFormatter umbaLogStreamMsg(&cerrWriter);
umba::SimpleFormatter umbaLogStreamNul(&nulWriter);

bool umbaLogGccFormat   = false; // true;
bool umbaLogSourceInfo  = false;

bool bOverwrite         = false;

unsigned jsonIndent     = 2;

std::string               inputFilename;
std::string               outputFilename;


#include "log.h"
//
#include "umba/cmd_line.h"
//

AppConfig<std::string> appConfig;

// Парсер параметров командной строки
#include "arg_parser.h"

//
std::string curFile;
unsigned lineNo = 0;

#include "processing.h"


static
auto trErrHandler = marty_tr::makeErrReportHandler([](marty_tr::MsgNotFound what, const std::string& msg, const std::string& catId, const std::string& langId)
{
    using umba::lout;
    using namespace umba::omanip;

    std::cerr << "---\n";
    std::cerr << "Translation not found(" << marty_tr::to_string(what) << "): [" << langId << ":" << catId << "] - " << msg << "\n";
    std::cerr << "\n";

}
);


int main(int argc, char* argv[])
{
    umba::time_service::init();

    marty_tr::tr_set_err_handler(&trErrHandler);
    marty_tr::tr_init_all_translations(tr_get_translations_json());


    //std::cout << "Doxygen: " << umba::toUtf8(findDoxygenExecutableName()) << "\n";

    using namespace umba::omanip;


    auto argsParser = umba::command_line::makeArgsParser( ArgParser<std::string>()
                                                        , CommandLineOptionCollector()
                                                        , argc, argv
                                                        , umba::program_location::getProgramLocation
                                                            ( argc, argv
                                                            , false // useUserFolder = false
                                                            //, "" // overrideExeName
                                                            )
                                                        );

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        argsParser.args.clear();
        argsParser.args.push_back("--overwrite");


        std::string cwd = umba::filesys::getCurrentDirectory<std::string>();
        std::cout << "Working Dir: " << cwd << "\n";

        #if (defined(WIN32) || defined(_WIN32))

            std::string rootPath;

            //if (winhelpers::isProcessHasParentOneOf({"devenv"}))
            {
                // По умолчанию студия задаёт текущим каталогом тот
                rootPath = umba::filename::makeCanonical(umba::filename::appendPath<std::string>(cwd, "..\\..\\..\\"));
                //argsParser.args.push_back("--batch-output-root=D:/temp/mdpp-test");
            }
            // else if (winhelpers::isProcessHasParentOneOf({"code"}))
            // {
            //     // По умолчанию VSCode задаёт текущим каталогом тот, где лежит бинарник
            //     rootPath = umba::filename::makeCanonical(umba::filename::appendPath<std::string>(cwd, "..\\..\\..\\..\\"));
            // }
            // else
            // {
            //     //rootPath = umba::filename::makeCanonical(umba::filename::appendPath<std::string>(cwd, "..\\..\\..\\"));
            // }

            //#endif

            rootPath = umba::filename::appendPathSepCopy(rootPath);

        #endif


        
        //argsParser.args.push_back("@" + rootPath + "_distr_conf/conf/umba-md-pp.options");

        // argsParser.args.push_back("--set-insert-options=filename,path,filenameLineNo");
        // argsParser.args.push_back("--add-examples-path=" + rootPath + "src;" + rootPath + "tests\\snippets");
        // //argsParser.args.push_back("--set-insert-options=lineno,notrim,notag,fail");
        // argsParser.args.push_back("--set-insert-options=filename,path,filenameLineNo,fail,snippet-options,trim-arround");
        // argsParser.args.push_back("--processing-options=generate-toc,meta-data");
        // argsParser.args.push_back("--serialize-meta-tags=title,descripion,author");
        // argsParser.args.push_back("--target-renderer=doxygen");
        //  
        // // argsParser.args.push_back("");
        // // argsParser.args.push_back("");
        // argsParser.args.push_back(rootPath + "tests\\test04.md_");

        argsParser.args.push_back("--batch-scan="+rootPath);
        argsParser.args.push_back("--batch-scan="+rootPath+"doc");
        argsParser.args.push_back("--batch-scan-recurse="+rootPath+"tests");
        
        argsParser.args.push_back("--batch-page-index-file=pages.md");
        
        // batch-exclude-files
        // batch-output-root


    }

    //programLocationInfo = argsParser.programLocationInfo;

    // try
    // {
        // Job completed - may be, --where option found
        if (argsParser.mustExit)
            return 0;
       
        // if (!argsParser.quet)
        // {
        //     printNameVersion();
        // }
       
        if (!argsParser.parseStdBuiltins())
            return 1;
        if (argsParser.mustExit)
            return 0;
       
        if (!argsParser.parse())
            return 1;
        if (argsParser.mustExit)
            return 0;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << e.what() << "\n";
    //     return -1;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << "command line arguments parsing error" << "\n";
    //     return -1;
    // }

    if (!argsParser.quet  /* && !hasHelpOption */ )
    {
        //printNameVersion();
        //LOG_MSG_OPT<<"\n";
        umba::cli_tool_helpers::printNameVersion(umbaLogStreamMsg);
    }



    //unsigned errCount = 0;

    auto &infoLog = argsParser.quet ? umbaLogStreamNul : umbaLogStreamMsg;


    if (appConfig.isBatchMode())
    {
        std::vector<std::string>  foundFiles;
        std::vector<std::string>  foundFilesRootFolders;

        batchScanForFiles( appConfig.batchScanPaths
                         , appConfig.mdppExtentions
                         , appConfig.batchExcludeDirs
                         , appConfig.batchExcludeFilesMaskList
                         , infoLog
                         , foundFiles
                         , &foundFilesRootFolders
                         );

        if (foundFiles.empty())
        {
            umbaLogStreamMsg << "No files found\n";
            return 0;
        }
        else
        {
            if (foundFiles.size() != foundFilesRootFolders.size())
            {
                LOG_ERR_OPT << "Missmatch files and folders size, something goes wrong" << "\n";
                return 1;
            }
            

            // umba::info_log::printSectionHeader(infoLog, "Files for processing");
            // for(const auto &foundFile: foundFiles)
            // {
            //     infoLog << foundFile << "\n";
            // }

            umba::info_log::printSectionHeader(infoLog, "Processing found files");

            std::string commonLang        = "English";
            bool        commonLangChanged = false    ;

            std::vector<std::string>::const_iterator fileIt   = foundFiles.begin();
            std::vector<std::string>::const_iterator folderIt = foundFilesRootFolders.begin();

            //std::map<std::wstring, std::string> pageIndex;
            std::vector< std::pair<std::wstring, std::string> > pagesIndex;
            std::string calculatedCommonPath = umba::filename::makeCanonical(umba::filename::getPath(foundFiles.front()));

            for(; fileIt!=foundFiles.end() && folderIt!=foundFilesRootFolders.end(); ++fileIt, ++folderIt)
            {
                auto fileRootPath = *folderIt;
                auto fileFullName = *fileIt  ;
                auto fileRelName  = fileFullName;
                
                if (!umba::string_plus::starts_with_and_strip(fileRelName, fileRootPath))
                {
                    LOG_ERR_OPT << "File '" << fileFullName << "' not in it's root folder '" << fileRootPath << "' - something goes wrong" << "\n";
                    return 2;
                }

                umba::filename::stripLastPathSep(fileRootPath);
                umba::filename::stripFirstPathSep(fileRelName);

                infoLog << "Processing file '" << fileRelName << "' from '" << fileRootPath << "' (" << fileFullName << ")";

                #define UMBA_MD_PP_BATCH_MODE_PROCESSING_LOG_MULTILINE
                // #if defined(UMBA_MD_PP_BATCH_MODE_PROCESSING_LOG_MULTILINE)
                //     infoLog << "\n    Target: ";
                // #else
                //     infoLog << ", target: ";
                // #endif

                std::string ext       = umba::filename::getExt(fileFullName);
                std::string targetExt = appConfig.getTargetFileExtention(ext);
                std::string targetRelName;

                if (ext==targetExt)
                {
                    LOG_WARN_OPT("same-file") << "source file and target file has same extention, skip processing it\n";
                    continue;
                }

                //std::string outputFileName;
                if (appConfig.batchOutputRoot.empty())
                {
                    // Просто меняем расширение исходного файла
                    targetRelName   = umba::filename::appendExt(umba::filename::getPathFile(fileFullName), targetExt);

                    std::string targetPath = umba::filename::makeCanonical(umba::filename::getPath(targetRelName));
                    std::string::const_iterator pit = targetPath.begin();
                    std::string::iterator       cit = calculatedCommonPath.begin();
                    for(; pit!=targetPath.end() && cit!=calculatedCommonPath.end() && *pit==*cit; ++pit, ++cit) {}
                    if (cit!=calculatedCommonPath.end())
                    {
                        calculatedCommonPath.erase(cit, calculatedCommonPath.end());
                    }

                    outputFilename = targetRelName;
                }
                else
                {
                    // Присовывем в output
                    targetRelName   = umba::filename::appendExt(umba::filename::getPathFile(fileRelName), targetExt);
                    outputFilename  = umba::filename::appendPath(appConfig.batchOutputRoot, targetRelName);
                }

                infoLog << "'" << outputFilename << "'" << "\n";

                //infoLog << "Processing file" foundFile << "\n";

                std::string inputFileText;
                if (!AppConfig<std::string>::readInputFile(*fileIt, inputFileText))
                {
                    LOG_ERR_OPT << umba::formatMessage("failed to read input file: '$(fileName)'")
                                                      .arg("fileName",*fileIt)
                                                      .toString()
                                << "\n";
                    return 3;
                }

                appConfig.setStrictPathFromFilename(*fileIt); // Пока так

                curFile = *fileIt;

                std::string projectOptionsFile;
                if (findProjectOptionsFile(inputFilename, projectOptionsFile))
                {
                    appConfig.setStrictPathFromFilename(projectOptionsFile);
                    argsParser.parseOptionsFile(projectOptionsFile);
                }
            
                appConfig.checkAdjustDocNumericLevels();
                appConfig.checkTargetFormat();

                Document doc;
                std::string resText     = processMdFile(appConfig, inputFileText, inputFilename, doc);
                std::string docTitle    = doc.getDocumentTitleAny();
                std::string docLanguage = doc.getDocumentLanguage(appConfig);

                if (!commonLangChanged) // Язык меняли?
                {
                    if (umba::string_plus::tolower_copy(commonLang)!=umba::string_plus::tolower_copy(docLanguage))
                    {
                        // найден язык, не являющийся дефолтным
                        commonLang = docLanguage; // Используем первое найденное значение, остальные игнорируем
                        commonLangChanged = true;
                    }
                }

                std::wstring pageIndexText = umba::fromUtf8(docTitle);
                pagesIndex.emplace_back(pageIndexText, targetRelName);
                // std::map<std::wstring, std::string> pageIndex;
                // std::vector< std::pair<std::wstring, std::string> > pagesIndex;

                if (appConfig.batchOutputRoot.empty())
                {
                    for(auto &pi : pagesIndex)
                    {
                        pi.second.erase(0, calculatedCommonPath.size());
                        umba::filename::stripFirstPathSep(pi.second);
                    }
                    umba::filename::stripLastPathSep(calculatedCommonPath);
                }

                try
                {
                    if (!argsParser.quet)
                    {
                        #if defined(UMBA_MD_PP_BATCH_MODE_PROCESSING_LOG_MULTILINE)
                            umbaLogStreamMsg << "    Writting output to: ";
                        #else
                            umbaLogStreamMsg << " - writting output to: ";
                        #endif
                        umbaLogStreamMsg << outputFilename << "\n";
                    }
            
                    umba::filesys::createDirectoryEx<std::string>( umba::filename::getPath(outputFilename), true /* forceCreatePath */ );

                    umba::cli_tool_helpers::writeOutput( outputFilename, umba::cli_tool_helpers::IoFileType::regularFile // outputFileType
                                                       , encoding::ToUtf8(), encoding::FromUtf8()
                                                       , resText, std::string() // bomData
                                                       , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                       );
                } // try
                catch(const std::runtime_error &e)
                {
                    LOG_ERR_OPT << e.what() << "\n";
                    return 4;
                }

            } // for


            std::string langTag = findLangTagByString(commonLang);
            if (langTag.empty())
                langTag = "en";

            std::string pageIndexTitle = marty_tr::tr("title"   , "pages-index", langTag);
            std::string noTitle        = marty_tr::tr("no-title", "pages-index", langTag);

            if (!appConfig.batchPageIndexFileName.empty() && !pagesIndex.empty())
            {
                for(auto it=pagesIndex.begin(); it!=pagesIndex.end(); ++it)
                {
                    if (it->first.empty())
                    {
                        it->first = umba::fromUtf8(noTitle);
                    }
                }

                std::stable_sort( pagesIndex.begin(), pagesIndex.end()
                                , [](const std::pair<std::wstring, std::string> &p1, const std::pair<std::wstring, std::string> &p2)
                                  {
                                      return p1.first<p2.first;
                                  }
                                );

                std::string pagesIndexText;
                
                pagesIndexText.append("# ");
                pagesIndexText.append(pageIndexTitle);
                //pagesIndexText.append(1, '\n');
                pagesIndexText.append(1, '\n');
                //marty_tr::tr(const std::string &msgId, std::string catId, std::string langId)

                std::string firstSectionLetter;
        //         marty_utf
        // auto uch = (utf8_char_t)str[curPos];
        //  
        // auto symbolNumBytes = getNumberOfBytesUtf8(uch);


                for(std::vector< std::pair<std::wstring, std::string> >::iterator it=pagesIndex.begin(); it!=pagesIndex.end(); ++it)
                {
                    std::string thisPageTitleUtf = umba::toUtf8(it->first);
                    if (!thisPageTitleUtf.empty())
                    {
                        std::size_t firstCharLen = marty_utf::getNumberOfBytesUtf8(thisPageTitleUtf[0]);
                        if (firstCharLen<=thisPageTitleUtf.size())
                        {
                            std::string titleFirstLetter = std::string(thisPageTitleUtf, 0, firstCharLen);
                            if (firstSectionLetter!=titleFirstLetter && !titleFirstLetter.empty())
                            {
                                firstSectionLetter = titleFirstLetter;
                                pagesIndexText.append("\n\n");
                                pagesIndexText.append(" - **");
                                pagesIndexText.append(firstSectionLetter);
                                pagesIndexText.append("**\n");
                            }
                        }
                    }

                    pagesIndexText.append(" - [");
                    pagesIndexText.append(thisPageTitleUtf);
                    pagesIndexText.append("](");
                    pagesIndexText.append(umba::filename::makeCanonical(it->second, '/'));
                    pagesIndexText.append(")\n");
                }

                auto targetPagesIndexText = marty_cpp::converLfToOutputFormat(pagesIndexText, appConfig.outputLinefeed);

                std::string pageIndexFileName = appConfig.batchPageIndexFileName;
                if (!umba::filename::isAbsPath(pageIndexFileName))
                {
                    if (!appConfig.batchOutputRoot.empty())
                    {
                        pageIndexFileName = umba::filename::appendPath(appConfig.batchOutputRoot, pageIndexFileName);
                    }
                    else
                    {
                        pageIndexFileName = umba::filename::appendPath(calculatedCommonPath, pageIndexFileName);
                    }
                }

                pageIndexFileName = umba::filename::makeCanonical(pageIndexFileName);
                
                umba::filesys::createDirectoryEx<std::string>( umba::filename::getPath(pageIndexFileName), true /* forceCreatePath */ );

                umba::cli_tool_helpers::writeOutput( pageIndexFileName, umba::cli_tool_helpers::IoFileType::regularFile // outputFileType
                                                   , encoding::ToUtf8(), encoding::FromUtf8()
                                                   , targetPagesIndexText, std::string() // bomData
                                                   , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                   );
                

            }


            return 0;
        }
    
    }
    else
    {
        // Single file mode
        if (inputFilename.empty())
        {
            LOG_ERR_OPT << "input file name not taken"
                        << "\n";
            return 5;
        }

        umba::cli_tool_helpers::IoFileType outputFileType = umba::cli_tool_helpers::IoFileType::nameEmpty;
        if (outputFilename.empty())
        {
            //outputFilename = "STDOUT";
            outputFilename = tryMakeOutputFilenameFromInput(inputFilename);
            outputFileType = umba::cli_tool_helpers::IoFileType::regularFile;
            //generateOutputFileNameFromInputFileName
        }
    
        if (outputFilename.empty())
        {
            outputFilename = "STDOUT";
        }
    
        outputFileType = umba::cli_tool_helpers::detectFilenameType(outputFilename, false /* !bInput */);
    
        #if defined(WIN32) || defined(_WIN32)
        if (outputFileType==umba::cli_tool_helpers::IoFileType::clipboard)
        {
            LOG_ERR_OPT << "invalid output file name"
                        << "\n";
            return 6;
        }
        #endif

        if (!argsParser.quet)
        {
            umbaLogStreamMsg << ": "<<inputFilename<<"\n";
        }
    
        std::string inputFileText;
        //if (!umba::filesys::readFile(inputFilename, inputFileText))
        if (!AppConfig<std::string>::readInputFile(inputFilename, inputFileText))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to read input file: '$(fileName)'")
                                              .arg("fileName",inputFilename)
                                              .toString()
                        << "\n";
            //errCount++;
            return 7;
        }
    
        UMBA_USED(lineNo);
    
        appConfig.setStrictPathFromFilename(inputFilename);
    
    
    
        //std::string 
        curFile = inputFilename; // = fileName;
        //unsigned lineNo = 0;
    
    
        std::string projectOptionsFile;
        if (findProjectOptionsFile(inputFilename, projectOptionsFile))
        {
            appConfig.setStrictPathFromFilename(projectOptionsFile);
            argsParser.parseOptionsFile(projectOptionsFile);
        }
    
        appConfig.checkAdjustDocNumericLevels();
        appConfig.checkTargetFormat();
    
    
        Document doc;
        std::string resText  = processMdFile(appConfig, inputFileText, inputFilename, doc);
        //std::string docTitle = doc.getDocumentTitleAny();
    
        try
        {
            if (!argsParser.quet)
            {
                umbaLogStreamMsg << "Writting output to: "<<outputFilename<<"\n";
            }

            umba::cli_tool_helpers::writeOutput( outputFilename, outputFileType
                                               , encoding::ToUtf8(), encoding::FromUtf8()
                                               , resText, std::string() // bomData
                                               , true /* fromFile */, true /* utfSource */ , bOverwrite
                                               );
        } // try
        catch(const std::runtime_error &e)
        {
            LOG_ERR_OPT << e.what() << "\n";
            return 8;
        }


    }


    return 0;
}


#if (defined(WIN32) || defined(_WIN32)) && defined(__GNUC__)

   // Fix for MinGW problem - https://sourceforge.net/p/mingw-w64/bugs/942/
   // https://github.com/brechtsanders/winlibs_mingw/issues/106
   // https://stackoverflow.com/questions/74999026/is-there-the-commandlinetoargva-function-in-windows-c-c-vs-2022


   #include <winsock2.h>
   #include <windows.h>
   #include <shellapi.h>

   int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
   {
       UMBA_USED(hInstance);
       UMBA_USED(hPrevInstance);
       UMBA_USED(lpCmdLine);
       UMBA_USED(nCmdShow);

       int nArgs = 0;
       wchar_t ** wargv = CommandLineToArgvW( GetCommandLineW(), &nArgs );
       if (!wargv)
       {
           return 1;
       }

       // Count the number of bytes necessary to store the UTF-8 versions of those strings
       int n = 0;
       for (int i = 0;  i < nArgs;  i++)
       {
         n += WideCharToMultiByte( CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL ) + 1;
       }

       // Allocate the argv[] array + all the UTF-8 strings
       char **argv = (char**)new char*[( (nArgs + 1) * sizeof(char *) + n )];
       if (!argv)
       {
           return 1;
       }
       
       // Convert all wargv[] --> argv[]
       char * arg = (char *)&(argv[nArgs + 1]);
       for (int i = 0;  i < nArgs;  i++)
       {
         argv[i] = arg;
         arg += WideCharToMultiByte( CP_UTF8, 0, wargv[i], -1, arg, n, NULL, NULL ) + 1;
       }
       argv[nArgs] = NULL;

       return main(nArgs, argv);

   }



#endif


