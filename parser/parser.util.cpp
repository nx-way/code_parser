#include "parser.util.hpp"
#include <sstream>
#include <iostream>
#include <fstream>
#include <clang-c/Index.h>
using namespace reflang;

std::string parser::Convert(const CXString& s)
{
	std::string result = clang_getCString(s);
	clang_disposeString(s);
	return result;
}



std::string parser::GetFullName(CXCursor cursor)
{
	std::string name;
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0)
	{
		std::string cur = Convert(clang_getCursorSpelling(cursor));
		if (name.empty())
		{
			name = cur;
		}
		else
		{
			name = cur + "::" + name;
		}
		cursor = clang_getCursorSemanticParent(cursor);
	}

	return name;
}

std::string parser::GetName(const CXType& type)
{
	//TODO: unfortunately, this isn't good enough. It only works as long as the
	// type is fully qualified.
	return Convert(clang_getTypeSpelling(type));
}

std::string parser::GetFile(const CXCursor& cursor)
{
	auto location = clang_getCursorLocation(cursor);
	CXFile file;
	clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);
	return Convert(clang_getFileName(file));
}
std::string parser::GetComment(const CXCursor& cursor){
    std::string comment;
    CXString comment_text = clang_Cursor_getBriefCommentText(cursor);
    if (comment_text.data){
        comment = clang_getCString(comment_text);
    }
    // Release the CXString to avoid memory leaks
    clang_disposeString(comment_text);

    return std::move(comment);
}
std::string parser::GetCode(const CXCursor& cursor, TypeBase::Type type){
    bool is_class = type == TypeBase::Type::Class;

    std::string code;
    // Get the source range of the class declaration

    CXSourceRange extent = clang_getCursorExtent(cursor);
    CXSourceLocation startLocation = clang_getRangeStart(extent);
    CXSourceLocation endLocation = clang_getRangeEnd(extent);

    unsigned int start_line, start_column, end_line, end_column;
    clang_getSpellingLocation(startLocation, nullptr, &start_line, &start_column, nullptr);
    clang_getSpellingLocation(endLocation, nullptr, &end_line, &end_column, nullptr);

//    CXSourceLocation endLocation = clang_getRangeEnd(extent);

    // Get the translation unit
    CXTranslationUnit TU = clang_Cursor_getTranslationUnit(cursor);


    // Get the source code between start and end locations
    CXToken* tokens;
    unsigned int numTokens;
    clang_tokenize(TU, extent, &tokens, &numTokens);

    std::ostringstream out;
    unsigned int prevLine = 0; // Previous line number


    unsigned int indent_space = start_column;
    char last_char = 0;
    int indent = 0;
    unsigned int numTokens_1 = numTokens -1;
    for (unsigned int i = 0; i < numTokens; ++i) {
        CXString tokenSpelling = clang_getTokenSpelling(TU, tokens[i]);
        const char* tokenStr = clang_getCString(tokenSpelling);
        char this_char = tokenStr[0];

        // Get the line number of the current token
        CXSourceLocation tokenLocation = clang_getTokenLocation(TU, tokens[i]);

        {
            // Get the cursor at that source location
//            CXCursor locationCursor = clang_getCursor(TU, tokenLocation);
//            int maxChildDepth = calculateCursorDepth(cursor, locationCursor);
        }

        unsigned int line, column;
        clang_getSpellingLocation(tokenLocation, nullptr, &line, &column, nullptr);

        // Add newline escape if the line number has changed
        bool find_new_line = line != prevLine && i != 0;

        bool is_last_token = i == numTokens_1;
        bool is_double_brace =  (last_char == '}') &&  (this_char == '}');


        if (find_new_line || (is_last_token && is_class)) {

            out << "\n";

//            if(is_last_token && is_class){
//                out << "\n";
////                for( int c = 0 ; c < indent ;c++){
////                    out << " ";
////                }
//                out << " ///[MEMBER_METHOD_PLACEHOLDER]\n";
//            }

            indent = is_last_token ? 0 : column - start_column ;

            for( int c = 0 ; c < indent ;c++){
                out << " ";
            }

        }



        prevLine = line;


        if(
                (last_char != 0)
                &&(last_char != ':') && ( this_char != ':' )&& ( this_char != ';' )&& ( this_char != '(' )&& ( this_char != '{' )
                && !is_last_token
        ){
            out << " ";
        }

        out << tokenStr;// << " "; // Add space between tokens
//        if(is_last_token  && is_class){
//            out << "\n";
//            out << "///[FUNCTION_PLACEHOLDER]\n";
//        }
        last_char = tokenStr[0];
        clang_disposeString(tokenSpelling);
    }

    code = out.str();

    clang_disposeTokens(TU, tokens, numTokens);


    return std::move(code);

}

bool parser::IsRecursivelyPublic(CXCursor cursor)
{
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0)
	{
		auto access = clang_getCXXAccessSpecifier(cursor);
		if (access == CX_CXXPrivate || access == CX_CXXProtected)
		{
			return false;
		}

		if (clang_getCursorLinkage(cursor) == CXLinkage_Internal)
		{
			return false;
		}

		if (clang_getCursorKind(cursor) == CXCursor_Namespace
				&& Convert(clang_getCursorSpelling(cursor)).empty())
		{
			// Anonymous namespace.
			return false;
		}

		cursor = clang_getCursorSemanticParent(cursor);
	}

	return true;
}
