#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.class.hpp"
#include "parser.enum.hpp"
#include "parser.function.hpp"
#include "parser.util.hpp"

using namespace reflang;

namespace
{
	std::ostream& operator<<(std::ostream& s, const CXString& str)
	{
		s << parser::Convert(str);
		return s;
	}

	CXTranslationUnit Parse(
			CXIndex& index, const std::string& file, int argc, char* argv[])
	{
		CXTranslationUnit unit = clang_parseTranslationUnit(
				index,
				file.c_str(), argv, argc,
				nullptr, 0,
				CXTranslationUnit_None);
		if (unit == nullptr)
		{
			std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
			exit(-1);
		}

		auto diagnostics = clang_getNumDiagnostics(unit);
		if (diagnostics != 0)
		{
			std::cerr << "> Diagnostics:" << std::endl;
			for (int i = 0; i != diagnostics; ++i)
			{
				auto diag = clang_getDiagnostic(unit, i);
				std::cerr << ">>> "
					<< clang_formatDiagnostic(
							diag, clang_defaultDiagnosticDisplayOptions());
			}
		}

		return unit;
	}

	struct GetTypesStruct
	{
		std::vector<std::unique_ptr<TypeBase>>* types;
		const parser::Options* options;
	};

	CXChildVisitResult GetTypesVisitor(
			CXCursor cursor, CXCursor parent, CXClientData client_data)
	{
        auto* data = reinterpret_cast<GetTypesStruct*>(client_data);
        std::string name =  parser::GetFullName(cursor);
        if ( !(!name.empty()
            && parser::IsRecursivelyPublic(cursor)
            && !(name.back() == ':')
            && regex_match(name, data->options->include)
            && !regex_match(name, data->options->exclude)))
        {
            return CXChildVisit_Recurse;
        }


		std::unique_ptr<TypeBase> type;
		switch (clang_getCursorKind(cursor))
		{
			case CXCursor_EnumDecl:
				type = std::make_unique<Enum>(parser::GetEnum(cursor));
				break;
			case CXCursor_ClassDecl:
			case CXCursor_StructDecl:
				type = std::make_unique<Class>(parser::GetClass(cursor));
				break;
			case CXCursor_FunctionDecl:
				type = std::make_unique<Function>(parser::GetFunction(cursor));
				break;
			default:
				break;
		}

		if (type)
		{
			data->types->push_back(std::move(type));
		}

		return CXChildVisit_Recurse;
	}
}  // namespace


std::vector<std::string> parser::GetSupportedTypeNames(
		const std::vector<std::string>& files,
		int argc, char* argv[],
		const Options& options)
{
	auto types = GetTypes(files, argc, argv, options);

	std::vector<std::string> names;
	names.resize(types.size());
    std::transform(types.begin(), types.end(),names.begin() ,[](auto&type){return type->GetFullName();});

	return names;
}

std::vector<std::unique_ptr<TypeBase>> parser::GetTypes(
		const std::vector<std::string>& files,
		int argc, char* argv[],
		const Options& options)
{
	std::vector<std::unique_ptr<TypeBase>> results;
	for (const auto& file : files)
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = Parse(index, file, argc, argv);

		auto cursor = clang_getTranslationUnitCursor(unit);

		GetTypesStruct data = { &results, &options };
		clang_visitChildren(cursor, GetTypesVisitor, &data);

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}
	return results;
}


void parser::print_type(const reflang::Enum &type) {
    std::cout << "Enum: " << type.GetFullName() << ", " << type.GetFile() << "\n";
    std::cout << "    Comment: " << type.GetComment() << "\n";

    std::cout << "    Values:\n";

    for (auto &v: type.Values) {
        std::cout << "        " << v.first << ", " << v.second << "\n";
    }

}

void parser::print_type(const reflang::Class &type) {
    std::cout << "Class: " << type.GetFullName() << ", " << type.GetFile() << "\n";
    std::cout << "    Comment: " << type.GetComment() << "\n";

    std::cout << "    Fields:\n";
    for (auto &v: type.Fields) {
        std::cout << "        Name: " << v.Name << ", Type: " << v.Type << "\n";
    }

    std::cout << "    Constructors:\n";
    for (auto &v: type.Constructors) {
        print_type(v);
    }

    std::cout << "    Methods:\n";
    for (auto &v: type.Methods) {
        print_type(v);

    }
    std::cout << "    StaticFields:\n";
    for (auto &v: type.StaticFields) {
        std::cout << "        Name: " << v.Name << ", Type: " << v.Type << "\n";
    }
    std::cout << "    StaticMethods:\n";
    for (auto &v: type.StaticMethods) {
        std::cout << "        Name: " << v.Name << ", ReturnType: " << v.ReturnType << "\n";
        for (auto &a: v.Arguments) {
            std::cout << "            Name: " << a.Name << ", Type: " << a.Type << "\n";
        }
    }
    std::cout << "**** Code:\n" << "///" << type.GetComment() << "\n/// TypeName:" << type.GetFullName() << "\n"<< type.GetCode() << "\n****\n";

}

void parser::print_type(const reflang::Function &type) {
    std::cout << "Function: " << type.GetFullName() << ", " << type.GetFile() << "\n";

    std::cout << "    Comment: " << type.GetComment() << "\n";

    std::cout << "    Name: " << type.Name << ", ReturnType: " << type.ReturnType << "\n";
    for (auto &a: type.Arguments) {
        std::cout << "    Name: " << a.Name << ", Type: " << a.Type << "\n";
    }

    std::cout << "**** Code:\n" << "///" << type.GetComment() << "\n/// TypeName:" << type.GetFullName() << "\n"<< type.GetCode() << "\n****\n";

}

void parser::group_types(const std::vector<std::unique_ptr<reflang::TypeBase>> &types,
                 std::vector<reflang::Enum> &supported_enum_list,
                 std::vector<reflang::Function> &supported_function_list,
                 std::vector<reflang::Class> &supported_class_list
) {
    for (const auto &type: types) {
        switch (type->GetType()) {
            case reflang::TypeBase::Type::Enum:
                supported_enum_list.emplace_back(static_cast<const reflang::Enum &>(*type));
                break;
            case reflang::TypeBase::Type::Function:
                supported_function_list.emplace_back(static_cast<const reflang::Function &>(*type));
                break;
            case reflang::TypeBase::Type::Class:
                supported_class_list.emplace_back(static_cast<const reflang::Class &>(*type));
                break;
        }
    }
}

