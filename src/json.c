#include <citrine/citrine.h>
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

#define CTR_OBJECT_RESOURCE_JSON 58

/**
 * @internal
 *
 * Adds json_object to resource inside object
 *
 **/
ctr_object* ctr_json_internal_addjobj(ctr_object* myself, json_object* jobj) {

	myself->info.type = CTR_OBJECT_TYPE_OTEX;

	ctr_resource* rsrc = ctr_heap_allocate(sizeof(ctr_resource));
	rsrc->type = CTR_OBJECT_RESOURCE_JSON;
	rsrc->ptr = jobj;

	myself->value.rvalue = rsrc;

	return myself;
}

/**
 * JSONObject new.
 *
 * Creates new JSONObject via json_object_new_object
 *
 **/
ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* jsonObjectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonObjectInstance->link = myself;

	return ctr_json_internal_addjobj(myself, json_object_new_object());
}

/**
 * JSON delete.
 *
 * Destroy JSONObject in variable jobj
 *
 **/
ctr_object* ctr_json_delete(ctr_object* myself, ctr_argument* argumentList) {

	int res = json_object_put((json_object*) myself->value.rvalue->ptr);

	if (res != 1) {
		ctr_argument errArgs;
		errArgs.object = ctr_build_string_from_cstring("put did not succeed in freeing the JSONObject");
		ctr_block_error(myself, &errArgs);
	}

	return myself;
}

/**
 * JSON put: [string] at: [string]
 *
 * Add simple object key-value pair to object. CAUTION: This is not the put
 * function in json-c that decrements a ref count.
 *
 **/
ctr_object* ctr_json_add(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* valObject = ctr_internal_cast2string(argumentList->object);
	char* val = ctr_heap_allocate_cstring(valObject);

	ctr_object* keyObject = ctr_internal_cast2string(argumentList->next->object);
	char* key = ctr_heap_allocate_cstring(keyObject);

	json_object_object_add((struct json_object*) myself->value.rvalue->ptr,
			       (const char*) key,
			       json_object_new_string((const char*) val)
	);

	ctr_heap_free(key);
	ctr_heap_free(val);

	return myself;
}

/**
 * JSON at: [string]
 *
 * Gets the object at the field specified
 *
 **/
ctr_object* ctr_json_get(ctr_object* myself, ctr_argument* argumentList) {

	json_object* jobj = myself->value.rvalue->ptr;

	ctr_object* keyObject = ctr_internal_cast2string(argumentList->object);
	char* key = ctr_heap_allocate_cstring(keyObject);

	json_object* robj;

	int found = json_object_object_get_ex(jobj, key, &robj);

	ctr_heap_free(key);

	if (!found) {
		ctr_argument errArgs;
		errArgs.object = ctr_build_string_from_cstring("JSON get could not locate key");
		ctr_block_error(myself, &errArgs);
		return myself;
	} else {
		ctr_object* jsonObjectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
		jsonObjectInstance->link = myself;

		return ctr_json_internal_addjobj(jsonObjectInstance, robj);
	}
}

/* Printing */

/**
 * JSON toString
 *
 * Simple conversion to string with the non-argument version of the c func
 *
 **/
ctr_object* ctr_json_tostring(ctr_object* myself, ctr_argument* argumentList) {

	const char* json_str = json_object_to_json_string((json_object*) myself->value.rvalue->ptr);

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
ctr_object* ctr_json_tostring_ext1(ctr_object* myself, ctr_argument* argumentList) {

	int flag = ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->object));

	const char* json_str = json_object_to_json_string_ext((json_object*) myself->value.rvalue->ptr, flag);

	return ctr_build_string_from_cstring((char*) json_str);
}

/**
 * JSONObject toString: [string] and: [string]
 *
 * Takes a string argument representing the string options available
 * when printing in json-c, PLAIN, PRETTY, SPACED
 *
 **/
ctr_object* ctr_json_tostring_ext2(ctr_object* myself, ctr_argument* argumentList) {

	int flags = ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->object)) |
		ctr_json_internal_strflag(ctr_internal_cast2string(argumentList->next->object));
	const char* json_str = json_object_to_json_string_ext((json_object*) myself->value.rvalue->ptr, flags);

	return ctr_build_string_from_cstring((char*) json_str);
}

/* Loading */

/**
 * @internal
 *
 * Adds the JSONObject object into the world
 **/
void begin(){

	ctr_object* jsonObject = ctr_json_new(CtrStdWorld, NULL);

	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "new" ), &ctr_json_new );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "delete" ), &ctr_json_delete );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "put:at:" ), &ctr_json_add );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "at:" ), &ctr_json_get );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString" ), &ctr_json_tostring );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:" ), &ctr_json_tostring_ext1 );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:and:" ), &ctr_json_tostring_ext2 );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "JSON" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
