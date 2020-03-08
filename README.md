# Speakap Backend Meetup - Extension
PHP Extension building example with tagged steps

[For old version](https://github.com/phplang/extension-tutorial)

To build image
-

Run `docker build -t php/extension .`

To test extension in userland
-

Run `docker run -it --rm -v "$PWD/extension.php":/usr/src/extension-tutorial/extension.php php/extension`

Further reading and sources

 - [PHP Internals](http://www.phpinternalsbook.com/)
 - [Programming PHP - Rasmus Lerdorf](https://docstore.mik.ua/orelly/webprog/php/)
 - [PHP at the Core: A Hacker's Guide](https://www.php.net/manual/en/internals2.php)
 - [Extending and Embedding PHP - Sara Golemon](https://books.google.nl/books/about/Extending_and_Embedding_PHP.html?id=zMbGvK17_tYC)
