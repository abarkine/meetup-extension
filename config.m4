dnl config.m4
PHP_ARG_WITH(tutorial)

if test "$PHP_TUTORIAL" != "no"; then
  PHP_ADD_INCLUDE(/usr/include/x86_64-linux-gnu/curl)
  PHP_ADD_LIBRARY_WITH_PATH(curl, /usr/lib/x86_64-linux-gnu, TUTORIAL_SHARED_LIBADD)
  PHP_SUBST(TUTORIAL_SHARED_LIBADD)

  PHP_NEW_EXTENSION(tutorial, tutorial.c, $ext_shared)
fi
