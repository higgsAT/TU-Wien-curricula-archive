// mail.cpp
#include <curl/curl.h>
#include <cstring>
#include <string>
#include <iostream>
#include "../header/mail_credentials.hpp"

/*
make sure SMTP is supported.

check via the command "curl --version":

claus@raspPi1:~/Documents$ curl --version
curl 8.5.0 (aarch64-unknown-linux-gnu) libcurl/8.5.0 OpenSSL/3.0.13 zlib/1.3 brotli/1.1.0 zstd/1.5.5 libidn2/2.3.7 libpsl/0.21.2 (+libidn2/2.3.7) libssh/0.10.6/openssl/zlib nghttp2/1.59.0 librtmp/2.3 OpenLDAP/2.6.10
Release-Date: 2023-12-06, security patched: 8.5.0-2ubuntu10.8
Protocols: dict file ftp ftps gopher gophers http https imap imaps ldap ldaps mqtt pop3 pop3s rtmp rtsp scp sftp smb smbs smtp smtps telnet tftp
Features: alt-svc AsynchDNS brotli GSS-API HSTS HTTP2 HTTPS-proxy IDN IPv6 Kerberos Largefile libz NTLM PSL SPNEGO SSL threadsafe TLS-SRP UnixSockets zstd

make sure "smtp" and/or "smtps" appers under "Protocols"
*/


struct UploadState {
	const char *data;
	size_t len;
	size_t pos;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
	UploadState *upload = static_cast<UploadState*>(userp);
	size_t max = size * nmemb;
	if (upload->pos >= upload->len) return 0;
	size_t to_copy = upload->len - upload->pos;
	if (to_copy > max) to_copy = max;
	memcpy(ptr, upload->data + upload->pos, to_copy);
	upload->pos += to_copy;
	return to_copy;
}

/*
 * Sends an email via SMTPS using credentials from mail_credentials.hpp.
 * @param subject  Email subject line
 * @param body     Plain-text email body
 * @return         CURLE_OK (0) on success, non-zero CURLcode on failure
 */
CURLcode send_mail(const std::string &subject, const std::string &body) {
	const std::string payload =
		"To: " + to + "\r\n"
		"From: " + from + "\r\n"
		"Subject: " + subject + "\r\n"
		"\r\n" +
		body + "\r\n";

	CURL *curl = curl_easy_init();
	if (!curl) return CURLE_FAILED_INIT;

	UploadState upload{payload.c_str(), payload.size(), 0};
	struct curl_slist *recipients = nullptr;

	curl_easy_setopt(curl, CURLOPT_URL,          smtp_url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERNAME,     username.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD,     password.c_str());
	curl_easy_setopt(curl, CURLOPT_MAIL_FROM,    from.c_str());

	recipients = curl_slist_append(recipients, to.c_str());
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,    recipients);

	curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
	curl_easy_setopt(curl, CURLOPT_READDATA,     &upload);
	curl_easy_setopt(curl, CURLOPT_UPLOAD,       1L);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		std::cerr << "send_mail() failed: " << curl_easy_strerror(res) << "\n";

	curl_slist_free_all(recipients);
	curl_easy_cleanup(curl);
	return res;
}
