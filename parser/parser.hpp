#ifndef REFLANG_PARSER_HPP
#define REFLANG_PARSER_HPP

#include <memory>
#include <string>
#include <vector>
#include <regex>

#include "types.hpp"

namespace reflang
{
	namespace parser
	{
		struct Options
		{
			std::regex include;
			std::regex exclude;
		};

		std::vector<std::string> GetSupportedTypeNames(
				const std::vector<std::string>& files,
				int argc, char* argv[],
				const Options& options = Options());

		std::vector<std::unique_ptr<TypeBase>> GetTypes(
				const std::vector<std::string>& files,
				int argc, char* argv[],
				const Options& options = Options());

        void print_type(const reflang::Enum &type);
        void print_type(const reflang::Function &type);
        void print_type(const reflang::Class &type);
        void group_types(const std::vector<std::unique_ptr<reflang::TypeBase>> &types,
                         std::vector<reflang::Enum> &supported_enum_list,
                         std::vector<reflang::Function> &supported_function_list,
                         std::vector<reflang::Class> &supported_class_list
        );
	};
}

#endif //REFLANG_PARSER_HPP
