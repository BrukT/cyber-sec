#include "certmanager.h"

CertManager::CertManager(string cert_name){
	// read CA certificate
	X509* CA_cert;
	FILE* file = fopen((CERT_PATH + "TrustedCA_cert.pem").c_str(), "r");
    if (!file) { debug(FATAL, "cannot open CA_cert.pem" << endl); throw ExCertificate(); }
    CA_cert = PEM_read_X509(file, NULL, NULL, NULL);
	fclose(file);
    if (!CA_cert) { debug(FATAL, "cannot read CA certificate" << endl); throw ExCertificate(); }

	// read Certificate Revocation List
	X509_CRL* crl;
    file = fopen((CERT_PATH + "TrustedCA_crl.pem").c_str(), "r");
    if (!file) { debug(ERROR, "cannot open CA_crl.pem" << endl); throw ExCertificate(); }
    crl = PEM_read_X509_CRL(file, NULL, NULL, NULL);
	fclose(file);
    if (!crl) { debug(ERROR, "cannot read CRL" << endl); throw ExCertificate(); }

	// build store
    this->store = X509_STORE_new();
	if (!this->store) { debug(FATAL, "cannot create the CA store" << endl); throw ExCertificate(); }
    if (X509_STORE_add_cert(this->store, CA_cert) != 1) { debug(FATAL, "cannot add CA_cert to store" << endl); throw ExCertificate(); }
    if (X509_STORE_add_crl(this->store, crl) != 1) { debug(FATAL, "cannot add CRL to store" << endl); throw ExCertificate(); }
    X509_STORE_set_flags(this->store, X509_V_FLAG_CRL_CHECK);

	debug(INFO, "CA store created successfully" << endl);

	// read my certificate
	file = fopen((CERT_PATH + cert_name + "_cert.pem").c_str(), "r");
    if (!file) { debug(FATAL, "cannot open " + cert_name + ".pem" << endl); throw ExCertificate(); }
    this->cert = PEM_read_X509(file, NULL, NULL, NULL);
	fclose(file);
    if (!this->cert) { debug(FATAL, "can not read my certificate" << endl); throw ExCertificate(); }

	X509_free(CA_cert);
	X509_CRL_free(crl);
}

CertManager::~CertManager(){
	X509_STORE_free(this->store);
	X509_free(this->cert);
	debug(INFO, "[I] destroying CA store and certificate" << endl);
}

X509* CertManager::getCert(){
	return this->cert;
}

//Here we are also doing the client verification
int CertManager::verifyCert(X509* cert, vector<string> clientList){ 
	// verification
    X509_STORE_CTX* ctx = X509_STORE_CTX_new();

	if (!ctx) { debug(ERROR, "cannot create ctx on verifying" << endl); return -1; }
    if (X509_STORE_CTX_init(ctx, this->store, cert, NULL) != 1) {
		debug(WARNING, "[W] cannot init ctx to verify" << endl);
		goto ripper;
	}
    if (X509_verify_cert(ctx) != 1) {
		debug(WARNING, "[W] cert verification failed" << endl);
		goto ripper;
	}

	if (!clientList.empty()){
		// check subject name of the server
		X509_NAME* subject_name = X509_get_subject_name(cert);
		string str(X509_NAME_oneline(subject_name, NULL, 0));
		
		debug(INFO, "[I] cert belongs to " + str << endl);
		
		//check if the name is in the list of clients
		for(unsigned int i=0; i<clientList.size(); i++){
			if ((int)str.find("CN=" + clientList[i]) == -1) {
				debug(FATAL, "[F] server name/client name does not match" << endl);
				goto ripper;
			}
			
		}
		free(subject_name);
	}
	
	

	debug(INFO, "[I] cert verification succeded" << endl);

	/* deallocate locally used things */
	X509_STORE_CTX_free(ctx);
	return 0;

	/* there has been an error, so deallocate everything */
	ripper:
		X509_STORE_CTX_free(ctx);
		return -1;
}
