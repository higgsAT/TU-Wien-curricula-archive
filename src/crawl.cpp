#include <string>
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <vector>
#include <cstring>
#include <boost/filesystem.hpp>
#include <filesystem>
#include <boost/crc.hpp>
#include <istream>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <iomanip>

// function inserting the entries into the logfile
void insert_logfile(std::string path_to_logfile, std::string msg_log1, std::string msg_log2 = "")
{
	// open the (log) file to write into
	std::ofstream logfile_ofstream;
	logfile_ofstream.open(path_to_logfile+"_log"+".txt", std::ios_base::app);

	// get the time and date
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char temp_insert_date_logfile[100] = {0};
	std::strftime(temp_insert_date_logfile, sizeof(temp_insert_date_logfile), "%Y-%m-%d %X ", std::localtime(&now));

	logfile_ofstream << temp_insert_date_logfile;
	std::cout << temp_insert_date_logfile;

	unsigned int padLen = 40;	// whitespace padding of the string
	if (msg_log1.size() > padLen)	// prevent an error when the logmsg1 is longer than the set paddin length
	{
		padLen = msg_log1.size();
	}

	msg_log1.append(padLen - msg_log1.size(), ' ');

	logfile_ofstream << msg_log1;
	std::cout << msg_log1;

	if (msg_log2 != "")
	{
		logfile_ofstream << msg_log2 << std::endl;
		std::cout << msg_log2 << std::endl;
	}
	else
	{
		logfile_ofstream << std::endl;
		std::cout << std::endl;
	}
}

// datastream (fetched source files) used by fetch_source_single_page
size_t write_fetched_data(void* ptr, size_t size, size_t nmemb, void* data)
{
	std::string* result = static_cast<std::string*>(data);
	*result += std::string((char*)ptr, size* nmemb);
	return size* nmemb;
}

// fetch the source code of a single page
std::string fetch_source_single_page(std::string url_full)
{
	std::string useragent = "https://github.com/clauskovacs/TU-Wien-curricula-archive";		// user agent string

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
void fetch_PDF_from_URL(std::string fetch_pdf_url, std::string filename, std::string temp_files_path)
{
	CURL* curl;
	FILE* fp;
	std::string useragent = "https://github.com/clauskovacs/TU-Wien-curricula-archive";		// user agent string

	// define locations where to save the file temporarily
	std::string outfilename = temp_files_path+filename;
	const char *cfilename = outfilename.c_str();

	curl = curl_easy_init();

	if (curl)
	{
		fp = fopen(cfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, fetch_pdf_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());		// set user agent string
		curl_easy_perform(curl);

		// cleanup
		curl_easy_cleanup(curl);
		fclose(fp);
	}
}


// cut the std::string / extract the filename which is the last part of the std::string
std::string filename_extraction(std::string process_string, std::string delimiter)
{
	size_t pos = 0;
	std::string token;

	while ((pos = process_string.find(delimiter)) != std::string::npos)
	{
		token = process_string.substr(0, pos);
		process_string.erase(0, pos + delimiter.length());	// remove the parts of the string left of the delimiter
	}

	return process_string;	// return the last part of the std::string (the filename)
}


// starting from the obtained page source code, this function extracts the links to the PDFs as well as the information about the PDFs. The obtained
// information is stored in the two std::vectors extracted_data1 and extracted_data2
void extract_PDF_URL_and_descriptions(std::string result, std::vector<std::string>& fetched_URLs, std::vector<std::string>& fetched_URLs_descriptions, std::string logpath)
{
	// iterate the std::string and find the desired elements
	for (unsigned int i = 0; i < result.length()-8; i++)
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
			std::string extract_URL;		// stores the link to the PDF

			bool already_in_list = false;	// prevent the same PDF being pushed multiple times into the std::vec (which would cause problems with further file processing)

			while (close_URL == false)
			{
				i++;

				if (result[i] == '\"')	// end of URL reached
				{
					close_URL = true;
					extract_URL.append(result.begin()+start_extract_pos_URL, result.begin()+i);

					// check whether this entry is already in the std::vector (multiple links leading to the same PDF)
					for (unsigned int j = 0; j < fetched_URLs.size(); j++)
					{

						if (filename_extraction(fetched_URLs[j], "/") == filename_extraction(extract_URL, "/"))
						{
							already_in_list = true;
							break;
						}
					}

					// only add the information _if_ this element is not already in the list (else the PDF would be added multiple times
					if (already_in_list == false)
					{
						fetched_URLs.push_back(extract_URL);
					}
				}
				else if (i == result.length())	// error .. end of string while tag is open!
				{
					// TODO: just skip this element
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
			i += 1;						// increment variable i by one to set the start position correctly
			int start_extract_pos_desc = i;	// cache the start position

			std::string extract_desc;	// description of the file

			while (close_descr == false)
			{
				i++;

				if (result[i] == '<')	// end of desc reached
				{
					close_descr = true;
					extract_desc.append(result.begin()+start_extract_pos_desc, result.begin()+i);

					// only add the information _if_ this element is not already in the list (else the PDF would be added multiple times
					if (already_in_list == false)
					{
						// clean-up the url (remove undesired strings)
						std::string delete_string_search = "&nbsp;";	// search and remove this string from the folder name
						std::string::size_type del_pos_search = extract_desc.find(delete_string_search);	// search the position of this string

						// string to delete has been found -> remove this part from the url (which is used as a folder name later on)
						if (del_pos_search != std::string::npos)	// if a position has been found it will be removed
						{
							insert_logfile(logpath, "cleanup extract_desc("+delete_string_search+"):", extract_desc);
							extract_desc.erase(del_pos_search, delete_string_search.length());
						}

						// push the description to the std::vector
						fetched_URLs_descriptions.push_back(extract_desc);
					}
				}
				else if (i == result.length())	// error .. end of string while tag is open!
				{
					// TODO: just skip this element
					std::cout << "error parsing the source code (wrong syntax of a closing tag for description-extration)" << std::endl;
					std::cout << "i = " << i << std::endl;
					exit(1);
				}
			}
		}
	}
}


// courtesy: Toby Speight (https://codereview.stackexchange.com/questions/133483/calculate-the-crc32-of-the-contents-of-a-file-using-boost)
// calculates the crc32 checksum for a given file
uint32_t crc32(std::string file_read_open)
{
	char buf[4096];
	boost::crc_32_type result;

	std::filebuf fb;

	if (fb.open (file_read_open, std::ios::in))
	{
		std::istream is(&fb);

		// calculate the crc32 checksum
		do
		{
			is.read(buf, sizeof buf);
			result.process_bytes(buf, is.gcount());
		}
		while (is);

		if (is.eof())
		{
			return result.checksum();
		}
		else
		{
			throw std::runtime_error("File read failed");
			return 0;
		}

		fb.close();
	}
	else
	{
		std::cout << "File read failed" << std::endl;
		return 0;
	}

	return 0;
}

int main()
{
	// file counters for the processing of the files (used in the logfiles)
	int total_amt_files_processed	= 0;	// total amount of files processed
	int total_amt_files_added		= 0;	// amount of new files added to the archive
	int total_amt_files_already_in	= 0;	// total number of files which are already in the archive

	// the site one wants to crawl
	std::string TLD						= "https://www.tuwien.at";													// top-level-domain
	std::string page_to_crawl			= "/tu-wien/organisation/zentrale-bereiche/studienabteilung/studienplaene";	// the page which will be crawled

	// file paths
	std::string temp_files_path			= "temp_downloads/";	// location where the temp downloaded files are stored
	std::string curricula_files_path	= "curricula/";			// location where the downloaded curricula are stored
	std::string log_files_path			= "logs/";				// location where the logs (info about the crawl) are stored

	// define the names of the folders where the curricula are stored to (corresponds to the numbering elements in the search_str std::vector)
	std::vector<std::string> folder_name_structure{"Bachelor/", "Master/", "Doktor/", "Erweiterungsstudium/", "Gemeinsame Studienprogramme/", "Alte Studienpläne/"};

	// the elements by which the curricula are sorted into (according to these searchstrings)
	std::vector<std::string> search_str{"BSc", "MSc", "Doktor", "Erweiterungsstudium", "Gemeinsame_Studienprogramme", "Alte_Studienplaene"};

	// fetch the date of today to insert it into the filename (and the logfile name)
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char temp_insert_date[100]			= {0};
	char temp_insert_date_logfile[100]	= {0};
	std::strftime(temp_insert_date, sizeof(temp_insert_date), " (%Y-%m-%d %X)", std::localtime(&now));
	std::strftime(temp_insert_date_logfile, sizeof(temp_insert_date_logfile), "%Y-%m-%d %X", std::localtime(&now));

 	insert_logfile(log_files_path+temp_insert_date_logfile, "start");
 	insert_logfile(log_files_path+temp_insert_date_logfile, "TLD: ", TLD);
 	insert_logfile(log_files_path+temp_insert_date_logfile, "crawled site: ", page_to_crawl);
 	insert_logfile(log_files_path+temp_insert_date_logfile, "temp download folder: ", temp_files_path);
 	insert_logfile(log_files_path+temp_insert_date_logfile, "curricula folder: ", curricula_files_path);

	for (unsigned int i = 0; i < search_str.size(); i++)
	{
		insert_logfile(log_files_path+temp_insert_date_logfile, "searchstr: "+std::to_string(i), search_str[i]);
	}

	//////////////////////////////////////////
	// 1. fetch the source code of the page //
	//////////////////////////////////////////

	// fetch the source code of the given page
 	insert_logfile(log_files_path+temp_insert_date_logfile, "fetching page source");
	std::string result = fetch_source_single_page(TLD+page_to_crawl);
 	insert_logfile(log_files_path+temp_insert_date_logfile, "page crawling donw");

	// write the crawled source file to the disk (in the log folder)
	std::ofstream source_of_page_ofstream;
	source_of_page_ofstream.open(log_files_path+temp_insert_date_logfile+"_source"+".txt", std::ios_base::app);
	source_of_page_ofstream << result;

	///////////////////////////////////////////////////////////////////////////////
	// 2. parse the obtained source code (extract PDF links, descriptions, etc.) //
	///////////////////////////////////////////////////////////////////////////////
	insert_logfile(log_files_path+temp_insert_date_logfile, "parsing page source");

	// define the structure in which the found data will be stored (two vectors of type std::String)
	std::vector<std::string> extract_PDF_urls;	// URLs pointing to the PDFs to be downloaded
	std::vector<std::string> extract_url_descr;	// description of the PDF according to the '<a href =' - element
	std::vector<int> extract_url_info;			// information about this curricula (from the two above given data entries)

	// extract the data
	extract_PDF_URL_and_descriptions(result, extract_PDF_urls, extract_url_descr, log_files_path+temp_insert_date_logfile);

	// check whether both std::vectors have the same size
	if (extract_PDF_urls.size() != extract_url_descr.size())
	{
		std::cout << "error: amount of obtained data does not match (" << extract_PDF_urls.size() << " | " << extract_url_descr.size() << ")" << std::endl;
		exit(1);
	}

	// extract additional information (BSc-, MSc-, expiring curriculum) -> iterate through each (found) element
	for (unsigned int i = 0; i < extract_PDF_urls.size(); i++)
	{
		bool found_element = false;

		// determine which curricula is which (of type)
		for (unsigned int j = 0; j < search_str.size(); j++)
		{
			if (extract_PDF_urls[i].find(search_str[j]) != std::string::npos)
			{
				extract_url_info.push_back(j);
				found_element = true;
			 	insert_logfile(log_files_path+temp_insert_date_logfile, "extracted file["+std::to_string(j)+"]:", extract_PDF_urls[i]+"|"+extract_url_descr[i]);

				// increment the number of total (extracted) files for the logfile
				total_amt_files_processed ++;

				break;
			}
		}

		// no element of interest
		if (found_element == false)
		{
			extract_url_info.push_back(-1);
		}
	}

	insert_logfile(log_files_path+temp_insert_date_logfile, "parsing page done");

	////////////////////////////////////////////
	// 3. download (all) the PDFs temporarily //
	////////////////////////////////////////////

	insert_logfile(log_files_path+temp_insert_date_logfile, "downloading PDFs");

	for (unsigned int i = 0; i < extract_PDF_urls.size(); i++)
	{
		// only parse desired links (ones of type PDF which were included in the search_str vector)
		if (extract_url_info[i] > -1)
		{
			// cout the PDF URLs
		 	insert_logfile(log_files_path+temp_insert_date_logfile, "downloading file["+std::to_string(extract_url_info[i])+"]:", extract_PDF_urls[i]);

			// TODO: check whether the file already exists in this (temp) folder!

			std::string extracted_file_name = filename_extraction(extract_PDF_urls[i], "/");

			// download a single PDF given by an URL
			fetch_PDF_from_URL(TLD+extract_PDF_urls[i], extracted_file_name, temp_files_path);

			// TODO: check whether the download was successful, i.e., check if the downloaded file exists
		}
	}

	insert_logfile(log_files_path+temp_insert_date_logfile, "downloading PDFs done");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. compare PDFs (new one?, new version?, changed version? -> hash the file -> save file if it is a new one) //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// go through each (temporarily) downloaded file: see if the file is already present in the downloaded folder or not

	// loop through all found files
	for (unsigned int i = 0; i < extract_PDF_urls.size(); i++)
	{
		if (extract_url_info[i] > -1)
		{
			// URL description clean-up ('amp;')
			std::string erase_search = "amp;";
			std::string::size_type search_pos = extract_url_descr[i].find(erase_search);	// search the position of this string

			if (search_pos != std::string::npos)	// if a position has been found it will be removed
			{
				insert_logfile(log_files_path+temp_insert_date_logfile, "cleanup extract_url_descr("+erase_search+"):", extract_url_descr[i]);
				extract_url_descr[i].erase(search_pos, erase_search.length());
			}

			// check if the folder already exists
			std::string check_folder_exist = curricula_files_path+folder_name_structure[extract_url_info[i]]+extract_url_descr[i];

			// old/new file paths
			std::string old_file_path = temp_files_path+filename_extraction(extract_PDF_urls[i], "/");
			std::string new_file_path = check_folder_exist+"/"+filename_extraction(extract_PDF_urls[i], "/");

			if (!boost::filesystem::exists(check_folder_exist))	// folder does not exist (create folder, move the file)
			{
				// create the folder and just copy the file into the folder (since it is the first file in this folder)
				std::filesystem::create_directories(check_folder_exist);
				insert_logfile(log_files_path+temp_insert_date_logfile, "create dir:", check_folder_exist);

				// add the crawled date to the file name
				new_file_path.insert(new_file_path.size()-4, temp_insert_date);

				// move the file to its final destination
				boost::filesystem::rename(old_file_path, new_file_path);
				insert_logfile(log_files_path+temp_insert_date_logfile, "move file(new):", old_file_path+"|"+new_file_path);

				// increase the amount of files added to the counter (logfile)
				total_amt_files_added ++;
			}
			else	// folder already exists -> check via crc32 checksum whether the file is new or not
			{
				// determine the crc32 checksum of the file which is being sorted into the folder
				uint32_t crc32_checksum_temp_file = crc32(old_file_path);
				insert_logfile(log_files_path+temp_insert_date_logfile, "crc32 of file:", old_file_path+"|"+std::to_string(crc32_checksum_temp_file));

				// fetch the (names of the) files in the folder
				std::string path = check_folder_exist;
				bool file_already_in_folder = false;

				// loop through all files found at this path
				for (const auto & entry : std::filesystem::directory_iterator(path))
				{
					// fetch the name of the files in this folder and build the file path
					std::string check_file_path = check_folder_exist+"/"+entry.path().filename().string();

					if (!std::filesystem::is_directory(check_file_path))	// don't process subdirectories
					{
						std::cout << check_file_path << "is a file" << std::endl;

						// create the crc32 checksum for this file
						uint32_t crc32_checksum_file_path = crc32(check_file_path);

						if (crc32_checksum_file_path == crc32_checksum_temp_file)	// file already in the folder -> delete file in temp folder
						{
							file_already_in_folder = true;

							// remove this file in the temp folder
							int remove_flag = std::remove(old_file_path.c_str());
							total_amt_files_already_in ++;

							// check for success of deletion
							if (remove_flag != 0)
							{
								std::cout << "deletion of file: " << old_file_path << " failed" << std::endl;
							}
							else	// deletion was successful -> create a log entry
							{
								insert_logfile(log_files_path+temp_insert_date_logfile, "delete file:", old_file_path.c_str());
							}

							break;
						}
					}
					else	// TODO: parse the subdirectories (add the same routine "for (const auto & entry : std::filesystem::directory_iterator(path))" to subdirectories. Which means check check the files in the subfolders too.
					{
						std::cout << check_file_path << "is a (sub)directory" << std::endl;
					}
				}

				// file not found in the folder (by comparing the crc32 checksum) -> move the file
				if (file_already_in_folder == false)
				{
					// add the crawled date to the file name
					new_file_path.insert(new_file_path.size()-4, temp_insert_date);

					// move the file to its final destination
					boost::filesystem::rename(old_file_path, new_file_path);
					insert_logfile(log_files_path+temp_insert_date_logfile, "move file(exist):", old_file_path+"|"+new_file_path);

					// increase the amount of files added to the counter (logfile)
					total_amt_files_added ++;
				}
			}
		}
	}

	///////////////////////
	// 5. closing checks //
	///////////////////////

	// check whether the temp download folder is empty

	int count_unsorted_files = 0;	// amount of files remaining in the temp download folder after running the program

	// loop through all files found in the temp download folder
	for (const auto & entry : std::filesystem::directory_iterator(temp_files_path))
	{
		std::string check_file_path_a = entry.path().filename().string();

		count_unsorted_files++;
	}

	// more than one file remained unsorted in the temp download folder
	if (count_unsorted_files > 0)
	{
		std::cout << "a total of " << count_unsorted_files << " files were not sorted and remain in the directory " << temp_files_path << std::endl;
	}

	insert_logfile(log_files_path+temp_insert_date_logfile, "amount of files in temp folder:", std::to_string(count_unsorted_files));

	// write info about the file processing into the logfile (amount of files processed, added, removed since they are already in the archive)
	insert_logfile(log_files_path+temp_insert_date_logfile, "amount of files processed:", std::to_string(total_amt_files_processed));
	insert_logfile(log_files_path+temp_insert_date_logfile, "amount of new files added:", std::to_string(total_amt_files_added));
	insert_logfile(log_files_path+temp_insert_date_logfile, "amount of files already in the archive (CRC32 duplicates):", std::to_string(total_amt_files_already_in));

 	insert_logfile(log_files_path+temp_insert_date_logfile, "end");

	return 0;
}
