#pragma once

#include "json_reflection_arg_macros.h"
#include "JsonHandler.h"

#include <type_traits>
#include <string>
#include <boost/optional.hpp>

struct JsonReflectionBase
{
public: 
    bool parseWebJsonInput(cJSON* webInputJson) 
    {
        bool hasError = false; 
        return !hasError; 
     }

    cJSON* genarateWebJsonOutput() const { 
        cJSON* pJson = cJSON_CreateObject(); 
        bool succFlag = true; 
        if( succFlag ) 
        { 
            return pJson; 
        } 
        cJSON_Delete(pJson); 
        return nullptr; 
    }
};

template<typename T>
auto JsonAddImp( cJSON* pJson, const std::string& key, T value) -> typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<T, std::string>::value || std::is_same<T, bool>::value, bool>::type
{
    return JsonHandler::JsonAdd(pJson, key, value);
}

template<typename T>
auto JsonAddImp( cJSON* pJson, const std::string& key, const std::vector<T>& value) -> typename std::enable_if<!(std::is_base_of<JsonReflectionBase, T>::value), bool>::type
{
    return JsonHandler::JsonAddAll(pJson, key, value);
}

template<typename T>
auto JsonAddImp( cJSON* pJson, const std::string& key, T value) -> typename std::enable_if<std::is_base_of<JsonReflectionBase, T>::value, bool>::type
{
    return JsonHandler::JsonAdd(pJson, key, value.genarateWebJsonOutput());
}

template<typename T>
auto JsonAddImp( cJSON* pJson, const std::string& key, const std::vector<T>& value) -> typename std::enable_if<std::is_base_of<JsonReflectionBase, T>::value, bool>::type
{
    std::vector<cJSON*> v;
    for( auto&& x : value )
    {
        v.push_back( x.genarateWebJsonOutput() );
    }
    return JsonHandler::JsonAddAll(pJson, key, v);
}

template<typename T>
bool JsonAddImp( cJSON* pJson, const std::string& key,const boost::optional<T>& value)
{
   if( value.has_value() )
   {
       return JsonAddImp(pJson, key, value.value());
    }
   return true;
}

template<typename T>
bool JsonAddImp( cJSON* pJson, const std::string& key,const std::vector<boost::any>& value)
{
    return JsonHandler::JsonAddAll(pJson, key, value);
}

static bool JsonAddImp(cJSON* pJson, const std::string& key, const boost::any& value)
{
	if (value.type().name() == typeid(int).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<const int>(value) );
	}
	else if (value.type().name() == typeid(std::string).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<std::string>(value));
	}
	else if (value.type().name() == typeid(const char*).name())
	{
		return JsonAddImp(pJson, key, std::string(boost::any_cast<const char*>(value)));
	}
	else if (value.type().name() == typeid(std::vector<int>).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<std::vector<int>>(value));
	}
	else if (value.type().name() == typeid(std::vector<std::string>).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<std::vector<std::string>>(value));
	}
	else if (value.type().name() == typeid(std::vector<std::vector<int>>).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<std::vector<std::vector<int>>>(value));
	}
	else if (value.type().name() == typeid(float).name() )
	{
		return JsonAddImp(pJson, key, boost::any_cast<float>(value));
	}
	else if (value.type().name() == typeid(double).name())
	{
		return JsonAddImp(pJson, key, boost::any_cast<double>(value));
	}

	TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "万能类型未包括该类型 : %s", value.type().name() );
	assert(1 != 1);
	return false;
}

template<typename T>
auto JsonGetImp( T& value, cJSON* pJson, const std::string& key, bool* hasError) -> typename std::enable_if<std::is_integral<T>::value || std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, std::string>::value || std::is_same<T, bool>::value, void>::type
{
    value = JsonHandler::JsonGet<T>(pJson, key, hasError);
}

template<typename T>
auto JsonGetImp( std::vector<T>& value, cJSON* pJson, const std::string& key, bool* hasError) -> typename std::enable_if<std::is_integral<T>::value || std::is_same<T, std::string>::value, void>::type
{
    value = JsonHandler::JsonGetAll<T>(pJson, key, hasError);
}

template<typename T>
auto JsonGetImp( T& value, cJSON* pJson, const std::string& key, bool* hasError) -> typename std::enable_if< std::is_base_of<JsonReflectionBase, T>::value,void>::type
{
    cJSON* sub = JsonHandler::JsonGet<cJSON*>(pJson, key, hasError);
    bool ret = value.parseWebJsonInput(sub);
	if (!ret && hasError != nullptr)
	{
		*hasError = !ret;
	}
}

template<typename T>
auto JsonGetImp( std::vector<T>& value, cJSON* pJson, const std::string& key, bool* hasError) -> typename std::enable_if< std::is_base_of<JsonReflectionBase,T>::value, void>::type
{
    std::vector<cJSON*> subs = JsonHandler::JsonGetAll<cJSON*>(pJson, key, hasError);
	bool ret = true;
    for( auto pJson : subs )
    {
        T v;
		ret |= v.parseWebJsonInput(pJson);
        value.push_back(v);
    }
	if (!ret && hasError!=nullptr)
	{
		*hasError = !ret;
	}
}

template<typename T>
void JsonGetImp( boost::optional<T>& value, cJSON* pJson, const std::string& key, bool* hasError)
{
    cJSON* pSub = nullptr;
    if( pJson != nullptr )
    {
        pSub = cJSON_GetObjectItem(pJson, key.c_str());
    }

    if(pSub == nullptr)
    {
        value = boost::none;
    }
    else
    {
        value = T();
        JsonGetImp(value.get(), pJson, key, hasError);
    }
}
