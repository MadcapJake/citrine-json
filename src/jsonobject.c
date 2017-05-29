#include <citrine/citrine.h>
#include <json-c/json.h>
#include <stdio.h>

struct json_object *jobj;

/**
 * JSONObject new.
 *
 * Hackishly creates a new JSON object
 *
 **/
ctr_object* ctr_json_object(ctr_object* myself, ctr_argument* argumentList) {

	jobj = json_object_new_object();

	return myself;
}

/**
 * JSONObject addObject: [string] value: [string]
 *
 * Add simple object key-value pair to object
 *
 **/
ctr_object* ctr_json_object_add(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* keyObject = ctr_internal_cast2string(argumentList->object);
	char* key = ctr_heap_allocate_cstring(keyObject);

	ctr_object* valObject = ctr_internal_cast2string(argumentList->next->object);
	char* val = ctr_heap_allocate_cstring(valObject);

	json_object_object_add(jobj, (const char*) key, json_object_new_string((const char*) val));

	ctr_heap_free(key);
	ctr_heap_free(val);

	return myself;
}

/**
 * JSONObject toString
 *
 * Simple conversion to string with the non-argument version of the c func
 *
 **/
ctr_object* ctr_json_object_to_json_string(ctr_object* myself, ctr_argument* argumentList) {

	const char* json_str = json_object_to_json_string(jobj);

	return ctr_build_string_from_cstring((char*) json_str);
}

/**
 * @internal
 *
 * Adds the JSONObject object into the world
 **/
void begin(){
	ctr_object* jsonObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonObject->link = CtrStdObject;
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "new" ), &ctr_json_object );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "addObject:value:" ), &ctr_json_object_add );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString" ), &ctr_json_object_to_json_string );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "JSONObject" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
