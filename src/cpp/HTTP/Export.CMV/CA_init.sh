##!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020 Frank Eskesen.
##
##       This file is free content, distributed under the "un-license,"
##       explicitly released into the Public Domain.
##       (See accompanying file LICENSE.UNLICENSE or the original
##       contained within http://unlicense.org)
##
##----------------------------------------------------------------------------
##
## Title-
##        CA_init.sh
##
## Function-
##        Initialize key subdirectory.
##
## Last change date-
##        2020/01/20
##
## Prerequisite-
##        ~/bat (Part of this distribution) in $PATH
##
## Usage-
##        Use the same password everywhere you're asked.
##        This key repository is only for testing. A simple password is OK.
##
##        cd ~/obj/cpp/HTTP/Export.CMV/key
##        CA build
##        ../CA_init.sh
##
##        Use "CA reset" to empty this subdirectory.
##
##############################################################################

##############################################################################
## Initialize
[[ -f "./openssl.cnf" ]] || { echo "Not initialized, need CA build"; exit 1; }

##############################################################################
## Initialize environment
export OPENSSL_CONF=$PWD/openssl.cnf
echo "OPENSSL_CONF=$OPENSSL_CONF"

##############################################################################
## Initialize the key subdirectory
echo ""
echo "Creating server CA"
echo -e "\n\n\n\nAdministration\nServer CA\n\n\n\n" | \
openssl req -newkey rsa:1024 -sha1 -keyform PEM -outform PEM \
        -keyout serverCAkey.pem -out serverCAreq.pem 2>/dev/null
# openssl req -in serverCAreq.pem -noout -text ## (Display request)

echo ""
echo "Signing server CA with root CA"
openssl ca -in serverCAreq.pem -out serverCAcert.pem -extensions v3_ca
# Following does not work: serverCA.pem is not a CA
# openssl x509 -req -sha1 -extensions v3_ca \
#         -CA cacert.pem -CAkey private/cakey.pem -CAcreateserial \
#         -in serverCAreq.pem -out serverCAcert.pem
cat serverCAcert.pem cacert.pem >serverCA.pem
openssl x509 -in serverCA.pem -noout -text -purpose

##############################################################################
echo ""
echo "Creating server certificate"
echo -e "\n\n\n\nOperations\nlocalhost.org\n\n\n\n" | \
openssl req -newkey rsa:1024 -sha1 -keyform PEM -outform PEM \
        -keyout serverkey.pem -out serverreq.pem 2>/dev/null

echo ""
echo "Signing server certificate with SERVER CA"
openssl x509 -req -sha1 -extensions usr_cert \
        -CA serverCA.pem -CAkey serverCAkey.pem -CAcreateserial \
        -in serverreq.pem -out servercert.pem
# cat servercert.pem serverCAcert.pem cacert.pem >server.pem
cat servercert.pem serverkey.pem serverCAcert.pem cacert.pem >server.pem
openssl x509 -in server.pem -noout -text -purpose

##############################################################################
echo ""
echo "Creating client certificate"
echo -e "\n\n\n\nOperations\nlocalhost.org\n\n\n\n" | \
openssl req -newkey rsa:1024 -sha1 -keyform PEM -outform PEM \
        -keyout clientkey.pem -out clientreq.pem 2>/dev/null

echo ""
echo "Signing client certificate with ROOT CA"
openssl x509 -req -sha1 -extensions usr_cert \
        -CA cacert.pem -CAkey private/cakey.pem -CAcreateserial \
        -in clientreq.pem -out clientcert.pem
# cat clientcert.pem cacert.pem >client.pem
cat clientcert.pem clientkey.pem cacert.pem >client.pem
openssl x509 -in client.pem -noout -text -purpose

##############################################################################
echo ""
echo "Creating dh512.pem and dh1024.pem"
openssl dhparam -check -text -5 -out dh512.pem   512
openssl dhparam -check -text -5 -out dh1024.pem 1024

## Initialization complete
echo ""
echo "Initialization complete"

