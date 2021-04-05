# TU-Wien-curricula-archive
A program to extract and archive university curricula of the TU Wien

## Description
Starting from a webpage this program extracts the PDF links to the curricula, downloads all found files and archives them. Determination whether the downloaded file is new or not is done via a CRC-32 checksum.

## Folder Structure
This project has the following folder structure
```
.
├── build
├── curricula
├── logs
├── obj
├── src
│   └── crawl.cpp
└── temp_downloads
```
Compilation steps are stored in *\build* and *\obj*. The program responsible for data extraction and processing is called *crawl.cpp*. The downloaded and sorted curricula are stored in *\curricula*. The folder *\temp_downloads* is used as an intermediate storage folder for the downloads and the folder *\logs* contains the runtime and processing logs which contain information about the program execution.


## Workflow of the Program *crawl.cpp*
1. Fetch the source code of a webpage using `fetch_source_single_page(...)`
2. Parse this data and extract the desired information using `extract_PDF_URL_and_descriptions(...)`

    The desired information are links to curricula with one exemplarily given by:

    `<a href="/fileadmin/Assets/dienstleister/studienabteilung/BSc_Studienplaene_2020/Bachelorstudium_Wirtschaftsinformatik_2020.pdf" target="_blank">Wirtschaftsinformatik<span class="sr-only">, öffnet eine Datei in einem neuen Fenster</span></a>`
    
    From each anchor tag the filepath (i) to the PDF as well as some additional information (ii) is extracted:
    1. `/fileadmin/Assets/dienstleister/studienabteilung/BSc_Studienplaene_2020/Bachelorstudium_Wirtschaftsinformatik_2020.pdf`
    2. `Wirtschaftsinformatik`

    The curricula type (BSc) in (ii) is extracted using `std::vector<std::string> search_str{...}` and the field of study is given by the link text. The extracted information is stored in `std::vectors extract_PDF_urls` (i) and `std::vectors extract_url_descr` (ii). Then, from the information stored in `extract_PDF_urls`, the curricula type (BSc, MSc, etc.) is determined which is stored in `std::vector<int> extract_url_info`.

3. Download all retrieved (valid) files into the folder *temp_downloads*.
4. Process the temporarily downloaded files

    1. Cleanup the file paths (e.g. remove *amp;*)
    2. Check if the folder where the file will be copied already exists
    
        1. If not, just create the folder and move the file into the newly created folder
        2. If the folder exists:
        
            1. Create the CRC-32 checksum of the file which should be sorted in (the target file)
            2. Loop through all files in the target path, calculating the CRC-32 checksum of every file and comparing this checksum against the checksum of the target file. If the file is already in the folder (determined by the checksum), just delete it from the temp folder. In case the file is not yet archived, move it into the folder (and add a time stamp to the filename).

    4. Closing check whether the temp folder is empty.

## Running the Crawler
Use the makefile to build and run the crawler.

