#ifndef REFLANG_PARSER_UTIL_HPP
#define REFLANG_PARSER_UTIL_HPP

#include <string>

#include <clang-c/Index.h>
#include "types.hpp"

namespace reflang
{
	namespace parser
	{
		std::string Convert(const CXString& s);

		std::string GetFullName( CXCursor cursor);
		std::string GetName(const CXType& type);
		std::string GetFile(const CXCursor& cursor);
        std::string GetComment(const CXCursor& cursor);
        std::string GetCode(const CXCursor& cursor, TypeBase::Type type);

		bool IsRecursivelyPublic(CXCursor cursor);
	}
}

#endif //REFLANG_PARSER_UTIL_HPP

