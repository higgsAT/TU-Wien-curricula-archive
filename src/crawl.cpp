#include <string>
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <vector>

// datastream (fetched source files) used by fetch_source_single_page
size_t write_fetched_data(void* ptr, size_t size, size_t nmemb, void* data)
{
	std::string* result = static_cast<std::string*>(data);
	*result += std::string((char*)ptr, size* nmemb);
	return size*nmemb;
}

// fetch the source code of a single page
std::string fetch_source_single_page(std::string url_full)
{
	std::string useragent = "spiderpig";		// user agent string

	CURL* ch_ = curl_easy_init();				// create a CURL handle
	char error_buffer[CURL_ERROR_SIZE];
	std::cout << error_buffer << std::endl;		// display the error log

	// SET OPTIONS
	curl_easy_setopt(ch_, CURLOPT_ERRORBUFFER, error_buffer);			// option set for error_buffer
	curl_easy_setopt(ch_, CURLOPT_WRITEFUNCTION, &write_fetched_data);	// pointer to the recieved data

	std::string result;
	curl_easy_setopt(ch_, CURLOPT_WRITEDATA, &result);					// write the data into this variable

	int id = 1;
	curl_easy_setopt(ch_, CURLOPT_VERBOSE, id);							// 1 ... a lot of verbose informations
	curl_easy_setopt(ch_, CURLOPT_URL, url_full.c_str());
	curl_easy_setopt(ch_, CURLOPT_USERAGENT, useragent.c_str());		// set user agent string
	curl_easy_setopt(ch_, CURLOPT_CONNECTTIMEOUT, 10);					// time(seconds) we want to be connected to the server
	curl_easy_setopt(ch_, CURLOPT_TIMEOUT, 30);							// maximum time(seconds) the transfer of the files may need
	// SET OPTIONS

	curl_easy_perform(ch_);	// start transfer with the options set above (multiple calls of this for the same handle is possible)
	curl_easy_cleanup(ch_);	// purges the handle (when crawling is done)

	return result;
}


// filestream used by fetch_PDF_from_URL
size_t write_file_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}


// download a PDF given by an URL
void fetch_PDF_from_URL(std::string fetch_pdf_url)
{
	CURL* curl;
	FILE* fp;
	CURLcode res;

	char outfilename[FILENAME_MAX] = "test.pdf";
	curl = curl_easy_init();

	if (curl)
	{
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, fetch_pdf_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);

		// cleanup
		curl_easy_cleanup(curl);
		fclose(fp);
	}
}


// starting from the obtained page source code, this function extracts the links to the PDFs as well as the information about the PDFs. The obtained
// information is stored in the two std::vectors extracted_data1 and extracted_data2
void extract_PDF_URL_and_descriptions(std::string result, std::vector<std::string>& fetched_URLs, std::vector<std::string>& fetched_URLs_descriptions)
{
	// extract the links and descriptions of the PDFs
	int shift_pos = 0;

	// iterate the std::string and find the desired elements
	for (int i = 0; i < result.length()-8; i++)
	{
		// rolling comparison to find open '<a href="' tags
		int ahref_compare = result.compare(i, 9, "<a href=\"");

		// extract the target URL to the PDF and the description according to the page source code
		if (ahref_compare == 0)	// found an opening '<a href="' tag
		{
			bool close_URL = false;
			i += 9;							// increment variable i by 9 to set the position at the end of the opening bracket
			int start_extract_pos_URL = i;	// cache the start position

			// extract the link to the PDF
			std::string extract_URL;	// stores the link to the PDF

			while (close_URL == false)
			{
				i++;

				if (result[i] == '\"')	// end of URL reached
				{
					close_URL = true;
					extract_URL.append(result.begin()+start_extract_pos_URL, result.begin()+i);
					fetched_URLs.push_back(extract_URL);
				}
				else if (i == result.length())	// error .. end of string while tag is open!
				{
					std::cout << "error parsing the source code (wrong syntax at closing tag for PDF-extration)" << std::endl;
					std::cout << "i = " << i << std::endl;
					exit(1);
				}
			}

			// increase i until '>' is reached (which marks the end of the URL and beginning of the description)
			while (result[i] != '>')
				i++;

			// get the description of the URL (as viewed by the browser)
			bool close_descr = false;
			i += 1;						// increment variable i by 9 to set the position at the end of the opening bracket
			int start_extract_pos_desc = i;	// cache the start position

			std::string extract_desc;	// description of the file

			while (close_descr == false)
			{
				i++;

				if (result[i] == '<')	// end of desc reached
				{
					close_descr = true;
					extract_desc.append(result.begin()+start_extract_pos_desc, result.begin()+i);
					fetched_URLs_descriptions.push_back(extract_desc);
				}
				else if (i == result.length())	// error .. end of string while tag is open!
				{
					std::cout << "error parsing the source code (wrong syntax at closing tag for description-extration)" << std::endl;
					std::cout << "i = " << i << std::endl;
					exit(1);
				}
			}
		}
	}
}


int main()
{



std::string result = "<p>UE 066 443 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2020/Masterstudium_Architektur_2020.pdf\" target=\"_blank\">Architektur<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a></p><p>UE 066 444 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/Alte_Studienplaene/Masterstudium_Building_Science_and_Technology.pdf\" target=\"_blank\">Building Science &amp; Technologie<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a> englischsprachig</p><h3>Bauingenieurwesen</h3><p>UE 066 505 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2020/Masterstudium_Bauingenieurwissenschaften_2020.pdf\" target=\"_blank\">Bauingenieurwissenschaften <span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a></p><h3>Biomedical Engineering (interfakultär)</h3><p>UE 066 453 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2020/Masterstudium_Biomedical_Engineering_2020.pdf\" target=\"_blank\">Biomedical Engineering<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a> (englischsprachig)</p><h3>Computational Science and Engineering (interfakultär)</h3><p>UE 066 646 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2020/Masterstudium_Computational_Science_and_Engineering_2020.pdf\" target=\"_blank\">Computational Science and Engineering<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a></p><h3>Elektrotechnik</h3><p>UE 066 504 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2020/Masterstudium_Embedded_Systems_2020.pdf\" target=\"_blank\">Embedded Systems<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a></p><p>UE 066 506 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2018/MasterEnergieundAutomatisierungstechnik.pdf\" target=\"_blank\">Energie- und Automatiesierungstechnik<span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a></p><p>UE 066 507 <a href=\"/fileadmin/Assets/dienstleister/studienabteilung/MSc_Studienplaene_2019/Masterstudium_Telecommunications.pdf\" target=\"_blank\">Telecommunications <span class=\"sr-only\">, öffnet eine Datei in einem neuen Fenster</span></a>englischsprachig</p><p>UE 066 508 ";











	// 1. fetch the source code of the page

	// the site one wants to crawl
//	std::string page_to_crawl = "https://www.tuwien.at/tu-wien/organisation/zentrale-bereiche/studienabteilung/studienplaene";

	// fetch the source code of the given page
//	std::string result = fetch_source_single_page(page_to_crawl);

	// 2. parse the obtained source code (extract PDF links, descriptions, etc.)

	// define the structure in which the found data will be stored (two vectors of type std::String)
	std::vector<std::string> extract_PDF_urls;	// URLs pointing to the PDFs to be downloaded
	std::vector<std::string> extract_url_descr;	// description of the PDF according to the '<a href =' - element
	std::vector<int> extract_url_info;	// information about this curricula (from the two above given data entries)

	// extract the data
	extract_PDF_URL_and_descriptions(result, extract_PDF_urls, extract_url_descr);

	// check whether both std::vectors have the same size
	if (extract_PDF_urls.size() != extract_url_descr.size())
	{
		std::cout << "error: amount of obtained data does not match (" << extract_PDF_urls.size() << " | " << extract_url_descr.size() << ")" << std::endl;
		exit(1);
	}

	// extract additional information (BSc-, MSc-, expiring curriculum) -> iterate through each (found) element
	for (int i = 0; i < extract_PDF_urls.size(); i++)
	{
		// the elements by which the curricula are sorted by
		std::vector<std::string> search_str {"BSc", "MSc", "Doktor", "Erweiterungsstudium", "Gemeinsame_Studienprogramme", "Alte_Studienplaene"};

		// determine which curricula is which (of type)
		for (int j = 0; j < search_str.size(); j++)
		{
			if (extract_PDF_urls[i].find(search_str[j]) != std::string::npos)
			{
				extract_url_info.push_back(j);
				break;
			}
		}
	}

	// 3. download (all) the PDFs temporarily


	// 4. compare PDFs (new one?, new version?, changed version? -> hash the file -> save file if it is a new one)


	// 5. create a log of the crawl


	// download a single PDF given by an URL
//	fetch_PDF_from_URL("http://phys.ik.cx/physics/aufgabensammlung523.pdf");

	return 0;
}
