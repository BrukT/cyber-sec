#ifndef CERTMANAGER_H
#define CERTMANAGER_H

#include <iostream>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "debug.h"
#include "exception.h"
#include "certmanager.h"

const string CERT_PATH = "cert/";

class CertManager {
    private:
        EVP_PKEY* privkey;
	    X509* cert;
        X509_STORE* store;
    public:
        CertManager(string cert_name = "server");
        ~CertManager();
        EVP_PKEY* getPrivKey();
        X509* getCert();
        int verifyCert(X509*, string name = "");
        int verifySignature(X509*, char*, int, unsigned char*, int);
};

#endif
