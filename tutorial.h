/* config.h gives the extension access to determinations made by configure */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Access PHP Macros */
#include "php.h"

/* To avoid name conflicts for global variables, use zend macro */
ZEND_BEGIN_MODULE_GLOBALS(tutorial)
    char *default_url;
ZEND_END_MODULE_GLOBALS(tutorial)

/* Produce an allusion that will allow you to access the global */
ZEND_EXTERN_MODULE_GLOBALS(tutorial)

/* Use TSRM macro to allow access global */
#define TUTORIALG(v) ZEND_MODULE_GLOBALS_ACCESSOR(tutorial, v)
