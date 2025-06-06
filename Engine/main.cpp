/*
    This is a small log:
        -neglected the Wishlist function because it will literally be useless here.
        -added a few more structs to programs.
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
#include <vector>
#include <iomanip>
#include <sys/stat.h>
#include <map>
#include <regex>

bool generatedStore = false;

class Rel_Date {
public:
    int day = 1, month = 1, year = 1;
};

class Product {
public:
    int id = 0;
    std::string gameName = "";
    Rel_Date date;
    std::string developer = "", publisher = "";
    double price = 0.0;
    double discount = 0.0;
    bool dlc = false;
};
//Folders
LPCSTR binFolder = "bin";
LPCSTR libraryFolder = "library";

//Files
LPCSTR storeFile = "store.txt";
LPCSTR cartFile = "cart.txt";
LPCSTR libraryFile = "library.txt";
LPCSTR orderHistoryFile = "orderHistory.txt";

//FilePaths
LPCSTR storeFilePath = "bin\\store.txt";

enum class CommandType {
    GENERATE,
    STORE,
    ADD,
    DEL,
    EDIT,
    HELP,
    UNKNOWN
};

CommandType parseCommand(const std::string& cmd) {
    if (cmd == "generate") return CommandType::GENERATE;
    if (cmd == "store") return CommandType::STORE;
    if (cmd == "add") return CommandType::ADD;
    if (cmd == "delete") return CommandType::DEL;
    if (cmd == "edit") return CommandType::EDIT;
    if (cmd == "help") return CommandType::HELP;
    return CommandType::UNKNOWN;
}

#pragma region Help
void help() {
    UINT oldCPOut = GetConsoleOutputCP();

    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\x1B[93mEngine \x1B[33mv1.0\n\n\x1B[37m";
    std::cout << "This app is the core of the \x1B[93m\"Mist\"\x1B[37m app.\nWith it you can generate, view and manage the products of the store.\n\n";
    
    std::cout << "Usage:\n";
    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m help\n";
    std::cout << u8"     ↪ Shows this.\n\n\n";

    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m generate\n";
    std::cout << u8"     ↪ Generates the perequisite files for the application.\n\n\n";

    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m store <dev>\n";
    std::cout << u8"     ↪ View the items available in the Store.\n";
    std::cout << u8"     ↪ <dev> argument is optional and shows more details\n\n\n";

    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m add <ID> <gameName> <Day> <Month> <Year> <Developer> <Publisher> <Price> <Discount> <DLC>\n";
    std::cout << u8"     ↪ Adds an instance of a game to the bin\\store.txt file.\n";
    std::cout << u8"     ↪ All of the fields are required.\n";
    std::cout << u8"→ <ID> - int, it must be 9 numbers long.\n";
    std::cout << u8"→ <gameName> - string\n";
    std::cout << u8"→ <Day> - int from 1 to 31\n";
    std::cout << u8"→ <Month> - int from 1 to 12\n";
    std::cout << u8"→ <Year> - int\n";
    std::cout << u8"→ <Developer> - string\n";
    std::cout << u8"→ <Publisher> - string\n";
    std::cout << u8"→ <Price> - double with 2 digits max.\n";
    std::cout << u8"→ <Discount> - double with 2 digits max. Cannot exceed 100.\n";
    std::cout << u8"→ <DLC> - bool true or false\n\n\n";

    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m delete <ID>\n";
    std::cout << u8"     ↪ Deletes the game with the specific ID if it exists.\n\n\n";

    std::cout << "\x1B[93m   .\\Engine.exe\x1B[37m edit <ID> <Field> <Value>\n";
    std::cout << u8"     ↪ Edits the field of a product with a new value.\n";
    std::cout << u8"→ <ID> - int\n";
    std::cout << u8"→ <Field> - will be one of these: ID | name | day | month | year | developer | publisher | price | discount | dlc |\n";
    std::cout << u8"→ <Value> - will inhibit the same value types and caps as \x1B[93m.\\Engine\x1B[37m add\n\n\n";

    SetConsoleOutputCP(oldCPOut);
}
#pragma endregion

#pragma region Generation
void generateFolder(LPCSTR folder) {
    DWORD fileAttr = GetFileAttributesA(folder);
    if (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY))
        std::clog << "\x1B[37m[\x1B[91mWarn\x1B[37m]\x1B[37m Folder \"\x1B[31m" << folder << "\x1B[37m\" already exists.\n\n";

    else {
        if (CreateDirectoryA(folder, NULL)) 
            std::clog << "\x1B[37m[\x1B[93mInfo\x1B[37m]: \x1B[92mSuccessfully \x1B[37mcreated the folder: \"\x1B[32m" << folder << "\x1B[37m\"\n\n";
        else 
            std::cerr << "\x1B[91mFailed\x1B[37m to create folder. \x1B[31mError code\x1B[37m: \x1B[91m" << GetLastError() << "\x1B[37m\n\n";
    }
}

void generateFileInFolder(const std::string& folder, const std::string& filename) {
    std::string fullPath = folder + "\\" + filename;

    DWORD fileAttr = GetFileAttributesA(fullPath.c_str());
    if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        std::clog << "\x1B[37m[\x1B[91mWarn\x1B[37m] File \"\x1B[31m" << fullPath << "\x1B[37m\" already exists.\n\n";
        return;
    }

    HANDLE hFile = CreateFileA(
        fullPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE) {
        std::clog << "\x1B[37m[\x1B[93mInfo\x1B[37m]: \x1B[92mCreated file:\x1B[37m " << fullPath << "\n\n";
        if (filename == "store.txt") generatedStore = true;
        CloseHandle(hFile);
    }
    else std::cerr << "\x1B[91mFailed to create file:\x1B[37m " << fullPath << ". \x1B[31mError code\x1B[37m: \x1B[91m" << GetLastError() << "\x1B[37m\n\n";
}

void generateStoreGames(std::ofstream& storeFile);

void handleStoreGeneration() {
    if (generatedStore) {
        std::ofstream storeFile(storeFilePath, std::ios::trunc);
        if (!storeFile.is_open()) {
            std::cerr << "\x1B[31mError\x1B[37m: Couldn't open the store file for writing.\n";
            return;
        }
        generateStoreGames(storeFile);
        storeFile.close();

        std::clog << "\x1B[37m[\x1B[93mInfo\x1B[37m]: \x1B[92mUpdated file:\x1B[37m " << storeFilePath << "\n\n";
    }
    else std::cerr << "\x1B[37m[\x1B[91mWarn\x1B[37m] File contents from \"\x1B[93mbin\\store\x1B[37m\" already generated. Will not Update.\n\n";
}

void generateStoreGames(std::ofstream& storeFile) {
    storeFile << "214145678 Counter_Strike_2 27 9 2023 Valve Valve 13.29 0.00 false\n";
    storeFile << "512712512 Portal_2 18 4 2011 Valve Valve 9.75 0.00 false\n";
    storeFile << "124611287 Terraria 16 5 2011 Re-Logic Re-Logic 9.75 0.00 false\n";
    storeFile << "123241598 Minecraft 17 5 2009 Mojang Mojang 26.95 0.00 false\n";
    storeFile << "674958984 Risk_Of_Rain_2 11 8 2020 Hopoo_Games Gearbox_Publishing 24.99 0.00 false\n";
    storeFile << "932745982 Risk_Of_Rain_2:_Survivors_Of_The_Void 1 3 2022 Hopoo_Games Gearbox_Publishing 14.99 0.00 true\n";
}

void handleGenerate() {
    generateFolder(binFolder);
    generateFolder(libraryFolder);

    generateFileInFolder(binFolder, storeFile);
    generateFileInFolder(binFolder, cartFile);
    generateFileInFolder(binFolder, orderHistoryFile);
    generateFileInFolder(libraryFolder, libraryFile);

    handleStoreGeneration();
}
#pragma endregion

#pragma region Store
void showStore(bool dev) {
    std::ifstream file("bin\\store.txt");
    if (!file.is_open()) {
        std::cerr << "\x1B[91mError\x1B[37m: Could not open the \x1B[91mstore\x1B[37m file.\nIf you didn't \x1B[93mgenerate\x1B[37m the perequisite files, please use \x1B[93m.\\Engine generate\x1B[37m\n\n";
        return;
    }

    std::vector<Product> products;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Product product;

        if (!(iss >> product.id >> product.gameName >> product.date.day >> product.date.month >> product.date.year
            >> product.developer >> product.publisher >> product.price >> product.discount
            >> std::boolalpha >> product.dlc)) {
            std::cerr << "\x1B[91mError\x1B[37m: parsing line: \x1B[91m" << line << "\x1B[37m\n\n";
            continue;
        }

        products.push_back(product);
    }

    file.close();

    std::sort(products.begin(), products.end(), [](const Product& a, const Product& b) {
        return a.gameName < b.gameName;
        });

    std::cout << std::fixed << std::setprecision(2);

    for (const auto& product : products) {
        double finalPrice = product.price - (product.price * product.discount / 100.0);
        std::string dlcStatus = product.dlc ? "yes" : "no";

        std::string displayName = product.gameName;
        std::replace(displayName.begin(), displayName.end(), '_', ' ');

        if (dev) {
            std::cout << "\x1B[93mID: \x1B[37m" << product.id
                << ", \x1B[93mGame: \x1B[37m" << product.gameName
                << ", \x1B[93mDate: \x1B[37m" << product.date.day << "/" << product.date.month << "/" << product.date.year
                << ", \x1B[93mDeveloper: \x1B[37m" << product.developer
                << ", \x1B[93mPublisher: \x1B[37m" << product.publisher
                << ", \x1B[93mOriginal Price: \x1B[37m" << product.price
                << ", \x1B[93mDiscount: \x1B[37m" << product.discount << "%"
                << ", \x1B[93mPrice: \x1B[37m" << finalPrice
                << ", \x1B[93mDLC: \x1B[37m" << dlcStatus << "\n\n\n\n";
        }
        else {
            std::cout << "\x1B[95mID: \x1B[37m" << product.id
                << ", \x1B[36mDiscount: \x1B[37m" << product.discount << "%"
                << ", \x1B[33mPrice: \x1B[37m" << finalPrice
                << ", \x1B[91mDLC: \x1B[37m" << dlcStatus
                << ", \x1B[32mGame: \x1B[37m" << displayName << "\n";
        }
    }
    std::cout << "\n";
}

void handleStore(int argc, char* argv[]) {
    bool showDevDetails = false;

    if (argc >= 3 && std::string(argv[2]) == "dev") showDevDetails = true;

    if (showDevDetails) showStore(true);
    if (!showDevDetails) showStore(false);
}
#pragma endregion

#pragma region Delete
void deleteProduct(const std::string& filename, int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "\x1B[31mError\x1B[37m: Usage is \x1B[93m.\\Engine delete <ID>\x1B[37m\n\n";
        return;
    }

    std::string idToDelete = argv[2];
    std::vector<std::string> lines;
    bool found = false;

    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "\x1B[31mError\x1B[37m: Could \x1B[91mnot\x1B[37m open file \x1B[91m" << filename << "\x1B[37m\nIf you didn't \x1B[93mgenerate\x1B[37m the perequisite files, please use \x1B[93m.\\Engine generate\x1B[37m\n\n";
        return;
    }

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        if (line.compare(0, idToDelete.length(), idToDelete) == 0
            && (line.length() == idToDelete.length() || line[idToDelete.length()] == ' ')) {
            found = true;
            continue;
        }
        lines.push_back(line);
    }
    fin.close();

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: Product with \x1B[93mID \x1B[33m" << idToDelete << "\x1B[37m not \x1B[91mfound.\x1B[37m\n\n";
        return;
    }

    std::ofstream fout(filename);
    if (!fout) {
        std::cerr << "\x1B[31mError\x1B[37m: Could \x1B[91mnot\x1B[37m open file \x1B[91m" << filename << "\x1B[37m for writing.\n\n";
        return;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        fout << lines[i];
        if (i != lines.size() - 1) fout << '\n';
    }
    fout.close();

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Product with \x1B[93mID \x1B[33m" << idToDelete << "\x1B[37m deleted \x1B[92msuccessfully\x1B[37m.\n\n";
}
#pragma endregion

#pragma region Add
bool isAllDigits(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

bool fileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void handleAdd(int argc, char* argv[]) {
    if (argc <= 2) {
        std::cerr << "\x1B[31mError\x1B[37m: Missing arguments for \x1B[93madd\x1B[37m. \n\x1B[93mUsage\x1B[37m: .\\Engine add <\x1B[96mid\x1B[37m> <\x1B[96mgameName\x1B[37m> <\x1B[96mday\x1B[37m> <\x1B[96mmonth\x1B[37m> <\x1B[96myear\x1B[37m> <\x1B[96mdeveloper\x1B[37m> <\x1B[96mpublisher\x1B[37m> <\x1B[96mprice\x1B[37m> <\x1B[96mdiscount\x1B[37m> <\x1B[96mdlc\x1B[37m>\n\n";
        return;
    }

    Product p;
    std::string idStr = argv[2];
    if (!isAllDigits(idStr) || idStr.size() != 9) {
        std::cerr << "\x1B[31mError\x1B[37m: \x1B[93mID\x1B[37m must be exactly \x1B[94m9\x1B[37m digits.\n\n";
        return;
    }
    try {
        p.id = std::stoi(idStr);
        p.gameName = argv[3];
        p.date.day = std::stoi(argv[4]);
        p.date.month = std::stoi(argv[5]);
        p.date.year = std::stoi(argv[6]);
        p.developer = argv[7];
        p.publisher = argv[8];
        p.price = std::stod(argv[9]);
        p.discount = std::stod(argv[10]);

        if (p.date.day < 1 || p.date.day > 31) {
            std::cerr << "\x1B[31mError\x1B[37m: Day must be \x1B[96m1\x1B[37m to \x1B[96m31\x1B[37m.\n\n";
            return;
        }
        if (p.date.month < 1 || p.date.month > 12) {
            std::cerr << "\x1B[31mError\x1B[37m: Month must be \x1B[96m1\x1B[37m to \x1B[96m12\x1B[37m.\n\n";
            return;
        }
        if (p.discount < 0.0 || p.discount > 100.0) {
            std::cerr << "\x1B[31mError\x1B[37m: Discount must be \x1B[96m0\x1B[37m to \x1B[96m100\x1B[37m.\n\n";
            return;
        }

        std::string dlcStr = argv[11];
        if (dlcStr == "true") {
            p.dlc = true;
        }
        else if (dlcStr == "false") {
            p.dlc = false;
        }
        else {
            std::cerr << "\x1B[31mError\x1B[37m: dlc must be '\x1B[96mtrue\x1B[37m' or '\x1B[96mfalse\x1B[37m'.\n\n";
            return;
        }
    }
    catch (...) {
        std::cerr << "\x1B[31mError\x1B[37m: Invalid argument types.\n\n";
        return;
    }

    const std::string filePath = "bin\\store.txt";

    if (!fileExists(filePath)) {
        std::cerr << "\x1B[31mError\x1B[37m: File '\x1B[91m" << filePath << "\x1B[37m' not found.\n\n";
        return;
    }

    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Failed to open '\x1B[91m" << filePath << "\x1B[37m' for reading.\n\n";
        return;
    }
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        int existingId;
        if (iss >> existingId && existingId == p.id) {
            std::cerr << "\x1B[31mError\x1B[37m: A product with the \x1B[93mID \x1B[33m" << p.id << "\x1B[37m already exists.\n\n";
            return;
        }
    }
    inFile.close();

    std::ofstream outFile(filePath, std::ios::app);
    if (!outFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Failed to open '\x1B[91m" << filePath << "\x1B[37m' for writing.\n\n";
        return;
    }

    outFile << p.id << ' '
        << p.gameName << ' '
        << p.date.day << ' ' << p.date.month << ' ' << p.date.year << ' '
        << p.developer << ' '
        << p.publisher << ' '
        << std::fixed << std::setprecision(2) << p.price << ' '
        << p.discount << ' '
        << (p.dlc ? "true" : "false") << '\n';

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Product added \x1B[32msuccessfully\x1B[37m to \x1B[92m" << filePath << "\x1B[37m.\n\n";
}
#pragma endregion

#pragma region Edit
bool isInt(const std::string& s) {
    return std::regex_match(s, std::regex("^-?\\d+$"));
}

bool isDouble(const std::string& s) {
    return std::regex_match(s, std::regex("^-?\\d+(\\.\\d{1,2})?$"));
}

void invalidTypeError() {
    std::cerr << "\x1B[31mError\x1B[37m: Invalid argument types.\n\n";
}

void handleEdit(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "\x1B[31mError\x1B[37m: Missing arguments for \x1B[93medit\x1B[37m. \n\x1B[93mUsage\x1B[37m: \x1B[93m.\\Engine\x1B[37m edit <\x1B[96mid\x1B[37m> <\x1B[96mfield\x1B[37m> <\x1B[96mvalue\x1B[37m>\n\n";
        return;
    }

    std::string idToEdit = argv[2];
    std::string fieldToEdit = argv[3];
    std::string newValue = argv[4];

    std::map<std::string, int> fieldIndex = {
        {"id", 0}, {"name", 1}, {"day", 2}, {"month", 3},
        {"year", 4}, {"developer", 5}, {"publisher", 6},
        {"price", 7}, {"discount", 8}, {"dlc", 9}
    };

    if (fieldIndex.find(fieldToEdit) == fieldIndex.end()) {
        std::cerr << "\x1B[31mError\x1B[37m: Invalid field name '\x1B[96m" << fieldToEdit << "\x1B[37m'.\n\n";
        return;
    }

    if (fieldToEdit == "id") {
        if (!isInt(newValue) || newValue.length() != 9) {
            std::cerr << "\x1B[31mError\x1B[37m: ID must be a \x1B[969-digit\x1B[37 number.\n\n";
            return;
        }
    }
    else if (fieldToEdit == "day") {
        if (!isInt(newValue)) {
            invalidTypeError();
            return;
        }
        int day = std::stoi(newValue);
        if (day < 1 || day > 31) {
            std::cerr << "\x1B[31mError\x1B[37m: Day must be between \x1B[961\x1B[37 and \x1B[9631\x1B[37.\n\n";
            return;
        }
    }
    else if (fieldToEdit == "month") {
        if (!isInt(newValue)) {
            invalidTypeError();
            return;
        }
        int month = std::stoi(newValue);
        if (month < 1 || month > 12) {
            std::cerr << "\x1B[31mError\x1B[37m: Month must be between \x1B[961\x1B[37 and \x1B[9612\x1B[37.\n\n";
            return;
        }
    }
    else if (fieldToEdit == "year") {
        if (!isInt(newValue)) {
            invalidTypeError();
            return;
        }
    }
    else if (fieldToEdit == "price") {
        if (!isDouble(newValue)) {
            invalidTypeError();
            return;
        }
    }
    else if (fieldToEdit == "discount") {
        if (!isDouble(newValue)) {
            invalidTypeError();
            return;
        }
        double discount = std::stod(newValue);
        if (discount < 0 || discount > 100) {
            std::cerr << "\x1B[31mError\x1B[37m: Discount must be between \x1B[960\x1B[37 and \x1B[96100\x1B[37.\n\n";
            return;
        }
    }
    else if (fieldToEdit == "dlc") {
        if (newValue != "true" && newValue != "false") {
            std::cerr << "\x1B[31mError\x1B[37m: DLC field must be either '\x1B[96true\x1B[37' or '\x1B[96false'\x1B[37.\n\n";
            return;
        }
    }

    std::ifstream inFile("bin/store.txt");
    if (!inFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open '\x1B[96mbin/store.txt'\x1B[37m\n\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool found = false;

    while (std::getline(inFile, line)) {
        std::istringstream ss(line);
        std::vector<std::string> fields;
        std::string word;

        while (ss >> word) fields.push_back(word);

        if (fields.size() < 10) {
            lines.push_back(line);
            continue;
        }

        if (fields[0] == idToEdit) {
            fields[fieldIndex[fieldToEdit]] = newValue;

            std::ostringstream updatedLine;
            for (int i = 0; i < fields.size(); ++i) {
                if (i > 0) updatedLine << " ";
                updatedLine << fields[i];
            }

            lines.push_back(updatedLine.str());
            found = true;
        }
        else {
            lines.push_back(line);
        }
    }
    inFile.close();

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: No entry with \x1B[93mID \x1B[33m" << idToEdit << "\x1B[37m found.\n";
        return;
    }

    std::ofstream outFile("bin/store.txt");
    for (const std::string& l : lines) outFile << l << "\n";
    outFile.close();

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Field '\x1B[96m" << fieldToEdit << "\x1B[37m' updated for \x1B[93mID \x1B[33m" << idToEdit << "\x1B[37m.\n\n";
}
#pragma endregion

int main(int argc, char* argv[]) {
    if (argc < 2) {
        help();
        return 1;
    }

    std::string cmdStr = argv[1];
    CommandType cmd = parseCommand(cmdStr);

    switch (cmd) {
    case CommandType::GENERATE:
        handleGenerate();
        break;

    case CommandType::STORE:
        handleStore(argc, argv);
        break;

    case CommandType::ADD:
        handleAdd(argc, argv);
        break;
       
    case CommandType::DEL:
        deleteProduct("bin\\store.txt", argc, argv);
        break;

    case CommandType::EDIT:
        handleEdit(argc, argv);
        break;

    case CommandType::HELP:
        help();
        break;

    default:
        std::cerr << "\x1B[31mError\x1B[37m: Unknown command: \x1B[93m" << cmdStr << "\x1B[37m\n";
        break;
    }
    return 0;
}