#pragma once

#include <string>
#include <vector>
#include <unordered_map>


struct SectionInfo
{
    unsigned        sectionLevel = 0;
    std::string     originalTitle   ;
    std::string     sectionNumber   ; // 
    std::string     fullTitle       ; // with section number
    std::string     sectionTargetId ;

}; // struct SectionInfo



struct Document
{
    std::vector<std::string>                           collectedMetadataTexts; // from all processed documents, each string is an \n multiline text

    std::vector<std::string>                           lines   ; //!< MD lines
    std::vector<std::string>                           tocLines; //!< MD lines

    std::map<std::string, std::vector<SectionInfo> >   sectionInfos; //!< ���� � �������. OrgTitle->Info. ������ - ������, ��� ����� ���� ���������, � ��������� � ��� �� ����� � �� ���������

}; // struct Document


