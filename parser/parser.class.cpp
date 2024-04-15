#include "parser.class.hpp"

#include "parser.util.hpp"

using namespace reflang;
#include <iostream>

namespace
{
	Function GetMethodFromCursor(CXCursor cursor)
	{
		auto type = clang_getCursorType(cursor);

		Function f(
				parser::GetFile(cursor), parser::GetFullName(cursor), parser::GetComment(cursor), parser::GetCode(cursor,TypeBase::Type::Function ));
		f.Name = parser::Convert(clang_getCursorSpelling(cursor));
		int num_args = clang_Cursor_getNumArguments(cursor);
		for (int i = 0; i < num_args; ++i)
		{
			auto arg_cursor = clang_Cursor_getArgument(cursor, i);
			NamedObject arg;
			arg.Name = parser::Convert(
					clang_getCursorSpelling(arg_cursor));
			if (arg.Name.empty())
			{
				arg.Name = "nameless";
			}
			auto arg_type = clang_getArgType(type, i);
			arg.Type = parser::GetName(arg_type);
			f.Arguments.push_back(arg);
		}

		f.ReturnType = parser::GetName(clang_getResultType(type));
		return f;
	}

    Function GetConstructorFromCursor(CXCursor cursor)
    {
        auto type = clang_getCursorType(cursor);


        Function f(
                parser::GetFile(cursor), parser::GetFullName(cursor), parser::GetComment(cursor), parser::GetCode(cursor,TypeBase::Type::Function ));
        f.Name = parser::Convert(clang_getCursorSpelling(cursor));
        int num_args = clang_Cursor_getNumArguments(cursor);
        for (int i = 0; i < num_args; ++i)
        {
            auto arg_cursor = clang_Cursor_getArgument(cursor, i);
            NamedObject arg;
            arg.Name = parser::Convert(
                    clang_getCursorSpelling(arg_cursor));
            if (arg.Name.empty())
            {
                arg.Name = "nameless";
            }
            auto arg_type = clang_getArgType(type, i);
            arg.Type = parser::GetName(arg_type);
            f.Arguments.push_back(arg);
        }

        // Set the return type to the class name being constructed
        auto class_cursor = clang_getCursorSemanticParent(cursor);
        f.ReturnType = parser::GetFullName(class_cursor);

        return f;
    }
	NamedObject GetFieldFromCursor(CXCursor cursor)
	{
		NamedObject field;
		field.Name = parser::Convert(clang_getCursorSpelling(cursor));
		field.Type = parser::GetName(clang_getCursorType(cursor));
		return field;
	}

	CXChildVisitResult VisitClass(
			CXCursor cursor, CXCursor parent, CXClientData client_data)
	{
		auto* c = reinterpret_cast<Class*>(client_data);
		if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic)
		{
			switch (clang_getCursorKind(cursor))
			{
			case CXCursor_CXXMethod:
				if (clang_CXXMethod_isStatic(cursor) != 0)
				{
					c->StaticMethods.push_back(GetMethodFromCursor(cursor));
				}
				else
				{
					c->Methods.push_back(GetMethodFromCursor(cursor));
				}
				break;
			case CXCursor_FieldDecl:
				c->Fields.push_back(GetFieldFromCursor(cursor));
				break;
			case CXCursor_VarDecl:
				c->StaticFields.push_back(GetFieldFromCursor(cursor));
				break;
            case CXCursor_Constructor:
//                std::cout << " get CXCursor_Constructor"<<"\n";
                 c->Constructors.push_back(GetConstructorFromCursor(cursor));

                break;
            default:
				break;
			}
		}
		return CXChildVisit_Continue;
	}
}

Class parser::GetClass(CXCursor cursor)
{

	Class c(GetFile(cursor), GetFullName(cursor), GetComment(cursor), GetCode(cursor,TypeBase::Type::Class ));
	clang_visitChildren(cursor, VisitClass, &c);
	return c;
}
