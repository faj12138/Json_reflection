#include <iostream>
#include <vector>

#include "CJson/cJSON.h"
#include "CJson/json_reflection_utility.h"

struct MyStruct : JsonReflectionBase
{
    int age;
    std::string name;
    std::vector<int> v_int;

    REFLECT_WEBJSON_INPUT_ARGS_NUM(age, name, v_int)
    REFLECT_WEBJSON_OUTPUT_ARGS_NUM(age, name, v_int)
};

int main() {
    std::string Jsonstr = R"({
      "age": 25,
      "name": "John",
      "v_int": [1, 2, 3]
    })";

    //from Json-to-Struct
    cJSON* cJsonInput = cJSON_Parse(Jsonstr.c_str());

    MyStruct _mystruct;
    _mystruct.parseWebJsonInput(cJsonInput);

    std::cout << "age: " << _mystruct.age << "\nname: " << _mystruct.name << std::endl;
    std::cout << "v_int: ";
    for (auto& it : _mystruct.v_int)
    {
        std::cout << it << " ";
    }
    std::cout << std::endl;

    //from Struct-to-Json
    cJSON* cJsonOutput = _mystruct.genarateWebJsonOutput();
    std::string outputStr =  cJSON_Print(cJsonOutput);
    std::cout << "output Json str: " << outputStr << std::endl;

    //release 
    cJSON_Delete(cJsonOutput);

    return 0;
}
