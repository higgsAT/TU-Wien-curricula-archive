#ifndef MAIL_HPP
#define MAIL_HPP
#include <string>
#include <curl/curl.h>

CURLcode send_mail(const std::string &subject, const std::string &body);
#endif
