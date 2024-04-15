#include "types.hpp"
using namespace reflang;

TypeBase::TypeBase(std::string&& file, std::string&& full_name, std::string&& comment, std::string&& code)
:	full_name_(std::move(full_name))
,	file_(std::move(file))
        ,	comment_(std::move(comment)), code_(code)

{
}

TypeBase::~TypeBase() = default;

const std::string& TypeBase::GetFullName() const
{
	return full_name_;
}

const std::string& TypeBase::GetName() const
{
	return full_name_;
}
const std::string& TypeBase::GetComment() const
{
    return comment_;
}
const std::string& TypeBase::GetCode() const
{
    return code_;
}
const std::string& TypeBase::GetFile() const
{
	return file_;
}

Enum::Enum( std::string&& file,   std::string && full_name,   std::string&& comment ,std::string&& code)
        :	TypeBase(std::move(file), std::move(full_name), std::move(comment) ,std::move(code))
{
}

Enum::Type Enum::GetType() const
{
	return Type::Enum;
}

Function::Function( std::string&& file,   std::string && full_name,   std::string&& comment ,std::string&& code)
        :	TypeBase(std::move(file), std::move(full_name), std::move(comment) ,std::move(code))
{
}

Function::Type Function::GetType() const
{
	return Type::Function;
}

Class::Class( std::string&& file,   std::string && full_name,   std::string&& comment ,std::string&& code)
        :	TypeBase(std::move(file), std::move(full_name), std::move(comment) ,std::move(code))
{
}

Class::Type Class::GetType() const
{
	return Type::Class;
}
