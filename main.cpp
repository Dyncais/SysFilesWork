#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sys/statvfs.h>
#include <string>
#include <filesystem>
#include <cstring>
#include <zip.h>
#include <tinyxml2.h>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace tinyxml2;

void createFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        std::cout << "File created: " << filename << std::endl;
    } else {
        std::cerr << "Error creating file: " << filename << std::endl;
    }
}

void writeToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << content << std::endl;
        std::cout << "Written to file: " << content << std::endl;
    } else {
        std::cerr << "Error writing to file: " << filename << std::endl;
    }
}

void readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        std::cout << "Contents of " << filename << ":\n";
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
    } else {
        std::cerr << "Error reading from file: " << filename << std::endl;
    }
}

void deleteFile(const std::string& filename) {
    if (fs::remove(filename)) {
        std::cout << "File deleted: " << filename << std::endl;
    } else {
        std::cerr << "Error deleting file: " << filename << std::endl;
    }
}

void createJsonFile(const std::string& filename, const json& j) {
    std::ofstream o(filename);
    if (o.is_open()) {
        o << j.dump(4) << std::endl;
        std::cout << "JSON file created: " << filename << std::endl;
    } else {
        std::cerr << "Error creating file: " << filename << std::endl;
    }
}

json getUserInput() {
    json j;
    std::string name;
    int age;
    bool is_student;

    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    std::cout << "Enter your age: ";
    std::cin >> age;
    std::cout << "Are you a student (1 for yes, 0 for no): ";
    std::cin >> is_student;
    std::cin.ignore(); 

    j["name"] = name;
    j["age"] = age;
    j["is_student"] = is_student;

    return j;
}

void readJsonFile(const std::string& filename) {
    std::ifstream i(filename);
    if (i.is_open()) {
        json j_read;
        i >> j_read;
        std::cout << "Contents of " << filename << ":\n" << j_read.dump(4) << std::endl;
    } else {
        std::cerr << "Error reading file: " << filename << std::endl;
    }
}

void createXmlFile(const std::string& filename) {
    XMLDocument doc;
    XMLNode* root = doc.NewElement("Root");
    doc.InsertFirstChild(root);
    
    std::string elementText;
    std::cout << "Enter the text for the element: ";
    std::getline(std::cin, elementText);
    
    XMLElement* element = doc.NewElement("Example");
    element->SetText(elementText.c_str());
    root->InsertEndChild(element);
    
    XMLError eResult = doc.SaveFile(filename.c_str());
    if (eResult == XML_SUCCESS) {
        std::cout << "XML file created: " << filename << std::endl;
    } else {
        std::cerr << "Error creating XML file: " << filename << std::endl;
    }
}


void printXmlElement(tinyxml2::XMLNode* node, int depth) {
    if (node == nullptr) return;

    const char* name = node->Value();
    if (name) {
        std::cout << std::string(depth * 2, ' ') << "Element: " << name << std::endl;
    }

    if (node->ToElement()) {
        const char* text = node->ToElement()->GetText();
        if (text) {
            std::cout << std::string(depth * 2, ' ') << "Text: " << text << std::endl;
        }
    }

    for (tinyxml2::XMLNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling()) {
        printXmlElement(child, depth + 1);
    }
}

void readXmlFile(const std::string& filename) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.LoadFile(filename.c_str());
    if (eResult != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error reading XML file: " << filename << std::endl;
        return;
    }

    tinyxml2::XMLNode* root = doc.FirstChild();
    if (root == nullptr) {
        std::cerr << "Error parsing XML file: No root element." << std::endl;
        return;
    }

    std::cout << "Contents of " << filename << ":\n";
    
    printXmlElement(root, 0);
}

void createZipArchive(const std::string& archiveName, const std::string& fileToAdd) {
    int error = 0;
    zip_t* zip = zip_open(archiveName.c_str(), ZIP_TRUNCATE | ZIP_CREATE, &error);
    
    if (!zip) {
        std::cerr << "Error creating ZIP archive: " << archiveName << " (error code: " << error << ")" << std::endl;
        return;
    }
    
    std::cout << "ZIP archive created: " << archiveName << std::endl;

    zip_source_t* source = zip_source_file(zip, fileToAdd.c_str(), 0, 0);
    if (!source) {
        std::cerr << "Error adding file to ZIP: " << fileToAdd << std::endl;
        zip_close(zip);
        return;
    }
    
    if (zip_file_add(zip, fs::path(fileToAdd).filename().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
        std::cerr << "Error adding file to ZIP: " << fileToAdd << " (error: " << zip_strerror(zip) << ")" << std::endl;
        zip_source_free(source);
    } else {
        std::cout << "File added to ZIP: " << fileToAdd << std::endl;
    }

    zip_close(zip);
    std::cout << "ZIP archive closed: " << archiveName << std::endl;
}

void extractFileFromZip(const std::string& archiveName, const std::string& fileName) {
    int error = 0;
    zip_t* zip = zip_open(archiveName.c_str(), 0, &error);
    
    if (!zip) {
        std::cerr << "Error opening ZIP archive: " << archiveName << " (error code: " << error << ")" << std::endl;
        return;
    }

    zip_file_t* zf = zip_fopen(zip, fileName.c_str(), 0);
    if (!zf) {
        std::cerr << "Error opening file in ZIP: " << fileName << std::endl;
        zip_close(zip);
        return;
    }

    std::ofstream outFile(fileName, std::ios::binary);
    char buffer[8192];
    zip_uint64_t bytesRead;
    while ((bytesRead = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
        outFile.write(buffer, bytesRead);
    }
    outFile.close();
    zip_fclose(zf);
    zip_close(zip);
    
    std::cout << "Extracted file: " << fileName << std::endl;
}

void getFileInfo(const std::string& fileName) {
    fs::path p(fileName);
    if (fs::exists(p)) {
        std::cout << "File: " << p.filename() << std::endl;
        std::cout << "Size: " << fs::file_size(p) << " bytes" << std::endl;
    } else {
        std::cerr << "File does not exist: " << fileName << std::endl;
    }
}

void deleteZipArchive(const std::string& archiveName) {
    if (fs::remove(archiveName)) {
        std::cout << "ZIP archive deleted: " << archiveName << std::endl;
    } else {
        std::cerr << "Error deleting ZIP archive: " << archiveName << std::endl;
    }
}

void printDiskInfo() {
    std::string paths[] = {"/", "/boot", "/var/log", "/home"};

    for (const auto& path : paths) {
        std::filesystem::space_info si = std::filesystem::space(path);
        struct statvfs buf;

        if (statvfs(path.c_str(), &buf) == 0) {
            std::cout << "Информация о диске " << path << ":\n";
            std::cout << "Общий размер: " << si.capacity / (1024 * 1024 * 1024) << " ГБ" << std::endl;  
            std::cout << "Свободно: " << si.free / (1024 * 1024 * 1024) << " ГБ" << std::endl;          
            std::cout << "Доступно: " << si.available / (1024 * 1024 * 1024) << " ГБ" << std::endl;
            std::cout << "-----------------------------" << std::endl;
        }
    }
}

void fileCommandLoop(int mode) {
    std::string command;
    while (true) {
        std::cout << "Enter command (create, read, delete, exit): ";
        std::cin >> command;
        std::cin.ignore();

        if (command == "create") {
            std::string filename;
            std::cout << "Enter filename: ";
            std::cin >> filename;
            std::cin.ignore();

            if (mode == 2) { // Обычные
                std::string content;
                std::cout << "Enter content to write: ";
                std::getline(std::cin, content);
                createFile(filename);
                writeToFile(filename, content);
            } else if (mode == 3) { // JSON
                json j = getUserInput();
                createJsonFile(filename, j);
            } else if (mode == 4) { // XML
                createXmlFile(filename);
            } else if (mode == 5) { // ZIP
                std::string fileToAdd;
                std::cout << "Enter the filename to add to the ZIP archive: ";
                std::getline(std::cin, fileToAdd);
                createZipArchive(filename, fileToAdd);
            }

        } else if (command == "read") {
            std::string filename;
            std::cout << "Enter filename: ";
            std::cin >> filename;

            if (mode == 2) {
                readFromFile(filename);
            } else if (mode == 3) {
                readJsonFile(filename);
            } else if (mode == 4) {
                readXmlFile(filename);
            } else if (mode == 5) {
                getFileInfo(filename);
            }

        } else if (command == "delete") {
            std::string filename;
            std::cout << "Enter filename: ";
            std::cin >> filename;
            if (mode == 5) {
                deleteZipArchive(filename);
            } else {
                deleteFile(filename);
            }

        } else if (command == "exit") {
            std::cout << "Exiting..." << std::endl;
            break;

        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }
}

int main() {

    while(true)
    {
    int choice;
    std::cout << "Choose option (1 - Disk Info, 2 - File Operations, 3 - JSON Operations, 4 - XML Operations, 5 - ZIP Operations): ";
    std::cin >> choice;
    std::cin.ignore();

    switch (choice) {
        case 1:
            printDiskInfo(); 
            break;
        case 2:
            std::cout << "Entering file operations mode...\n";
            fileCommandLoop(2); 
            break;
        case 3:
            std::cout << "Entering JSON operations mode...\n";
            fileCommandLoop(3);
            break;
        case 4:
            std::cout << "Entering XML operations mode...\n";
            fileCommandLoop(4);
            break;
        case 5:
            std::cout << "Entering ZIP operations mode...\n";
            fileCommandLoop(5);
            break;
        default:
            std::cout << "Invalid choice." << std::endl;
            break;
    }
    }
    return 0;
}
