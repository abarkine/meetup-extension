FROM debian:buster-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends --no-install-suggests \
    php7.3 \
    php7.3-dev \
    gcc \
    autoconf \
    automake \
    libtool \
    re2c \
    bison \
    build-essential \
    libcurl4-openssl-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /tmp/* && \
    rm -rf /var/tmp/*

WORKDIR /usr/src/extension-tutorial

COPY config.m4 tutorial.c tutorial.h ./

# Prepare the build environment for a PHP extension
RUN phpize && \
    # Make sure all of the dependencies for the rest of the build and install process are available
    ./configure && \
    # Build the project
    make && \
    # Show information about our extension
    php -d extension=modules/tutorial.so --re tutorial

ENTRYPOINT ["php", "-d", "extension=modules/tutorial.so", "-d", "tutorial.default_url=http://asil.dev", "extension.php"]
