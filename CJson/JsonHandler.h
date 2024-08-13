#pragma once

#include "cJSON.h"

#include <string>
#include <memory>
#include <type_traits>
#include <vector>
#include <boost/any.hpp>

#include "../mylog.h"

class JsonHandler
{
public:
	template<typename T>
	 static T JsonGet(cJSON* pJson, const std::string& key, bool* hasError = 0)
	{
		cJSON* pTargetJson = NULL;
		if (pJson != NULL)
		{
			pTargetJson = cJSON_GetObjectItem(pJson, const_cast<char*>(key.c_str()));
		}
		else
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get key=%s, but pJson == nullptr", key.c_str());
		}

		T ret = JsonGet<T>(pTargetJson, hasError);
		if (hasError && *hasError == true)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get key=%s ,   failed!!!", key.c_str());
		}
		return ret;
	}

	template<typename T>
	 static T JsonGet(cJSON* pJson, const int index, bool* hasError = 0)
	{
		if (pJson == NULL)
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get index=%d, but pJson == nullptr", index);
			return NULL;
		}
		
		cJSON* pTargetJson = cJSON_GetArrayItem(pJson, index);
		if (pTargetJson == nullptr)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get index=%d  failed, may be index too large", index);
		}

		T ret = JsonGet<T>(pTargetJson, hasError);
		if (hasError && *hasError == true)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get index=%d  failed !!!", index);
		}
		return ret;
	}

	template<typename T>
	 static std::vector<T> JsonGetAll(cJSON* pJson, bool* hasError = 0)
	{
		std::vector<T> v;
		if (pJson != NULL && pJson->type == cJSON_Array)
		{
			int len = cJSON_GetArraySize(pJson);
			v.reserve(len);
			for (int i = 0; i < len; ++i)
			{
				auto&& value = JsonGet<T>(pJson, i, hasError);
				v.push_back(value);
			}
		}
		else
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get array failed !!!  %s", pJson == nullptr ? " pJson == nullptr" : "pJson is not a array");
		}
		return v;
	}

	template<typename T>
	 static std::vector<T> JsonGetAll(cJSON* pJson, const std::string& key, bool* hasError = 0)
	{
		cJSON* arrayJson = JsonGet<cJSON*>(pJson, key, hasError);
		std::vector<T> ret = JsonGetAll<T>(arrayJson, hasError);
		if (hasError && *hasError == true)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "get array whose key=%s ,   failed!!!", key.c_str());
		}
		return ret;
	}

	 static std::string JsonToString(cJSON* pJson)
	{
		std::string strRet;
		if (pJson != nullptr)
		{
			char* ptr = cJSON_Print(pJson);
			if (ptr != nullptr)
			{
				strRet = ptr;
				free(ptr);
			}
		}
		else
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "JsonToString, but pJson == nullptr");
		}
		return strRet;
	}

	 static std::string JsonToString(std::shared_ptr<cJSON>& pJson)
	{
		return JsonToString(pJson.get());
	}

	template<typename T>
	 static auto JsonAdd(cJSON* pJson, const std::string& key, T value)
		->typename std::enable_if<(std::is_integral<T>::value || std::is_floating_point<T>::value) && sizeof(T) <= 8, bool>::type
	{
		if (!pJson || pJson->type != cJSON_Object)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed! %s", key.c_str(), pJson ? "pJson type invaild": "pJson is nullptr");
			return false;
		}

		cJSON* number = cJSON_CreateNumber(value);
		bool ret = JsonAdd(pJson, key, number);
		if (!ret)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed!!!", key.c_str());
		}
		return ret;
	}

	static bool JsonAdd(cJSON* pJson, const std::string& key, const std::string& value)
	{
		if (!pJson || pJson->type != cJSON_Object)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed! %s", key.c_str(), pJson ? "pJson type invaild" : "pJson is nullptr");
			return false;
		}

		cJSON* str = cJSON_CreateString(value.c_str());

		bool ret = JsonAdd(pJson, key, str);
		if (!ret)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed!", key.c_str());
		}
		return ret;
	}

	static bool JsonAdd(cJSON* pJson, const std::string& key, bool value)
	{
		if (!pJson || pJson->type != cJSON_Object)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed! %s", key.c_str(), pJson ? "pJson type invaild" : "pJson is nullptr");
			return false;
		}

		if (value)
		{
			cJSON_AddTrueToObject(pJson, key.c_str());
		}
		else
		{
			cJSON_AddFalseToObject(pJson, key.c_str());
		}

		return true;
	}

	 static bool JsonAdd(cJSON* pJson, const std::string& key, cJSON* value)
	{
		if (!pJson || pJson->type != cJSON_Object)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed! %s", key.c_str(), pJson ? "pJson type invaild" : "pJson is nullptr");
			return false;
		}

		if (!value)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add  key=%s failed! %s", key.c_str(), "value(cJSON*) is nullptr");
			return false;
		}

		cJSON_AddItemToObject(pJson, key.c_str(), value);
		return true;
	}

	template<typename T>
	 static bool JsonAddAll(cJSON* pJson, const std::string& key, const std::vector<T>& value)
	{
		if (!pJson || pJson->type != cJSON_Object )
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add array whose key=%s , %s", key.c_str(), pJson ?  "pJson type invaild": "pJson == nullptr");
			return false;
		}

		cJSON* pArray = cJSON_CreateArray();
		if (pArray == nullptr )
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "cJSON_CreateArray return nullptr");
			return false;
		}

		for ( auto& x: value)
		{
			bool ret = JsonAdd(pArray, x);
			if (!ret)
			{
				TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add value to array failed !array's key=%s ", key.c_str());
				SafeDeleteImp<T>(pArray); //如果是cJSON*，则清空pArray，这样为了让指针给传入者自己去管理
				cJSON_Delete(pArray); 
				return false;
			}
		}

		cJSON_AddItemToObject(pJson, key.c_str(), pArray);
		return true;
	}
private:
	template<typename T>
	static auto JsonGet(cJSON* pJson, bool* hasError = 0)->
		typename std::enable_if<std::is_same<int, T>::value || std::is_same<unsigned int, T>::value, T>::type
	{
		if (!pJson || pJson->type != cJSON_Number)
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "value is not a number type, return default 0");
			return 0;
		}
		else
		{
			return pJson->valueint;
		}
	}

	template<typename T>
	static auto JsonGet(cJSON* pJson, bool* hasError = 0)->
		typename std::enable_if<std::is_same<bool, T>::value, T>::type
	{
		if (!pJson || (pJson->type != cJSON_False && pJson->type != cJSON_True))
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "value is not a bool type, return default false");
			return false;
		}
		else
		{
			return pJson->type == cJSON_False ? false : true;
		}
	}

	template<typename T>
	static auto JsonGet(cJSON* pJson, bool* hasError = 0)->
		typename std::enable_if<std::is_same<long long, T>::value || std::is_same<unsigned long long, T>::value || std::is_floating_point<T>::value, T>::type
	{
		if (!pJson || pJson->type != cJSON_Number)
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "value is not a number type, return default 0");
			return 0;
		}
		else
		{
			return pJson->valuedouble;
		}
	}

	template<typename T>
	static auto JsonGet(cJSON* pJson, bool* hasError = 0)->typename std::enable_if<std::is_same<std::string, T>::value, T>::type
	{
		if (!pJson || pJson->type != cJSON_String)
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "value is not a string type, return empty string");
			return "";
		}
		else
		{
			return pJson->valuestring;
		}
	}

	template<typename T>
	static auto JsonGet(cJSON* pJson, bool* hasError = 0)->typename std::enable_if<std::is_same<cJSON*, T>::value, T>::type
	{
		if (!pJson || pJson->type != cJSON_Array && pJson->type != cJSON_Object)
		{
			setError(hasError);
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "value is not a array type or a object type");
			return NULL;
		}
		else
		{
			return pJson;
		}
	}

	static void setError(bool* hasError)
	{
		if (hasError == NULL)
		{
			return;
		}
		else
		{
			*hasError = true;
		}
	}

	template<typename T>
	static auto JsonAdd(cJSON* pJson, T value)
		->typename std::enable_if<(std::is_integral<T>::value || std::is_floating_point<T>::value) && sizeof(T) <= 8, bool>::type
	{
		if (pJson->type != cJSON_Array)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "this function only can add a value to a array");
			return false;
		}
		cJSON* pJsonNumber = cJSON_CreateNumber(value);
		if (pJsonNumber == nullptr)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "cJSON_CreateNumber return nullptr");
			return false;
		}
		cJSON_AddItemToArray(pJson, pJsonNumber);
		return true;
	}

	static bool JsonAdd(cJSON* pJson, const std::string& value)
	{
		if (pJson == nullptr || pJson->type != cJSON_Array)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "this function only can add a value to a array");
			return false;
		}
		cJSON* pJsonString= cJSON_CreateString(value.c_str());
		if (pJsonString == nullptr)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "cJSON_CreateString return nullptr");
			return false;
		}
		cJSON_AddItemToArray(pJson, pJsonString);
		return true;
	}

	static bool JsonAdd(cJSON* pJson, cJSON* value)
	{
		if (pJson == nullptr || value == nullptr || pJson->type != cJSON_Array)
		{
			TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "%s", value == nullptr ? "value is a nullptr":"cJSON_CreateString return nullptr");
			return false;
		}
		cJSON_AddItemToArray(pJson, value);
		return true;
	}

    static bool JsonAdd(cJSON* pJson, const boost::any& value)
    {
        if (value.type().name() == typeid(int).name())
        {
            return JsonAdd(pJson, boost::any_cast<const int>(value) );
        }
        else if (value.type().name() == typeid(std::string).name())
        {
            return JsonAdd(pJson, boost::any_cast<std::string>(value));
        }
        else if (value.type().name() == typeid(const char*).name())
        {
            return JsonAdd(pJson,std::string(boost::any_cast<const char*>(value)));
        }

       TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "can‘t not support this any type: %s", value.type().name());
        return false;
    }

    //重复调用解数组
    template<typename T>
    static auto JsonAdd(cJSON* pJson, const std::vector<T>& value)->typename std::enable_if<!(std::is_integral<T>::value || std::is_floating_point<T>::value), bool>::type
    {
        if (!pJson || pJson->type != cJSON_Array )
        {
            TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add array , %s", pJson ?  "pJson type invaild": "pJson == nullptr");
            return false;
        }

        cJSON* pArray = cJSON_CreateArray();
        if (pArray == nullptr )
        {
            TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "cJSON_CreateArray return nullptr");
            return false;
        }

        for ( auto& x: value)
        {
            bool ret = JsonAdd(pArray, x);
            if (!ret)
            {
                TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add value to array failed !");
                SafeDeleteImp<T>(pArray); //??????cJSON*????????pArray??????????????????????????????
                cJSON_Delete(pArray);
                return false;
            }
        }

        cJSON_AddItemToArray(pJson, pArray);
        return true;
    }

    //不再嵌套层
    template<typename T>
    static auto JsonAdd(cJSON* pJson, const std::vector<T>& value)->typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, bool>::type
    {
        if (!pJson || pJson->type != cJSON_Array )
        {
            TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add array , %s", pJson ?  "pJson type invaild": "pJson == nullptr");
            return false;
        }

        cJSON* pArray = cJSON_CreateArray();
        if (pArray == nullptr )
        {
            TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "cJSON_CreateArray return nullptr");
            return false;
        }

        for ( auto& x: value)
        {
            bool ret = JsonAdd(pArray, x);
            if (!ret)
            {
                TRACE_LOG_A(nullptr, TRACE_LOG_LEVEL_ERROR, "add value to array failed !");
                SafeDeleteImp<T>(pArray); //??????cJSON*????????pArray??????????????????????????????
                cJSON_Delete(pArray);
                return false;
            }
        }

        cJSON_AddItemToArray(pJson, pArray);
        return true;
    }

	template<typename T>
	static void SafeDeleteImp(cJSON*){}
	
	template<>
	static void SafeDeleteImp<cJSON*>(cJSON* pJsonArray)
	{
		memset(pJsonArray, 0, sizeof(cJSON));
	}
};