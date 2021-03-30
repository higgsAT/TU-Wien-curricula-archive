#include <string>
#include <iostream>
#include <curl/curl.h>

// datastream (fetched source files)
size_t write_fetched_data( void* ptr, size_t size, size_t nmemb, void * data )
{
	std::string * result = static_cast<std::string*>(data);
	*result += std::string( (char*)ptr, size*nmemb );
	return size*nmemb;
}

// filestream
size_t write_file_data( void * ptr, size_t size, size_t nmemb, FILE * stream ) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}


int main()
{
	std::string url_full = "http://phys.ik.cx/programming/cpp/curl/01/";
	std::string useragent = "spiderpig";  		// user agent string

	CURL * ch_ = curl_easy_init();   			// create a CURL handle
	char error_buffer[CURL_ERROR_SIZE];
	std::cout << error_buffer << std::endl;		// display the error log

	// SET OPTIONS
	curl_easy_setopt( ch_, CURLOPT_ERRORBUFFER, error_buffer );			// option set for error_buffer
	curl_easy_setopt( ch_, CURLOPT_WRITEFUNCTION, &write_fetched_data);	// pointer to the recieved data

	std::string result;
	curl_easy_setopt( ch_, CURLOPT_WRITEDATA, &result );			// write the data into this variable

	int id = 1;
	curl_easy_setopt( ch_, CURLOPT_VERBOSE, id );					// 1 ... a lot of verbose informations
	curl_easy_setopt( ch_, CURLOPT_URL, url_full.c_str() );
	curl_easy_setopt( ch_, CURLOPT_USERAGENT, useragent.c_str() );	// set user agent string
	curl_easy_setopt( ch_, CURLOPT_CONNECTTIMEOUT, 10);				// time(seconds) we want to be connected to the server
	curl_easy_setopt( ch_, CURLOPT_TIMEOUT, 30);					// maximum time(seconds) the transfer of the files may need
	// SET OPTIONS

	curl_easy_perform(ch_);	// start transfer with the options set above (multiple calls of this for the same handle is possible)
	curl_easy_cleanup(ch_);	// purges the handle (when crawling is done)

	std::cout << result << std::endl;





	CURL *curl;
	FILE *fp;
	CURLcode res;
	std::string fetch_pdf_url = "http://phys.ik.cx/physics/aufgabensammlung523.pdf";
	char outfilename[FILENAME_MAX] = "test.pdf";
	curl = curl_easy_init();

	if (curl)
	{
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, fetch_pdf_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	}
	return 0;









}
