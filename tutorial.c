/* tutorial.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"

#include <curl/curl.h>

static zend_class_entry *curl_easy_ce = NULL;
static zend_object_handlers curl_easy_handlers;

typedef struct _curl_easy_object {
    CURL *handle;
    zend_object std;
} curl_easy_object;

static zend_object* curl_easy_to_zend_object(curl_easy_object *objval) {
    return ((zend_object*)(objval + 1)) - 1;
}
static curl_easy_object* curl_easy_from_zend_object(zend_object *objval) {
    return ((curl_easy_object*)(objval + 1)) - 1;
}

static PHP_METHOD(CurlEasy, __construct) {
    curl_easy_object *objval = curl_easy_from_zend_object(Z_OBJ_P(getThis()));
    zend_string *url = NULL;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|P", &url) == FAILURE) {
        return;
    }

    if (url) {
        CURLcode urlRet = curl_easy_setopt(objval->handle, CURLOPT_URL, ZSTR_VAL(url));
        if (urlRet != CURLE_OK) {
            zend_throw_exception(zend_ce_exception, "Failed setting URL", (zend_long)urlRet);
        }
    }
}

static PHP_METHOD(CurlEasy, setOpt) {
    curl_easy_object *objval = curl_easy_from_zend_object(Z_OBJ_P(getThis()));
    zend_long opt;
    zval *value;
    CURLcode ret = CURLE_OK;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lz", &opt, &value) == FAILURE) {
        return;
    }

    switch (opt) {
        case CURLOPT_URL: {
            zend_string *strval = zval_get_string(value);
            ret = curl_easy_setopt(objval->handle, opt, ZSTR_VAL(strval));
            zend_string_release(strval);
            break;
        }
        default:
            zend_throw_exception_ex(zend_ce_exception, opt, "Unknown curl_easy_setopt() option: %ld", (long)opt);
            return;
    }
    if (ret != CURLE_OK) {
        zend_throw_exception_ex(zend_ce_exception, ret, "Failed setting option: %ld", (long)opt);
    }
    RETURN_ZVAL(getThis(), 1, 0);
}

static PHP_METHOD(CurlEasy, perform) {
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }

    curl_easy_object *objval = curl_easy_from_zend_object(Z_OBJ_P(getThis()));
    CURLcode ret = curl_easy_perform(objval->handle);
    if (ret != CURLE_OK) {
        zend_throw_exception_ex(zend_ce_exception, ret, "Failed connection: %ld", (zend_long)ret);
    }
}

static PHP_METHOD(CurlEasy, escape) {
    zend_string *str;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &str) == FAILURE) {
        return;
    }

    char *escaped = curl_escape(ZSTR_VAL(str), ZSTR_LEN(str));
    if (!escaped) {
        zend_throw_exception_ex(zend_ce_exception, 0, "Failed escaping %s", ZSTR_VAL(str));
        return;
    }
    RETVAL_STRING(escaped);
    curl_free(escaped);
}

static zend_function_entry curl_easy_methods[] = {
    PHP_ME(CurlEasy, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, setOpt, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, perform, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, escape, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

static zend_object* curl_easy_ctor(zend_class_entry *ce) {
    curl_easy_object *objval = ecalloc(1, sizeof(curl_easy_object) + zend_object_properties_size(ce));
    objval->handle = curl_easy_init();

    zend_object* ret = curl_easy_to_zend_object(objval);
    zend_object_std_init(ret, ce);
    object_properties_init(ret, ce);
    ret->handlers = &curl_easy_handlers;

    return ret;
}

static zend_object* curl_easy_clone(zval *srcval) {
    zend_object *zsrc = Z_OBJ_P(srcval);
    zend_object *zdst = curl_easy_ctor(zsrc->ce);
    zend_objects_clone_members(zdst, zsrc);

    curl_easy_object *src = curl_easy_from_zend_object(zsrc);
    curl_easy_object *dst = curl_easy_from_zend_object(zdst);
    dst->handle = curl_easy_duphandle(src->handle);

    return zdst;
}

static void curl_easy_free(zend_object *zobj) {
    curl_easy_object *obj = curl_easy_from_zend_object(zobj);
    curl_easy_cleanup(obj->handle);

    zend_object_std_dtor(zobj);
}

static PHP_FUNCTION(tutorial_curl_version) {
    php_printf("%s\n", curl_version());
}

static PHP_FUNCTION(tutorial_curl_ver) {
    RETURN_STRING(curl_version());
}

static PHP_FUNCTION(tutorial_curl_escape) {
    char *str, *escaped;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &len) == FAILURE) {
        return;
    }

    escaped = curl_escape(str, len);
    if (escaped) {
        RETVAL_STRING(escaped);
        curl_free(escaped);
        return;
    }
}

static PHP_FUNCTION(tutorial_curl_info) {
    curl_version_info_data *info = curl_version_info(CURLVERSION_NOW);

    if (!info) {
        return;
    }

    array_init(return_value);
    add_assoc_long(return_value, "age", info->age);
    add_assoc_string(return_value, "version", info->version);
    add_assoc_long(return_value, "version_num", info->version_num);
    add_assoc_string(return_value, "host", info->host);
    add_assoc_long(return_value, "features", info->features);
    if (info->ssl_version) {
        zval ssl;
        array_init(&ssl);
        add_assoc_string(&ssl, "version", info->ssl_version);
        add_assoc_long(&ssl, "version_num", info->ssl_version_num);
        add_assoc_zval(return_value, "ssl", &ssl);
    }
    if (info->libz_version) {
        add_assoc_string(return_value, "libz", info->libz_version);
    }
    if (info->protocols) {
        const char* const* p = info->protocols;
        zval protos;
        array_init(&protos);
        while (*p) {
            add_next_index_string(&protos, *p);
            ++p;
        }
        add_assoc_zval(return_value, "protocols", &protos);
    }
}

static PHP_FUNCTION(tutorial_hello_world) {
    zend_array *arr;
    zval *tmp;
    const char *name = "Stranger";
    zend_bool greet = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "h", &arr) == FAILURE) {
        return;
    }

    tmp = zend_symtable_str_find(arr, "name", strlen("name"));
    if (tmp && (Z_TYPE_P(tmp) == IS_STRING)) {
        name = Z_STRVAL_P(tmp);
    }

    tmp = zend_symtable_str_find(arr, "greet", strlen("greet"));
    if (tmp && (Z_TYPE_P(tmp) == IS_FALSE)) {
        greet = 0;
    }

    if (greet) {
        php_printf("Hello %s\n", name);
    } else {
        php_printf("Goodbye %s\n", name);
    }
}

static PHP_FUNCTION(tutorial_greet_everyone) {
    zend_array *names;
    zend_ulong idx;
    zend_string *name;
    zval *greeting;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "h", &names) == FAILURE) {
        return;
    }

    /* foreach ($names as $name => $greeting) { */
    ZEND_HASH_FOREACH_KEY_VAL(names, idx, name, greeting)
        zend_string *strgreet = zval_get_string(greeting);
        if (name) {
            php_printf("%s %s\n", ZSTR_VAL(strgreet), ZSTR_VAL(name));
        } else {
            php_printf("%s #%ld\n", ZSTR_VAL(strgreet), (long)idx);
        }
        zend_string_release(strgreet);
    ZEND_HASH_FOREACH_END();
}

static zend_function_entry tutorial_functions[] = {
    PHP_FE(tutorial_curl_version, NULL)
    PHP_FE(tutorial_curl_ver, NULL)
    PHP_FE(tutorial_curl_escape, NULL)
    PHP_FE(tutorial_curl_info, NULL)
    PHP_FE(tutorial_hello_world, NULL)
    PHP_FE(tutorial_greet_everyone, NULL)
    PHP_FE_END
};

static PHP_MINIT_FUNCTION(tutorial) {
    if (CURLE_OK != curl_global_init(CURL_GLOBAL_DEFAULT)) {
        return FAILURE;
    }

    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Tutorial\\CURLEasy", curl_easy_methods);
    curl_easy_ce = zend_register_internal_class(&ce);
    curl_easy_ce->create_object = curl_easy_ctor;

    memcpy(&curl_easy_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    curl_easy_handlers.offset = XtOffsetOf(curl_easy_object, std);
    curl_easy_handlers.clone_obj = curl_easy_clone;
    curl_easy_handlers.free_obj = curl_easy_free;

    zend_declare_class_constant_long(curl_easy_ce, "OPT_URL", strlen("OPT_URL"), CURLOPT_URL);

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(tutorial) {
    curl_global_cleanup();

    return SUCCESS;
}

zend_module_entry tutorial_module_entry = {
    STANDARD_MODULE_HEADER,
    "tutorial",
    tutorial_functions,
    PHP_MINIT(tutorial),
    PHP_MSHUTDOWN(tutorial),
    NULL, /* RINIT */
    NULL, /* RSHUTDOWN */
    NULL, /* MINFO */
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_TUTORIAL
ZEND_GET_MODULE(tutorial)
#endif

