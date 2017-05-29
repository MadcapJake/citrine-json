#include <citrine/citrine.h>
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

// Mod State
struct json_object *jobj;
int to_string_flags;

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
 * JSONObject put.
 *
 * Destroy JSONObject in variable jobj
 *
 **/
ctr_object* ctr_json_object_put(ctr_object* myself, ctr_argument* argumentList) {

	int res;

	res = json_object_put(jobj);

	if (res != 1) {
		ctr_argument errArgs;
		errArgs.object = ctr_build_string_from_cstring("put did not succeed in freeing the JSONObject");
		ctr_block_error(myself, &errArgs);
	}
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

int ctr_json_internal_strflag(ctr_object* flagObject) {
	int flag;
	char* flagString;

	flagString = ctr_heap_allocate_cstring(flagObject);

	if (strncmp(flagString, "PRETTY", 6) == 0) { flag = JSON_C_TO_STRING_PRETTY; }
	else if (strncmp(flagString, "SPACED", 6) == 0) { flag = JSON_C_TO_STRING_SPACED; }
        else { flag = JSON_C_TO_STRING_PLAIN; }

	ctr_heap_free(flagString);

	return flag;
}

/**
 * JSONObject toString: [string]
 *
 * Takes a string argument representing the string options available
 * when printing in json-c, PLAIN, PRETTY, SPACED
 *
 **/
ctr_object* ctr_json_object_to_json_string_ext1(ctr_object* myself, ctr_argument* argumentList) {

	int flag;

	flag = ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->object));

	const char* json_str = json_object_to_json_string_ext(jobj, flag);

	return ctr_build_string_from_cstring((char*) json_str);
}

/**
 * JSONObject toString: [string] and: [string]
 *
 * Takes a string argument representing the string options available
 * when printing in json-c, PLAIN, PRETTY, SPACED
 *
 **/
ctr_object* ctr_json_object_to_json_string_ext2(ctr_object* myself, ctr_argument* argumentList) {

	int flags;

	flags = ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->object)) |
		ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->next->object));
	const char* json_str = json_object_to_json_string_ext(jobj, flags);

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
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "put" ), &ctr_json_object_put );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "addObject:value:" ), &ctr_json_object_add );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString" ), &ctr_json_object_to_json_string );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:" ), &ctr_json_object_to_json_string_ext1 );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:and:" ), &ctr_json_object_to_json_string_ext2 );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "JSONObject" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
