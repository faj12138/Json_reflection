#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define REFLECT_WEBJSON_ARGS_ELEMENT_STRING(elem) \
    #elem

#define REFLECT_WEBJSON_OUTPUT_ARGS_EXTENT_ELEM(r, data, elem) \
    succFlag &= JsonAddImp(pJson, REFLECT_WEBJSON_ARGS_ELEMENT_STRING(elem), elem);

#define REFLECT_WEBJSON_OUTPUT_ARGS_NUM(...) \
       public: \
           cJSON* genarateWebJsonOutput() const { \
               cJSON* pJson = cJSON_CreateObject(); \
               bool succFlag = true; \
               BOOST_PP_SEQ_FOR_EACH(REFLECT_WEBJSON_OUTPUT_ARGS_EXTENT_ELEM,~,BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
                  if( succFlag ) \
                  { \
                     return pJson; \
                  } \
                 cJSON_Delete(pJson); \
              return nullptr; \
            }


#define REFLECT_WEBJSON_INPUT_ARGS_EXTENT_ELEM(r, data, elem) \
    JsonGetImp(elem,webInputJson, REFLECT_WEBJSON_ARGS_ELEMENT_STRING(elem), &hasError);

#define REFLECT_WEBJSON_INPUT_ARGS_NUM(...) \
       public: \
           bool parseWebJsonInput(cJSON* webInputJson) { \
           bool hasError = false; \
           BOOST_PP_SEQ_FOR_EACH(REFLECT_WEBJSON_INPUT_ARGS_EXTENT_ELEM,~,BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
           return !hasError; \
           }
