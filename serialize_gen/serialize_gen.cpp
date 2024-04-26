//
// Created by waxz on 4/12/24.
//

#include "lyra/lyra.hpp"
#include <string>
#include <vector>
#include <fstream>

#include "parser.hpp"
#include "parser.util.hpp"


#include "common/string_func.h"

/* tinyalloc */
#include "tinyalloc/tinyalloc.h"

static char heap[4*1024*1024];
static const ta_cfg_t cfg = {
        .base = heap,
        .limit = &heap[sizeof(heap)],
        .max_blocks = 256,
        .split_thresh = 16,
        .alignment = 4,
};
#define malloc(size)   ta_alloc(&cfg,size)
#define realloc(ptr, size)   ta_realloc(&cfg,ptr,size)

#define calloc(n,size) ta_calloc(&cfg,n,size)
#define free(p)        ta_free(&cfg,p)


/*
namespace Test{
    struct House {
        std::string address;
        int rooms;
        House(std::string address_,   int rooms_ ): address{address_},rooms{rooms_}{
        }
        House(){}
        House(const toml::value& value) {
            // Extract integer value from TOML value
            rooms= toml::get<decltype(rooms)>(value.at("rooms"));
            // Extract string value from TOML value
            address = toml::get<decltype(address)>(value.at("address"));
        }
        toml::value into_toml() const // you need to mark it const.
        {
            return toml::value{{"rooms", this->rooms}, {"address", this->address} };
        }
    };
    void from_json(const nlohmann::json & value,House& object ){
        from_json(value["rooms"], object.rooms);
        from_json(value["address"], object.address);
    }
    void to_json( nlohmann::json & value,const House& object ){
        to_json(value["rooms"], object.rooms);
        to_json(value["address"], object.address);
    }
}
 */


std::stringstream generate_code(const std::vector<reflang::Class>& supported_class_list ){

    std::cout << "generate_code:\n";
    std::vector<std::string> full_name;


    bool need_json = false;
    bool need_toml = false;

    std::stringstream full_implement_buffer_out;
    char* implement_buffer = (char*)malloc(10000);
    char* full_implement_buffer = (char*)malloc(10000);

    std::vector<std::stringstream > impl_code_vec;




    for(auto& type: supported_class_list){
        std::cout <<"GetComment:\n" <<  type.GetComment()<< "\n";
       const char* type_full_name = type.GetFullName().c_str();
        {
            full_name.clear();
            int idx = 0;
            char* x ;
            SPLIT_STRING(type_full_name,":",idx, x, {
                full_name.push_back(x);
            });

        }
        const char* base_name = full_name.back().c_str();


        const char* namespace_fmt = R"(
namespace %s
{

%s

}

)";


        size_t class_level = full_name.size();

        class_level = 0;
        std::string class_indent(class_level*4, ' ');
        std::string member_indent((class_level + 1)*4 -1, ' ');
        std::string member_exp_indent((class_level + 2)*4 -1, ' ');

        const char* class_indent_str = class_indent.c_str();
        const char* member_indent_str = member_indent.c_str();
        const char* member_exp_indent_str = member_exp_indent.c_str();



        std::cout << "base_name: " << base_name << "\n";

        bool find_json_in_comment = type.GetComment().find("GEN[JSON]") != std::string::npos;
        bool find_toml_in_comment = type.GetComment().find("GEN[TOML]") != std::string::npos;

        std::cout << "find_json_in_comment: " << find_json_in_comment<<"\n";
        std::cout << "find_toml_in_comment: " << find_toml_in_comment<<"\n";

        need_json = need_json || find_json_in_comment;
        need_toml = need_toml || find_toml_in_comment;
        char filed_buffer[200];


        const char* filed_from_toml_fmt = R"(%s %s= toml::get<decltype(%s)>(value.at("%s"));)";
        const char* filed_to_toml_fmt = R"(%s {"%s", this->%s})";
        const char* filed_from_json_fmt = R"(%s from_json(value["%s"], object.%s);)";
        const char* filed_to_json_fmt = R"(%s to_json(value["%s"], object.%s);)";

        const char* impl_toml_fmt = R"(

%s explicit %s(const toml::value& value) {
%s
%s }

%s toml::value into_toml() const {
%s return toml::value{
%s};
%s }
)";
        const char* impl_json_fmt = R"(
%s void from_json(const nlohmann::json & value,%s& object ){
%s
%s }


%s void to_json( nlohmann::json & value,const %s& object ){
%s
%s }
)";


        std::stringstream impl_toml_out;
        std::stringstream impl_json_out;
        if(find_toml_in_comment){


            std::stringstream fields_from_toml_out;
            std::stringstream fields_to_toml_out;

            size_t field_num = type.Fields.size();

            for(size_t i = 0 ; i < field_num; i++){
                auto& field = type.Fields[i];
                const char * name = field.Name.c_str();
                sprintf(filed_buffer,filed_from_toml_fmt,member_exp_indent_str, name,name,name);
                fields_from_toml_out << filed_buffer << "\n";

                sprintf(filed_buffer,filed_to_toml_fmt,member_exp_indent_str,name,name);
                fields_to_toml_out << filed_buffer;
                if (i < (field_num - 1)){
                    fields_to_toml_out << ",\n";
                }else{
//                    fields_to_toml_out << "\n";
                }
            }
            std::cout << "fields_to_toml_out: \n" << fields_to_toml_out.str()<<"\n";
            std::cout << "fields_from_toml_out: \n" << fields_from_toml_out.str()<<"\n";
            std::string fields_to_toml_out_str = fields_to_toml_out.str();
            std::string fields_from_toml_out_str = fields_from_toml_out.str();


            sprintf(implement_buffer,impl_toml_fmt, member_indent_str,base_name, fields_from_toml_out_str.c_str(), member_indent_str,member_indent_str,member_exp_indent_str,fields_to_toml_out_str.c_str(), member_indent_str  );
            impl_toml_out << implement_buffer;
            std::cout << "implement_buffer: \n" << implement_buffer<<"\n";

        }

        if (find_json_in_comment){


            std::stringstream fields_from_json_out;
            std::stringstream fields_to_json_out;

            for(auto& field : type.Fields){
                const char * name = field.Name.c_str();
                sprintf(filed_buffer,filed_from_json_fmt,member_indent_str, name,name);
                fields_from_json_out << filed_buffer << "\n";

                sprintf(filed_buffer,filed_to_json_fmt,member_indent_str, name,name);
                fields_to_json_out << filed_buffer << "\n";


            }
            std::cout << "fields_to_json_out: \n" << fields_to_json_out.str()<<"\n";
            std::cout << "fields_from_json_out: \n" << fields_from_json_out.str()<<"\n";
            std::string fields_to_json_out_str = fields_to_json_out.str();
            std::string fields_from_json_out_str = fields_from_json_out.str();


            sprintf(implement_buffer,impl_json_fmt, class_indent_str,base_name, fields_from_json_out_str.c_str(), class_indent_str,class_indent_str,base_name,fields_to_json_out_str.c_str(), class_indent_str  );

            std::cout << "implement_buffer: \n" << implement_buffer<<"\n";
            impl_json_out << implement_buffer;
        }

        if(find_toml_in_comment || find_json_in_comment){
            std::string code = type.GetCode();
            code.back() = ' ';

            std::stringstream impl_code;

            for(size_t i = 0 ; i < full_name.size() -1;i++){
                impl_code << "namespace " << full_name[i] << "{\n";
            }
            impl_code << "/// " << type.GetComment() << "\n";

            impl_code << code ;

            if (find_toml_in_comment){
                impl_code << impl_toml_out.str();
            }

//            if(find_json_in_comment)
            {
                bool find_empty_default_constructor = false;
                for(auto&c : type.Constructors){
                    if (c.Arguments.empty()){
                        find_empty_default_constructor = true;
                        break;
                    }
                }

                if(!find_empty_default_constructor){
                    impl_code << member_indent << base_name << "() = default;\n";
                }


            }

            impl_code << "\n};\n";
            if (find_json_in_comment){
                impl_code << impl_json_out.str();
            }

            for(size_t i = 0 ; i < full_name.size() -1;i++){
                impl_code << "}\n";
            }

            std::cout << "impl_code:\n" << impl_code.str() << "\n";
            impl_code_vec.emplace_back(std::move(impl_code));


        }

    }


    full_implement_buffer_out << "#pragma once\n";
    full_implement_buffer_out << "#include <vector>\n";
    full_implement_buffer_out << "#include <string>\n";
    full_implement_buffer_out << "#include <unordered_map>\n";

    if (need_toml){
        full_implement_buffer_out << "#include <toml.hpp>\n";
    }

    if(need_json){
        full_implement_buffer_out << "#include <nlohmann/json.hpp>\n";
    }

    for( auto& code : impl_code_vec){
            full_implement_buffer_out << code.str();
    }



    free(implement_buffer);
    free(full_implement_buffer);
    return full_implement_buffer_out;
}


int main(int argc, char **argv) {

    /* tinyalloc */
    ta_init(&cfg);

    bool get_help = false;
    std::string exe_name;
    bool list_only = false;
    std::string filter_include = ".*";
    std::string filter_exclude = "^(std|_|::).*";
    std::string output = "";
    std::vector<std::string> input_files;

    std::vector<std::string> inputs;
    // get topic xml
    auto cli
            = lyra::exe_name(exe_name)
              | lyra::help(get_help)
              | lyra::opt(list_only, "list_only")["-l"]["--list-only"]("Only list type names, don't generate")
              | lyra::opt(filter_include, "filter_include")["--include"](
                    "regex for which types to include in reflection generation")
              | lyra::opt(filter_exclude, "filter_exclude")["--exclude"](
                    "regex for which types to exclude from reflection generation")
              |
              lyra::opt(output, "output")["-o"]["--output"]("Output file path to write declarations (header) to. If empty "
                                                         "stdout is used (outputs to console).")
              | lyra::arg(input_files, "input_files")("Specify input source files");


    auto result = cli.parse({argc, argv});
    if (get_help) {
        std::cout << cli << std::endl;
        return 0;

    }


    if (!result) {
        std::cerr << "Error in command line: " << result.message() << std::endl;
        return 0;
    }
    if (input_files.empty()) {
        std::cout << "No input files specified." << std::endl;

        return 0;
    }

    std::cout << "input_files :" << input_files.size() << "\n";
    for (auto f: input_files) {
        std::cout << f << "\n";
    }
    std::cout << "inputs :" << inputs.size() << "\n";
    for (auto f: inputs) {
        std::cout << f << "\n";
    }
    reflang::parser::Options options;
    options.include = "^(" + filter_include + ")$";
    options.exclude = "^(" + filter_exclude + ")$";

    argc = 0;
    auto types = reflang::parser::GetTypes(input_files, argc, argv, options);

    std::vector<reflang::Enum> supported_enum_list;
    std::vector<reflang::Function> supported_function_list;
    std::vector<reflang::Class> supported_class_list;
    reflang::parser::group_types(types, supported_enum_list, supported_function_list,supported_class_list );





    {
        std::for_each(supported_enum_list.begin(), supported_enum_list.end(), [ ](auto& x){ reflang::parser::print_type(x) ;});
        std::for_each(supported_function_list.begin(), supported_function_list.end(), [ ](auto& x){ reflang::parser::print_type(x) ;});
        std::for_each(supported_class_list.begin(), supported_class_list.end(), [ ](auto& x){ reflang::parser::print_type(x) ;});
    }

    {
        auto code = generate_code(supported_class_list);


        if (!output.empty()){

            auto it = std::find(input_files.begin(), input_files.end(),output);
            if (it != input_files.end()){

                std::cout << "\n//==== generated code\n";
                std::cout <<  code.str() << std::endl;
                std::cout << "Warning, " << output << " appears in input_files, skip generating!!" << std::endl;
                return 0;
            }

            // Open the file for writing
            std::ofstream outFile(output.c_str());

            // Check if the file is opened successfully
            if (outFile.is_open()) {
                // Write the string to the file
                outFile << code.str();

                // Close the file
                outFile.close();

                std::cout << "String has been written to the file." << std::endl;
            } else {
                std::cerr << "Unable to open file for writing." << std::endl;
            }

        }else{
            std::cout << "\n//==== generated code\n";
            std::cout <<  code.str() << std::endl;

        }

    }


}