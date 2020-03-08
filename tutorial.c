#include "tutorial.h"
#include "zend_exceptions.h"

#include <curl/curl.h>

/* Defining functions and variables as static because our scope is limited to this file */
static zend_class_entry *curl_easy_class_entry = NULL;
static zend_object_handlers curl_easy_handlers;

ZEND_DECLARE_MODULE_GLOBALS(tutorial);

/* Internal class to keep track of both cURL and Zend */
typedef struct _curl_easy_object {
    CURL *handle;
    zend_object std;
} curl_easy_object;

/* Type juggling methods for object interaction from/to Zend */
static zend_object* curl_easy_to_zend_object(curl_easy_object *objval) {
    return ((zend_object*)(objval + 1)) - 1;
}
static curl_easy_object* curl_easy_from_zend_object(zend_object *objval) {
    return ((curl_easy_object*)(objval + 1)) - 1;
}

static PHP_METHOD(CurlEasy, __construct) {
    curl_easy_object *objval = curl_easy_from_zend_object(Z_OBJ_P(getThis()));
    /* Accessing mdoule global */
    char *url = TUTORIALG(default_url);
    size_t url_len = 0;

    /* We are looking for a string argument */
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|p", &url, &url_len) == FAILURE) {
        return;
    }

    /* url has at least one character */
    if (url && url[0]) {
        CURLcode urlRet = curl_easy_setopt(objval->handle, CURLOPT_URL, url);
        if (urlRet != CURLE_OK) {
            zend_throw_exception(zend_ce_exception, "Failed setting URL: %ld", (zend_long)urlRet);
        }
    }
}

static PHP_METHOD(CurlEasy, setOpt) {
    curl_easy_object *objval = curl_easy_from_zend_object(Z_OBJ_P(getThis()));
    zend_long opt;
    zval *value;
    CURLcode ret = CURLE_OK;

    /* Looking for long zval */
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

    /* Returning current object */
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

/* Instead of PHP_FE we are using PHP_ME */
/* These are not simple functions, they are methods */
static zend_function_entry curl_easy_methods[] = {
    PHP_ME(CurlEasy, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, setOpt, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, perform, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(CurlEasy, escape, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

static zend_object* curl_easy_ctor(zend_class_entry *ce) {
    /* Safer calloc */
    curl_easy_object *objval = ecalloc(1, sizeof(curl_easy_object) + zend_object_properties_size(ce));
    objval->handle = curl_easy_init();

    zend_object* ret = curl_easy_to_zend_object(objval);
    zend_object_std_init(ret, ce);
    object_properties_init(ret, ce);
    /* Connect cross relation */
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
    /* Print directly to stdout */
    php_printf("%s\n", curl_version());
}

static PHP_FUNCTION(tutorial_curl_ver) {
    RETURN_STRING(curl_version());
}

static PHP_FUNCTION(tutorial_curl_escape) {
    char *str;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &len) == FAILURE) {
        return;
    }

    char* escaped = curl_escape(str, len);
    if (escaped) {
        /* Set return value */
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

    /* return_value is a magic value */
    /* If no RETURN_XXXVAL is set, return_value will be used */
    /* If it is not set, function/method will return NULL */
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

    /* Looking for HashTable */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "h", &arr) == FAILURE) {
        return;
    }

    /* Search key in HashTable */
    zval* tmp = zend_symtable_str_find(arr, "name", strlen("name"));
    const char *name = "Stranger";
    if (tmp && (Z_TYPE_P(tmp) == IS_STRING)) {
        name = Z_STRVAL_P(tmp);
    }

    tmp = zend_symtable_str_find(arr, "greet", strlen("greet"));
    zend_bool greet = 1;
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

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "h", &names) == FAILURE) {
        return;
    }

    zend_ulong idx;
    zend_string *name;
    zval *greeting;
    /* foreach ($names as ($name|$idx) => $greeting) */
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

PHP_FUNCTION(tutorial_get_default)
{
    RETVAL_STRING(TUTORIALG(default_url));
}

static zend_function_entry tutorial_functions[] = {
    PHP_FE(tutorial_curl_version, NULL)
    PHP_FE(tutorial_curl_ver, NULL)
    PHP_FE(tutorial_curl_escape, NULL)
    PHP_FE(tutorial_curl_info, NULL)
    PHP_FE(tutorial_hello_world, NULL)
    PHP_FE(tutorial_greet_everyone, NULL)
    PHP_FE(tutorial_get_default, NULL)
    PHP_FE_END
};

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY(
        "tutorial.default_url", /* Entry Name */
        "", /* Default Entry Value */
        PHP_INI_ALL, /* Entry can be changed in everywhere, other options are limit to php.ini or .htaccess or ini_set */
        OnUpdateString, /* Change modification handler */
        default_url, /* Corresponding global variable */
        zend_tutorial_globals, /* Global struct type */
        tutorial_globals /* Global struct */
    )
PHP_INI_END()

static PHP_MINIT_FUNCTION(tutorial) {
    if (CURLE_OK != curl_global_init(CURL_GLOBAL_DEFAULT)) {
        return FAILURE;
    }

    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Tutorial\\CURLEasy", curl_easy_methods);
    curl_easy_class_entry = zend_register_internal_class(&ce);
    /* Class constructor */
    curl_easy_class_entry->create_object = curl_easy_ctor;

    memcpy(&curl_easy_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    curl_easy_handlers.offset = XtOffsetOf(curl_easy_object, std);
    /* Clone operation */
    curl_easy_handlers.clone_obj = curl_easy_clone;
    /* What happens when GC arrives */
    curl_easy_handlers.free_obj = curl_easy_free;

    zend_declare_class_constant_long(curl_easy_class_entry, "OPT_URL", strlen("OPT_URL"), CURLOPT_URL);

    REGISTER_INI_ENTRIES();

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(tutorial) {
    UNREGISTER_INI_ENTRIES();
    curl_global_cleanup();

    return SUCCESS;
}

/* Called to initialize a module's globals before any module_startup_func */
static PHP_GINIT_FUNCTION(tutorial) {
    ZEND_TSRMLS_CACHE_UPDATE();
    tutorial_globals->default_url = NULL;
}

zend_module_entry tutorial_module_entry = {
    STANDARD_MODULE_HEADER,
    "tutorial", /* Module Name */
    tutorial_functions, /* Module Functions */
    PHP_MINIT(tutorial), /* Module Init */
    PHP_MSHUTDOWN(tutorial), /* Module Shutdown */
    NULL, /* Request Init */
    NULL, /* Request Shutdown */
    NULL, /* Module Info */
    "0.1", /* Module Version */
    PHP_MODULE_GLOBALS(tutorial), /* Allocate memory for globals */
    PHP_GINIT(tutorial), /* Global Init */
    NULL, /* Global Shutdown */
    NULL, /* Post Deactivate */
    STANDARD_MODULE_PROPERTIES_EX
};

/* Allow Zend to get the extension's zend_module_entry at runtime */
#ifdef COMPILE_DL_TUTORIAL
ZEND_GET_MODULE(tutorial)
#endif

