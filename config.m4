dnl Provide help text for configure
PHP_ARG_WITH(tutorial)

dnl Check --with-XXX directives
if test "$PHP_TUTORIAL" != "no"; then
  dnl Define include path that we want to usee
  PHP_ADD_INCLUDE(/usr/include/x86_64-linux-gnu/curl)

  dnl Define library path that we want to usee
  PHP_ADD_LIBRARY_WITH_PATH(curl, /usr/lib/x86_64-linux-gnu, TUTORIAL_SHARED_LIBADD)

  dnl Files to be used to build the extension
  PHP_NEW_EXTENSION(tutorial, tutorial.c, $ext_shared)

  dnl Enable shared builds of the extension
  PHP_SUBST(TUTORIAL_SHARED_LIBADD)
fi
