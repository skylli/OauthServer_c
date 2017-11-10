/**
 *
 * Glewlwyd OAuth2 Authorization Server
 *
 * OAuth2 authentiation server
 * Users are authenticated with a LDAP server
 * or users stored in the database 
 * Provides Json Web Tokens (jwt)
 * 
 * Resource CRUD
 *
 * Copyright 2016-2017 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include "glewlwyd.h"

/****
*description check whether the element is in the tables;
*input :
* config config
* j_resource : surce jason.
* table : table that searchr from ; table_key : table key whether value equel jason_key's value.
* return : 1 exist.
*/
int table_element_exist(struct config_elements *config, json_t *j_resource, 
                      const char *table, const char *table_key, const char *json_key){
                      
  int select_res,res = 0;
  json_t * j_query, * j_result, * j_return, * j_element, * j_scope, * j_scope_entry;

  if(!json_key || !table || !table_key || !j_resource ) return 0;
  
  //Log_dump_jason(">> find table by names", j_resource);
  if (json_is_string(json_object_get(j_resource, json_key))) {
    
          j_query = json_pack("{sss{ss}}",
                              "table",
                              table,
                              "where",
                                table_key,
                                json_string_value(json_object_get(j_resource, json_key)));
                                
           Log_dump_jason(">> find table by names", j_query);
          select_res = h_select(config->conn, j_query, &j_result, NULL);
          json_decref(j_query);
          
          if (select_res == H_OK) {
            if (json_array_size(j_result) > 0) {
              res = 1;
            }
            json_decref(j_result);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "is_resource_valid - Error executing j_query");
          }
          
    }
  return res;
}
/**
 * Get the list of all resources
 */
json_t * get_resource_list(struct config_elements * config) {
  json_t * j_query, * j_result, * j_return, * j_element, * j_scope, * j_scope_entry;
  size_t index;
  int res, i_scope;
  char * scope_clause;
  
  j_query = json_pack("{sss[sssss]}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "columns",
                        "gr_id",
                        "gr_uuid AS uuid",
                        "gr_aeskey AS aeskey",
                        "gr_name AS name",
                        "gr_description AS description");
                      //  "gr_uri AS uri");
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      if (config->use_scope) {
        scope_clause = msprintf("IN (SELECT `gs_id` FROM %s WHERE `gr_id`='%" JSON_INTEGER_FORMAT "')", GLEWLWYD_TABLE_RESOURCE_SCOPE, json_integer_value(json_object_get(j_element, "gr_id")));
        j_query = json_pack("{sss[s]s{s{ssss}}}",
                            "table",
                            GLEWLWYD_TABLE_SCOPE,
                            "columns",
                              "gs_name",
                            "where",
                              "gs_id",
                                "operator",
                                "raw",
                                "value",
                                scope_clause);
        o_free(scope_clause);
        res = h_select(config->conn, j_query, &j_scope, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          json_object_set_new(j_element, "scope", json_array());
          json_array_foreach(j_scope, i_scope, j_scope_entry) {
            json_array_append_new(json_object_get(j_element, "scope"), json_copy(json_object_get(j_scope_entry, "gs_name")));
          }
          json_decref(j_scope);
        }
      }
      json_object_del(j_element, "gr_id");
    }
    j_return = json_pack("{siso}", "result", G_OK, "resource", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_resource_list error getting resource list");
    j_return = json_pack("{si}", "result", G_ERROR_DB);
  }
  return j_return;
}

/**
 * Get a specifric resource
 */
json_t * get_resource(struct config_elements * config, const char * resource) {
  json_t * j_query, * j_result, * j_return, * j_scope, * j_scope_entry;
  int res, i_scope;
  char * scope_clause;
  
  j_query = json_pack("{sss[sssss]s{ss}}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "columns",
                        "gr_id",
                        "gr_uuid AS uuid",
                        "gr_aeskey AS aeskey",
                        "gr_name AS name",
                        "gr_description AS description",
                     //   "gr_uri AS uri",
                      "where",
                        "gr_uuid",
                        resource);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      if (config->use_scope) {
        scope_clause = msprintf("IN (SELECT `gs_id` FROM %s WHERE `gr_id`='%" JSON_INTEGER_FORMAT "')", GLEWLWYD_TABLE_RESOURCE_SCOPE, json_integer_value(json_object_get(json_array_get(j_result, 0), "gr_id")));
        j_query = json_pack("{sss[s]s{s{ssss}}}",
                            "table",
                            GLEWLWYD_TABLE_SCOPE,
                            "columns",
                              "gs_name",
                            "where",
                              "gs_id",
                                "operator",
                                "raw",
                                "value",
                                scope_clause);
        o_free(scope_clause);
        
          y_log_message(Y_LOG_LEVEL_INFO, " >>> search tables j_query %p ",j_query);
            {
            char *p = json_dumps(j_query,JSON_COMPACT);
                 if(p){
                     y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                     free(p);
                     p = NULL;
                 }
            }
        res = h_select(config->conn, j_query, &j_scope, NULL);

                y_log_message(Y_LOG_LEVEL_INFO, " >>> search tables j_query %p ",j_scope);
            {
            char *p = json_dumps(j_scope,JSON_COMPACT);
                 if(p){
                     y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                     free(p);
                     p = NULL;
                 }
            }
                
        json_decref(j_query);
        if (res == H_OK) {
          json_object_set_new(json_array_get(j_result, 0), "scope", json_array());
          json_array_foreach(j_scope, i_scope, j_scope_entry) {
            json_array_append_new(json_object_get(json_array_get(j_result, 0), "scope"), json_copy(json_object_get(j_scope_entry, "gs_name")));
          }
          json_decref(j_scope);
        }
      }
      json_object_del(json_array_get(j_result, 0), "gr_id");
      j_return = json_pack("{siso}", "result", G_OK, "resource", json_copy(json_array_get(j_result, 0)));
    } else {
      j_return = json_pack("{si}", "result", G_ERROR_NOT_FOUND);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_resource error getting scoipe list");
    j_return = json_pack("{si}", "result", G_ERROR_DB);
  }
  return j_return;
}

/*
** append category and function
*
** jason <--add--- "category":{"name":"socket","function":["turnon","turnoff","color change"]}
*
*/
json_t *_append_category_function(struct config_elements * config, json_t *j_resource){
  json_t *j_value,* j_query, * j_result, * j_return, * j_scope, * j_scope_entry,*j_ret_arry,*j_ret_value,*j_categor_id;
  int res, i_scope,i_ret; 
  
  char *p_s_ctg_name = json_string_value(json_object_get(j_resource, "category"));
  if( !p_s_ctg_name) 
    return NULL;
  
  j_return = json_object();
  j_value = json_string_nocheck(p_s_ctg_name);
  if(json_object_set_new_nocheck(j_return, "name", j_value)) {
      if(j_value)
        json_decref(j_value);
      
      return NULL;
  }  
  //Log_dump_jason("category dump :  ", j_return);
// get gctg_id from table g_category

  char *scope_clause = msprintf("IN (SELECT `gctg_id` FROM %s WHERE `gctg_name`='%s')",
     GLEWLWYD_TABLE_CATEGORY, p_s_ctg_name);
 
  j_query = json_pack("{sss[s]s{s{ssss}}}",
                     "table",
                     GLEWLWYD_TABLE_CATEGORY_FUNCTION,
                     "columns",
                       "gf_id",
                     "where",
                       "gctg_id",
                         "operator",
                         "raw",
                         "value",
                         scope_clause);
  o_free(scope_clause);
 // Log_dump_jason(">>> request ", j_query);
  
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  
 // Log_dump_jason("get gf_id from g_category_function ", j_result);
  if (res != H_OK || ! j_result || json_array_size(j_result) <= 0 ) {
      if(j_result){
        json_decref(j_result);
        j_result = NULL;
      }
      
      y_log_message(Y_LOG_LEVEL_WARNING, "not find device function");
      return NULL;
  }


  // get function from g_function  
  json_t *ret_arry = json_array();
  json_array_foreach(j_result,i_ret,j_ret_value){
    
  json_t *j_select = NULL,*jvalue=NULL;
  int index = 0;
  j_query = json_pack("{sss[s]s{si}}",
                        "table",
                        GLEWLWYD_TABLE_FUNCTION,
                        "columns",
                          "gf_name",
                          "where",
                          "gf_id",
                          json_integer_value(json_object_get(json_array_get(j_result,(size_t)i_ret),"gf_id")));
    
    res = h_select(config->conn, j_query, &j_select, NULL);
    json_decref(j_query);
    j_query = NULL;

    
    //Log_dump_jason("function dump ", j_select);
    if(res != H_OK) 
      break;
    json_array_foreach(j_select, index, jvalue){
      json_array_append_new(ret_arry, json_copy(json_object_get(jvalue, "gf_name")));
    }
    json_decref(j_select);
    j_select = NULL;
  }

  json_decref(j_result);
  j_result = NULL;
  
  //search g_category_function find gf_id: []
  //Log_dump_jason("function list ",ret_arry);
  //find function array "function":["turnon","turnoff","color change"]
  
  json_object_set_new(j_return, "function", json_copy(ret_arry));
  
  //Log_dump_jason("return  ",j_return);
  json_decref(ret_arry); 
  return j_return;
}
/**
 * Get a specifric resource
 */
json_t *get_resource_by_user(struct config_elements * config, json_t *sjson,const char *username) {

  json_t * j_query, * j_result, * j_return, * j_scope, * j_scope_entry,*j_ret_arry,*j_ret_value;
  int res, i_scope,i_ret; 
  char * scope_clause;
    //get resource id
    if ( username == NULL ){
      y_log_message(Y_LOG_LEVEL_ERROR, "get_resource error");
      return json_pack("{si}", "result", G_ERROR_DB);
    }

    scope_clause = msprintf("IN (SELECT `gu_id` FROM %s WHERE `gu_name`='%s')",
        GLEWLWYD_TABLE_USER, username);

    j_query = json_pack("{sss[s]s{s{ssss}}}",
                        "table",
                        GLEWLWYD_TABLE_USER_RESOURCE,
                        "columns",
                          "gr_id",
                        "where",
                          "gu_id",
                            "operator",
                            "raw",
                            "value",
                            scope_clause);
    o_free(scope_clause);
    Log_dump_jason("request ", j_query);
    
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    
    Log_dump_jason("j_result ", j_result);
    if (res != H_OK || ! j_result || json_array_size(j_result) <= 0 ) {
        if(j_result){
          json_decref(j_result);
          j_result = NULL;
        }
        
        y_log_message(Y_LOG_LEVEL_WARNING, "not find user uuid");
        return json_pack("{sis{ss}}", "result", G_ERROR_NOT_FOUND,"resource","reason","resource not find");
    }
  // creat result jason
  json_t *ret_arry = json_array();
  json_array_foreach(j_result,i_ret,j_ret_value){

  json_t *j_select = NULL,*jvalue=NULL;
  int index = 0;
  j_query = json_pack("{sss[sssssss]s{si}}",
                        "table",
                        GLEWLWYD_TABLE_RESOURCE,
                        "columns",
                          "gr_id",
                          "gr_uuid AS uuid",
                          "gr_aeskey AS aeskey",
                          "gr_name AS name",
                          "gr_description AS description",
                          "gr_category AS category",
                          "gr_factory AS factory",
                          //"gr_uri AS uri",
                          "where",
                          "gr_id",
                          json_integer_value(json_object_get(json_array_get(j_result,(size_t)i_ret),"gr_id")));
    
    y_log_message(Y_LOG_LEVEL_INFO, "arry index %d  arry size %d  gr_id %d ",i_ret,
      json_integer_value(json_object_get(json_array_get(j_result,0),"gr_id")));
    
    res = h_select(config->conn, j_query, &j_select, NULL);
    Log_dump_jason("dump select :: ",j_select);
    
    json_decref(j_query);
    j_query = NULL;
    if(res != H_OK) 
      break;
    json_array_foreach(j_select, index, jvalue){
      json_object_del(jvalue, "gr_id");
      // jvlue was device,so add category and function in it.
      json_t *j_category =  _append_category_function(config,jvalue);
      if(j_category){
        // add category and function 
        json_object_del(jvalue, "category");
        json_object_set_new(jvalue, "category", json_copy(j_category));
        json_decref(j_category);
      }
      json_array_append_new(ret_arry, json_copy(jvalue));
    }
    json_decref(j_select);
    j_select = NULL;
  }

  json_decref(j_result);
  j_result = NULL;
  
  Log_dump_jason(">>> search return arry ",ret_arry);
  if(json_array_size(ret_arry) > 0)
    j_return = json_pack("{sis{siso}}", "result", G_OK, "resource", "amount",json_array_size(ret_arry),"resource",json_copy(ret_arry));
  else
    j_return = json_pack("{sis{ss}}", "result", G_ERROR_NOT_FOUND,"resource","reason", "resourcd_not find");
  
  json_decref(ret_arry);
  ret_arry = NULL;

  return j_return;
}

/**
 * Get a specifric resource
 */
json_t * get_resource_by_useruuid(struct config_elements * config, json_t *sjson) {
  json_t * j_query, * j_result, * j_return, * j_scope, * j_scope_entry,*j_ret_arry,*j_ret_value;
  int res, i_scope,i_ret;
  char * scope_clause;
    //get resource id
    if (json_object_get(sjson, "user_uuid") == NULL ){
      y_log_message(Y_LOG_LEVEL_ERROR, "get_resource error getting scoipe list");
                return json_pack("{si}", "result", G_ERROR_DB);
    }

    scope_clause = msprintf("IN (SELECT `gu_id` FROM %s WHERE `gu_uuid`='%s')",
        GLEWLWYD_TABLE_USER, json_string_value(json_object_get(sjson, "user_uuid")));
    j_query = json_pack("{sss[s]s{s{ssss}}}",
                        "table",
                        GLEWLWYD_TABLE_USER_RESOURCE,
                        "columns",
                          "gr_id",
                        "where",
                          "gu_id",
                            "operator",
                            "raw",
                            "value",
                            scope_clause);
    o_free(scope_clause);

    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    
    if (res != H_OK || ! j_result || json_array_size(j_result) <= 0 ) {
        if(j_result){
          json_decref(j_result);
          j_result = NULL;
        }
        
        return json_pack("{sis{ss}}", "result", G_ERROR_NOT_FOUND,"resource","reason","user not find");
    }
    
  // creat result jason
  json_t *ret_arry = json_array();
  json_array_foreach(j_result,i_ret,j_ret_value){
    
    json_t *j_select = NULL,*jvalue=NULL;
    int index = 0;
    
    j_query = json_pack("{sss[sssss]s{si}}",
                        "table",
                        GLEWLWYD_TABLE_RESOURCE,
                        "columns",
                          "gr_id",
                          "gr_uuid AS uuid",
                          "gr_aeskey AS aeskey",
                          "gr_name AS name",
                          "gr_description AS description",
                        //  "gr_uri AS uri",
                        "where",
                          "gr_id",
                          json_integer_value(json_object_get(json_array_get(j_result,(size_t)i_ret),"gr_id")));
    
    y_log_message(Y_LOG_LEVEL_INFO, "arry index %d  arry size %d  gr_id %d ",i_ret,json_integer_value(json_object_get(json_array_get(j_result,0),"gr_id")));
    

    res = h_select(config->conn, j_query, &j_select, NULL);
    Log_dump_jason("dump select :: ",j_select);
    
    json_decref(j_query);
    j_query = NULL;
    if(res != H_OK) 
      break;
    
    json_array_foreach(j_select, index, jvalue){
      json_object_del(jvalue, "gr_id");
      json_array_append_new(ret_arry, json_copy(jvalue));
    }

    json_decref(j_select);
    j_select = NULL;
  }

  json_decref(j_result);
  j_result = NULL;
  
  Log_dump_jason(">>> search return arry ",ret_arry);
  if(json_array_size(ret_arry) > 0)
    j_return = json_pack("{sis{siso}}", "result", G_OK, "resource", "amount",json_array_size(ret_arry),"resource",json_copy(ret_arry));
  else
    j_return = json_pack("{sis{ss}}", "result", G_ERROR_NOT_FOUND,"resource","reason", "resourcd_not find");
  json_decref(ret_arry);
  ret_arry = NULL;

  Log_dump_jason(">>> search return jason ",ret_arry);

  return j_return;
}

/**
 * Check if the resource parameters are valid
 */
json_t * is_resource_valid(struct config_elements * config, json_t * j_resource, int add) {
  json_t * j_return = json_array(), * j_query, * j_result, * j_scope;
  int res;
  size_t index;
  
  if (j_return != NULL) {
    if (json_is_object(j_resource)) {
      if (add) {
        // check uuid
        if (json_is_string(json_object_get(j_resource, "uuid"))) {
          j_query = json_pack("{sss{ss}}",
                              "table",
                              GLEWLWYD_TABLE_RESOURCE,
                              "where",
                                "gr_uuid",
                                json_string_value(json_object_get(j_resource, "uuid")));
                                
          //Log_dump_jason(">> find table by names", j_query);
          res = h_select(config->conn, j_query, &j_result, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            if (json_array_size(j_result) > 0) {
              json_array_append_new(j_return, json_pack("{ss}", "name", "resource alread exist"));
            }
            json_decref(j_result);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "is_resource_valid - Error executing j_query");
          }
        }
        if (!json_is_string(json_object_get(j_resource, "name")) || json_string_length(json_object_get(j_resource, "name")) == 0 || json_string_length(json_object_get(j_resource, "name")) > 128 || strchr(json_string_value(json_object_get(j_resource, "name")), ' ') != NULL) {
          json_array_append_new(j_return, json_pack("{ss}", "name", "uuid name must be a non empty string of maximum 128 characters, without space characters"));
        }
      }
      if (json_object_get(j_resource, "description") != NULL && (!json_is_string(json_object_get(j_resource, "description")) || json_string_length(json_object_get(j_resource, "description")) > 512)) {
        json_array_append_new(j_return, json_pack("{ss}", "description", "resource description is optional and must be a string of maximum 512 characters"));
      }
      if (config->use_scope) {
        if (json_object_get(j_resource, "scope") == NULL || !json_is_array(json_object_get(j_resource, "scope"))) {
          json_array_append_new(j_return, json_pack("{ss}", "scope", "scope is a mandatory array of scope names"));
        } else {
          json_array_foreach(json_object_get(j_resource, "scope"), index, j_scope) {
            if (!json_is_string(j_scope)) {
              json_array_append_new(j_return, json_pack("{ss}", "scope", "scope name must be a string"));
            } else {
              j_result = get_scope(config, json_string_value(j_scope));
              if (check_result_value(j_result, G_ERROR_NOT_FOUND)) {
                char * message = msprintf("scope name '%s' not found", json_string_value(j_scope));
                json_array_append_new(j_return, json_pack("{ss}", "scope", message));
                o_free(message);
              } else if (!check_result_value(j_result, G_OK)) {
                y_log_message(Y_LOG_LEVEL_ERROR, "is_client_valid - Error while checking scope name '%s'", json_string_value(j_scope));
              }
              json_decref(j_result);
            }
          }
        }
      }
// remove url by sky 2017-10-18 
#if 0
      if (json_object_get(j_resource, "uri") == NULL || ((!json_is_string(json_object_get(j_resource, "uri")) || json_string_length(json_object_get(j_resource, "uri")) > 512))) {
        json_array_append_new(j_return, json_pack("{ss}", "uri", "resource uri is mandatory and must be a string of maximum 512 characters"));
      }
#endif
      // check the factory and category.
      if (json_object_get(j_resource, "category") == NULL || ((!json_is_string(json_object_get(j_resource, "category")) || json_string_length(json_object_get(j_resource, "category")) > 128))) {
        json_array_append_new(j_return, json_pack("{ss}", "uri", "resource gr_category_name is mandatory and must be a string of maximum 128 characters"));
      }
      // check whether the category being register.
      if(!table_element_exist(config,j_resource,GLEWLWYD_TABLE_CATEGORY,ELEMENT_CATEGORY_NAME,JASON_CATEGORY_KEY)){
        json_array_append_new(j_return, json_pack("{ss}",JASON_CATEGORY_KEY, "no such catagory,please connect admin"));
      }
      // check where the factory is register
       if(!table_element_exist(config,j_resource,GLEWLWYD_TABLE_FACTORY,ELEMENT_FACTORY_NAME,JASON_FACTORY_KEY)){
         json_array_append_new(j_return, json_pack("{ss}",JASON_FACTORY_KEY, "your factory haven't register,please connect admin to register."));
       }

    } else {
      json_array_append_new(j_return, json_pack("{ss}", "resource", "resource must be a json object"));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_resource_valid - Error allocating resources for j_result");
  }
  return j_return;
}
// creat "s{ss} struct"
int _append_reference_value(json_t *json,const char *key,const char *sele_key,
            const char *from_key,const char *where_key,const char *value_find)
{
    // alloc vlaue 
    char *jvalue = msprintf("(SELECT `%s` FROM `%s` WHERE `%s`='%s')",sele_key,from_key,where_key,value_find);
    if(jvalue){
        json_array_append_new(json, json_pack("{s{ss}", key,"raw",jvalue));
        return 0;
    }
    return -1;
}
/**
 * Add a new resource
 */
int add_resource(struct config_elements * config, json_t * j_resource) {
  json_t * j_query, * j_scope;
  int res, to_return;
  char * clause_scope, * escaped,*user_uuid;
  char * clause_login;
  size_t index;


  j_query = json_pack("{sss{ssssssssss}}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "values",
                      "gr_name",
                      json_string_value(json_object_get(j_resource, "name")),
                      "gr_uuid",//add gr_uuid
                      json_string_value(json_object_get(j_resource, "uuid")),//add gr_uuid value
                      "gr_aeskey",//add gr_aeskey
                      json_string_value(json_object_get(j_resource, "aeskey")),//add gr_aeskey value
                      "gr_aeskey",//add gr_aeskey
                      json_string_value(json_object_get(j_resource, "category")),//add gr_category
                      "gr_category",
                      json_string_value(json_object_get(j_resource, "factory")),//add gr_factory
                      "gr_factory",
                      json_object_get(j_resource, "description")!=NULL?json_string_value(json_object_get(j_resource, "description")):"");
                      /*"gr_uri",
                      json_object_get(j_resource, "uri")!=NULL?json_string_value(json_object_get(j_resource, "uri")):"");*/
  
  Log_dump_jason(">> select jason ", j_query);

  if (json_object_get(j_resource, "description") != NULL) {
    json_object_set_new(json_object_get(j_query, "values"), "gr_description", json_copy(json_object_get(j_resource, "description")));
  }
  
  res = h_insert(config->conn, j_query, NULL);
  json_decref(j_query);
      if (res == H_OK) {
        to_return = G_OK;
        if (json_object_get(j_resource, "scope") != NULL && config->use_scope) {
          escaped = h_escape_string(config->conn, json_string_value(json_object_get(j_resource, "uuid")));
          clause_login = msprintf("(SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
          o_free(escaped);
          escaped = NULL;
          j_query = json_pack("{sss[]}",
                              "table",
                              GLEWLWYD_TABLE_RESOURCE_SCOPE,
                              "values");
          json_array_foreach(json_object_get(j_resource, "scope"), index, j_scope) {
            escaped = h_escape_string(config->conn, json_string_value(j_scope));
            clause_scope = msprintf("(SELECT `gs_id` FROM `%s` WHERE `gs_name`='%s')", GLEWLWYD_TABLE_SCOPE, escaped);
            o_free(escaped);
            escaped = NULL;
            json_array_append_new(json_object_get(j_query, "values"), json_pack("{s{ss}s{ss}}", "gr_id", "raw", clause_login, "gs_id", "raw", clause_scope));
            o_free(clause_scope);
          }
          if (json_array_size(json_object_get(j_query, "values")) > 0) {
              char *p = json_dumps(j_query,JSON_COMPACT);
                     if(p){
                         y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                         free(p);
                         p = NULL;
                     }
            y_log_message(Y_LOG_LEVEL_INFO, ">>>> g_resource_scope");
            if ( (res =h_insert(config->conn, j_query, NULL)) != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_client_database - Error adding scope");
              to_return = G_ERROR_DB;
            }
          }
          o_free(clause_login);
          json_decref(j_query);
        }
      } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "add_resource - Error executing j_query");
    return G_ERROR_DB;
  }
      
  y_log_message(Y_LOG_LEVEL_INFO, ">>>> g_resource_scope res = %d",res);
  if (res == H_OK) {
        // insert tables g_user_resource
        if ( json_object_get(j_resource, "user_uuid") != NULL ) {
            //struct (SELECT `gr_id` FROM `g_resource` WHERE `gr_name`=input)
          escaped = h_escape_string(config->conn, json_string_value(json_object_get(j_resource, "uuid")));
          clause_login = msprintf("(SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
          o_free(escaped);
          escaped = NULL;
          
            // struct  (SELECT `gu_id` FROM `g_user` WHERE `gu_uuid`='input_Id')
          escaped = h_escape_string(config->conn, json_string_value(json_object_get(j_resource, "user_uuid")));
          user_uuid = msprintf("(SELECT `gu_id` FROM `%s` WHERE `gu_uuid`='%s')", GLEWLWYD_TABLE_USER, escaped);
          o_free(escaped);
          escaped = NULL;
          
          j_query = json_pack("{sss[]}",
                              "table",
                              GLEWLWYD_TABLE_USER_RESOURCE,
                              "values");
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{s{ss}s{ss}}", "gr_id", "raw", clause_login, "gu_id", "raw", user_uuid));
          o_free(clause_login); 
          clause_login = NULL;
          o_free(user_uuid); 
          user_uuid = NULL;
          
          if (json_array_size(json_object_get(j_query, "values")) > 0) {

          
          y_log_message(Y_LOG_LEVEL_INFO, ">>>> g_resource_user :: ");
              char *p = json_dumps(j_query,JSON_COMPACT);
                     if(p){
                         y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                         free(p);
                         p = NULL;
                     }
            if ( (res =h_insert(config->conn, j_query, NULL)) != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_client_database - Error adding scope");
              to_return = G_ERROR_DB;
            }
          }
          json_decref(j_query);
        }
    }else{
        y_log_message(Y_LOG_LEVEL_ERROR, "add_resource - Error executing j_query");
        return G_ERROR_DB;
    }
    
  return to_return;
}

/**
 * Add a new resource
* 1. insert g_resource tables.
* 2. insert g_resource_scope.
* 3. insert g_user_resource.
 */
int add_resource_by_username(struct config_elements * config, json_t * j_resource,const char *username) {

  json_t * j_query, * j_scope;
  int res, to_return;
  char * clause_scope, * escaped,*user_id;
  char * clause_login;
  int w_t_resource_ok= 0,w_t_resource_scope_ok = 0,w_t_user_resource_ok = 0; 
  size_t index;
  
  j_query = json_pack("{sss{ssssssssss}}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "values",
                        "gr_uuid",//add gr_uuid
                        json_string_value(json_object_get(j_resource, "uuid")),//add gr_uuid value
                        "gr_name",
                        json_string_value(json_object_get(j_resource, "name")),
                        "gr_aeskey",//add gr_aeskey
                        json_string_value(json_object_get(j_resource, "aeskey")),//add gr_aeskey value
                        "gr_category",
                        json_string_value(json_object_get(j_resource, "category")),//add gr_category
                        "gr_factory",
                        json_string_value(json_object_get(j_resource, "factory")),//add gr_factory
                        "gr_description",
                        json_object_get(j_resource, "description")!=NULL?json_string_value(json_object_get(j_resource, "description")):"");
                        /*"gr_uri",
                        json_object_get(j_resource, "uri")!=NULL?json_string_value(json_object_get(j_resource, "uri")):"");*/

  if (json_object_get(j_resource, "description") != NULL) {
    json_object_set_new(json_object_get(j_query, "values"), "gr_description", json_copy(json_object_get(j_resource, "description")));
  }

  
  res = h_insert(config->conn, j_query, NULL);
  
  json_decref(j_query);
      if (res == H_OK) {
        w_t_resource_ok = 1;// insert g_resource  ok
        to_return = G_OK;
        if (json_object_get(j_resource, "scope") != NULL && config->use_scope) {
          escaped = h_escape_string(config->conn, json_string_value(json_object_get(j_resource, "uuid")));
          clause_login = msprintf("(SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
          o_free(escaped);
          escaped = NULL;
          j_query = json_pack("{sss[]}",
                              "table",
                              GLEWLWYD_TABLE_RESOURCE_SCOPE,
                              "values");
          json_array_foreach(json_object_get(j_resource, "scope"), index, j_scope) {
            escaped = h_escape_string(config->conn, json_string_value(j_scope));
            clause_scope = msprintf("(SELECT `gs_id` FROM `%s` WHERE `gs_name`='%s')", GLEWLWYD_TABLE_SCOPE, escaped);
            o_free(escaped);
            escaped = NULL;
            json_array_append_new(json_object_get(j_query, "values"), json_pack("{s{ss}s{ss}}", "gr_id", "raw", clause_login, "gs_id", "raw", clause_scope));
            o_free(clause_scope);
          }
          if (json_array_size(json_object_get(j_query, "values")) > 0) {
              char *p = json_dumps(j_query,JSON_COMPACT);
                     if(p){
                         y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                         free(p);
                         p = NULL;
                     }
            if ( (res =h_insert(config->conn, j_query, NULL)) != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_client_database - Error adding scope");
              // todo you have to clean the date that have been write to databases. vary importemt.
              to_return = G_ERROR_DB;
            }
            else w_t_resource_scope_ok = 1;
          }
          o_free(clause_login);
          json_decref(j_query);
        }
      } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "add_resource - Error executing j_query");
    
    goto ADD_END;
  }
      
  if (res == H_OK) {
        // insert tables g_user_resource
        if ( username != NULL ) {
            //struct (SELECT `gr_id` FROM `g_resource` WHERE `gr_name`=input)
          escaped = h_escape_string(config->conn, json_string_value(json_object_get(j_resource, "uuid")));
          clause_login = msprintf("(SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
          o_free(escaped);
          escaped = NULL;
          
            // struct  (SELECT `gu_id` FROM `g_user` WHERE `gu_uuid`='input_Id')
          user_id = msprintf("(SELECT `gu_id` FROM `%s` WHERE `gu_name`='%s')", GLEWLWYD_TABLE_USER, username);
          
          j_query = json_pack("{sss[]}",
                              "table",
                              GLEWLWYD_TABLE_USER_RESOURCE,
                              "values");
          
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{s{ss}s{ss}}", "gr_id", "raw", clause_login, "gu_id", "raw", user_id));
          o_free(clause_login); 
          clause_login = NULL;
          o_free(user_id); 
          user_id = NULL;
          
          if (json_array_size(json_object_get(j_query, "values")) > 0) {

          
              char *p = json_dumps(j_query,JSON_COMPACT);
                     if(p){
                         y_log_message(Y_LOG_LEVEL_DEBUG, "INSERT user_scope %s",p);
                         free(p);
                         p = NULL;
                     }
            if ( (res =h_insert(config->conn, j_query, NULL)) != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_client_database - Error adding scope");
              to_return = G_ERROR_DB;
            }
            else w_t_user_resource_ok = 1;
          }
          json_decref(j_query);
        }
    }else{
        y_log_message(Y_LOG_LEVEL_ERROR, "add_resource - Error executing j_query");
        goto ADD_END;
    }
ADD_END:
  if(to_return == G_ERROR_DB){
      delete_resource(config,json_string_value(json_object_get(j_resource, "uuid")));
  }
  return to_return;
}

/**
 * Updates an existing resource
 */
int set_resource(struct config_elements * config, const char * resource, json_t * j_resource) {
  json_t * j_query, * j_scope;
  int res, to_return;
  char * clause_scope, * escaped;
  size_t index;
  char * clause_login;
  
  j_query = json_pack("{sss{ss}s{ss}}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "set",
                       // "gr_uri",
                       // json_string_value(json_object_get(j_resource, "uri")),
                        "gr_description",
                        json_object_get(j_resource, "description")!=NULL?json_string_value(json_object_get(j_resource, "description")):"",
                      "where",
                        "gr_uid",
                        resource);
  if (json_object_get(j_resource, "description") != NULL) {
    json_object_set_new(json_object_get(j_query, "set"), "gr_description", json_copy(json_object_get(j_resource, "description")));
  }
  
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    to_return =  G_OK;
    if (json_object_get(j_resource, "scope") != NULL && config->use_scope) {
      escaped = h_escape_string(config->conn, resource);
      clause_login = msprintf("= (SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
      o_free(escaped);
      j_query = json_pack("{sss{s{ssss}}}",
                          "table",
                          GLEWLWYD_TABLE_RESOURCE_SCOPE,
                          "where",
                            "gr_id",
                              "operator",
                              "raw",
                              "value",
                              clause_login);
      o_free(clause_login);
      res = h_delete(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
          escaped = h_escape_string(config->conn, resource);
          clause_login = msprintf("(SELECT `gr_id` FROM `%s` WHERE `gr_uuid`='%s')", GLEWLWYD_TABLE_RESOURCE, escaped);
          o_free(escaped);
          j_query = json_pack("{sss[]}",
                              "table",
                              GLEWLWYD_TABLE_RESOURCE_SCOPE,
                              "values");
          json_array_foreach(json_object_get(j_resource, "scope"), index, j_scope) {
            escaped = h_escape_string(config->conn, json_string_value(j_scope));
            clause_scope = msprintf("(SELECT `gs_id` FROM `%s` WHERE `gs_name`='%s')", GLEWLWYD_TABLE_SCOPE, escaped);
            o_free(escaped);
            json_array_append_new(json_object_get(j_query, "values"), json_pack("{s{ss}s{ss}}", "gr_id", "raw", clause_login, "gs_id", "raw", clause_scope));
            o_free(clause_scope);
          }
          o_free(clause_login);
          if (json_array_size(json_object_get(j_query, "values")) > 0) {
            if (h_insert(config->conn, j_query, NULL) != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_client_database - Error adding scope");
              to_return =  G_ERROR_DB;
            }
          }
          json_decref(j_query);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "set_client_database - Error deleting old scope");
        to_return =  G_ERROR_DB;
      }
      
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "set_resource - Error executing j_query");
    to_return =  G_ERROR_DB;
  }
  return to_return;
}

/**
 * Deletes an existing resource
 */
int delete_resource(struct config_elements * config, const char * resource) {
  json_t * j_query;
  int res;
  

  j_query = json_pack("{sss{ss}}",
                      "table",
                      GLEWLWYD_TABLE_RESOURCE,
                      "where",
                        "gr_uuid",
                        resource);
  
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return G_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "set_resource - Error executing j_query");
    return G_ERROR_DB;
  }
}
