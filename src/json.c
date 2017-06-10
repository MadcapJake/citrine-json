#include <citrine/citrine.h>
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CTR_OBJECT_RESOURCE_JSON 58

/**
 * @internal
 *
 * Throws error with object, message, and a description
 *
 **/
void ctr_json_internal_error(ctr_object* myself, char* msg, char* desc) {
	char errstr[80];
	sprintf(errstr, "%s ~ %s", msg, desc);
	ctr_argument errArgs;
	errArgs.object = ctr_build_string_from_cstring(errstr);
	ctr_block_error(myself, &errArgs);
}

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
 * JSON new.
 *
 * Creates new JSONObject via json_object_new_object
 *
 **/
ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* jsonObjectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonObjectInstance->link = myself;

	return ctr_json_internal_addjobj(jsonObjectInstance, json_object_new_object());
}

/**
 * [JSON] delete.
 *
 * Destroy JSONObject in variable jobj
 *
 **/
ctr_object* ctr_json_delete(ctr_object* myself, ctr_argument* argumentList) {

	int res = json_object_put((json_object*) myself->value.rvalue->ptr);

	if (res != 1)
		ctr_json_internal_error(myself,
					"JSON delete",
					"Did not succeed in freeing object"
		);

	return myself;
}

/**
 * [JSON] type.
 *
 * Returns 'JSON' as the type string
 *
 **/
ctr_object* ctr_json_type(ctr_object* myself, ctr_argument* argumentList) {

	return ctr_build_string_from_cstring("JSON");
}

/**
 * [JSON] put: [object] at: [string]
 *
 * Add simple object key-value pair to object. CAUTION: This is not the put
 * function in json-c that decrements a ref count.
 *
 **/
ctr_object* ctr_json_add(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object *keyObject, *putObject;
	char *putStr;
	void *val = NULL;

	putObject = argumentList->object;

	keyObject = ctr_internal_cast2string(argumentList->next->object);
	char *key = ctr_heap_allocate_cstring(keyObject);

	switch (putObject->info.type) {
		case CTR_OBJECT_TYPE_OTNIL:
			// Don't add to object?
			val = NULL;
			break;
		case CTR_OBJECT_TYPE_OTBOOL:
			val = json_object_new_boolean(putObject->value.bvalue);
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			if (ctr_number_eq(ctr_number_ceil(putObject, argumentList),
					  argumentList)) {
				// Int
				val = json_object_new_int((int) putObject->value.nvalue);
			} else {
				// Float
				val = json_object_new_double((double) putObject->value.nvalue);
			}
			break;
		case CTR_OBJECT_TYPE_OTSTRING:
			putStr = ctr_heap_allocate_cstring(putObject);
			val = json_object_new_string(putStr);
			ctr_heap_free(putStr);
			break;
		case CTR_OBJECT_TYPE_OTNATFUNC:
		case CTR_OBJECT_TYPE_OTBLOCK:
			// Run block applied to the JSON object
			argumentList->object = ctr_block_run(myself, argumentList, myself);
			ctr_json_add(myself, argumentList);
			break;
		case CTR_OBJECT_TYPE_OTOBJECT:
		case CTR_OBJECT_TYPE_OTARRAY:
		case CTR_OBJECT_TYPE_OTMISC:
		case CTR_OBJECT_TYPE_OTEX:
			// TODO: Consider supporting some kind of object here?
			putObject = ctr_internal_cast2string(argumentList->object);
			putStr = ctr_heap_allocate_cstring(putObject);

			char msg_and_args[80];
			sprintf(msg_and_args, "JSON put : %s at: '%s'", putStr, key);
			ctr_heap_free(putStr);
			ctr_heap_free(key);

			ctr_json_internal_error(myself, msg_and_args, "Cannot put that in there");
	}

	if (val) {
		struct json_object* jobj = myself->value.rvalue->ptr;
		json_object_object_add(jobj, key, val);
	}

	ctr_heap_free(key);
	/* if (val != NULL) ctr_heap_free(val); */

	return myself;
}

/**
 * @internal
 *
 * Converts jobj to citrine object.
 *
 **/
ctr_object* ctr_json_internal_j2c(json_object* j) {
	ctr_object* jsonObjectInstance;

	switch(json_object_get_type(j)) {
		case json_type_null:
			return ctr_build_nil();
		case json_type_boolean:
			return ctr_build_bool(json_object_get_boolean(j));
		case json_type_double:
			return ctr_build_number_from_float(json_object_get_double(j));
		case json_type_int:
			return ctr_build_number_from_float(json_object_get_int(j));
		case json_type_string:
			return ctr_build_string_from_cstring((char*) json_object_get_string(j));
		default:
			jsonObjectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
			jsonObjectInstance->link = ctr_internal_object_find_property(CtrStdWorld, ctr_build_string_from_cstring("JSON"), 0);
			return ctr_json_internal_addjobj(jsonObjectInstance, j);

	}
}

/**
 * [JSON] at: [string]
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

	if (!found) {
		char msg_and_key[80];
		sprintf(msg_and_key, "JSON at: '%s'", key);
		ctr_json_internal_error(myself, msg_and_key, "Could not locate key");
		return myself;
	}
	ctr_heap_free(key);

	return ctr_json_internal_j2c(robj);
}

/**
 * [JSON] map: [Block]
 *
 * Maps the json for_each key and value to Citrine block arguments. Works just
 * like the builtin map/each for Citrine maps.
 *
 **/
ctr_object* ctr_json_foreach(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	json_object* jobj;
	char *key;
	struct json_object *val;
	struct lh_entry *entry;

	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
		CtrStdFlow->info.sticky = 1;
	}
	block->info.sticky = 1;

	jobj = myself->value.rvalue->ptr;

 	for(entry = json_object_get_object(jobj)->head;
	    (entry ? (key = (char*)entry->k, val = (struct json_object*)entry->v, entry) : 0);
	    entry = entry->next) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = ctr_build_string_from_cstring(key);
		argument2->object = ctr_json_internal_j2c(val);
		argument3->object = myself;
		arguments->next = argument2;
		argument2->next = argument3;
		ctr_block_run(block, arguments, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
		ctr_heap_free( arguments );
		ctr_heap_free( argument2 );
		ctr_heap_free( argument3 );
		if (CtrStdFlow == CtrStdBreak) break;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL;
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/* Printing */

/**
 * [JSON] toString
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
 * [JSON] toString: [string] and: [string]
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

/**
 *
 * [JSON] respondTo: [string]
 *
 * Default response throws error for undefined behavior
 *
 **/
ctr_object* ctr_json_default_response(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* msgObj = ctr_internal_cast2string(argumentList->object);
	char* msg = ctr_heap_allocate_cstring(msgObj);

	ctr_json_internal_error(myself, msg, "Undefined behavior for message");

	ctr_heap_free(msg);

	return myself;
}

/* Loading */

/**
 * @internal
 *
 * Adds the JSONObject object into the world
 **/
void begin(){

	ctr_object* jsonObject = ctr_json_new(CtrStdWorld, NULL);
	/* jsonObject->link = CtrStdWorld; */

	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "new" ), &ctr_json_new );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "delete" ), &ctr_json_delete );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "type" ), &ctr_json_type );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "put:at:" ), &ctr_json_add );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "at:" ), &ctr_json_get );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "map:" ), &ctr_json_foreach );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "each:" ), &ctr_json_foreach );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString" ), &ctr_json_tostring );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:" ), &ctr_json_tostring_ext1 );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "toString:and:" ), &ctr_json_tostring_ext2 );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "respondTo:"), &ctr_json_default_response );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "JSON" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
