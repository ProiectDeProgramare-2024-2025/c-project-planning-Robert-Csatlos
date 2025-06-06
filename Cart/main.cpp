#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <chrono>
#include <unordered_set>

class CartItem {
public:
    CartItem() = default;

    CartItem(int id, const std::string& name, double price, double discount, bool dlc)
        : id(id), name(name), price(price), discount(discount), dlc(dlc) {}

    int id = 0;
    std::string name;
    double price = 0.00;
    double discount = 0.00;
    bool dlc = false;
};

class LibraryItem {
public:
    LibraryItem() = default;

    LibraryItem(int id, const std::string& name, bool dlc, bool installed = false)
        : id(id), name(name), dlc(dlc), installed(installed) {}

    int id;
    std::string name;
    bool dlc;
    bool installed = false;
};

bool isValidId(const std::string& id) {
    if (id.size() != 9) return false;
    for (char c : id) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool parseDlc(const std::string& dlcStr) {
    std::string lower;
    std::transform(dlcStr.begin(), dlcStr.end(), std::back_inserter(lower), ::tolower);
    return (lower == "true" || lower == "1");
}

std::string dlcToString(bool dlc) {
    return dlc ? "true" : "false";
}

enum class CommandType {
    HELP,
    ADD,
    VIEW,
    DEL,
    BUY,
    HISTORY,
    UNKNOWN
};

CommandType parseCommand(const std::string& cmd) {
    if (cmd == "help") return CommandType::HELP;
    if (cmd == "add")  return CommandType::ADD;
    if (cmd == "view") return CommandType::VIEW;
    if (cmd == "buy") return CommandType::BUY;
    if (cmd == "history") return CommandType::HISTORY;
    if (cmd == "delete") return CommandType::DEL;
    return CommandType::UNKNOWN;
}

#pragma region Help
void help() {
    UINT oldCPOut = GetConsoleOutputCP();

    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\x1B[93mCart \x1B[33mv1.0\n\n\x1B[37m";
    std::cout << "This app is handling the purchasing mechanic of the \x1B[93m\"Mist\"\x1B[37m app.\nWith it you can buy products from the store.\n\n";
    
    std::cout << "Usage:\n";
    std::cout << "\x1B[93m   .\\Cart.exe\x1B[37m help\n";
    std::cout << u8"     ↪ Shows this.\n\n\n";

    std::cout << "\x1B[93m   .\\Cart.exe\x1B[37m view\n";
    std::cout << u8"     ↪ Shows the contents of the cart, alongside the prices and discounts\n\n\n";

    std::cout << "\x1B[93m   .\\Cart.exe\x1B[37m add <ID>\n";
    std::cout << u8"     ↪ Adds a game to the cart. ID is an int of length 9.\n\n\n";

    std::cout << "\x1B[93m   .\\Cart.exe\x1B[37m delete <ID>\n";
    std::cout << u8"     ↪ Removes a game from the cart. ID is an int of length 9.\n\n\n";

    std::cout << "\x1B[93m   .\\Cart.exe\x1B[37m buy\n";
    std::cout << u8"     ↪ Buys the contents of the cart.\n     ↪ Use .\\Library view to view your library.\n\n\n";



    SetConsoleOutputCP(oldCPOut);
}
#pragma endregion

#pragma region Add
void add(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "\x1B[31mError\x1B[37m: Missing ID argument.\n\n";
        return;
    }
    if (argc > 3) {
        std::cerr << "\x1B[31mError\x1B[37m: Too many arguments.\n\n";
        return;
    }

    std::string idStr = argv[2];

    if (!isValidId(idStr)) {
        std::cerr << "\x1B[31mError\x1B[37m: Invalid ID format. Must be exactly 9 digits.\n\n";
        return;
    }

    int id = std::stoi(idStr);

    std::ifstream storeFile("bin/store.txt");
    if (!storeFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/store.txt.\n\n";
        return;
    }

    std::ifstream cartReadFile("bin/cart.txt");
    if (cartReadFile) {
        std::string cartLine;
        while (std::getline(cartReadFile, cartLine)) {
            std::istringstream iss(cartLine);
            int cartId;
            if (iss >> cartId && cartId == id) {
                std::cerr << "\x1B[31mError\x1B[37m: Item already in cart.\n\n";
                return;
            }
        }
    }

    std::ofstream cartFile("bin/cart.txt", std::ios::app);
    if (!cartFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/cart.txt for writing.\n\n";
        return;
    }

    std::string line;
    bool found = false;

    while (std::getline(storeFile, line)) {
        std::istringstream iss(line);
        int fileId;
        std::string name, developer, publisher, dlcStr;
        int day, month, year;
        double price, discount;

        if (!(iss >> fileId >> name >> day >> month >> year >> developer >> publisher >> price >> discount >> dlcStr)) {
            continue;
        }

        if (fileId == id) {
            found = true;
            bool dlc = parseDlc(dlcStr);
            cartFile << fileId << " " << name << " " << price << " " << discount << " " << dlcToString(dlc) << "\n";
            std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] Item added to cart \x1B[32msuccessfully\x1B[37m.\n\n";
            break;
        }
    }

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: ID not found in store.\n\n";
    }
}
#pragma endregion

#pragma region View
void view() {
    std::ifstream cartFile("bin/cart.txt");
    if (!cartFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/cart.txt.\nUse \x1B[93m.\\Engine.exe generate \x1B[37mto generate the prerequisite files. \n\n";
        return;
    }

    std::vector<CartItem> items;
    std::string line;

    double totalOriginal = 0.0;
    double totalDiscounted = 0.0;
    double discountSum = 0.0;
    int discountCount = 0;

    while (std::getline(cartFile, line)) {
        std::istringstream iss(line);
        CartItem item;
        std::string dlcStr;

        if (!(iss >> item.id >> item.name >> item.price >> item.discount >> dlcStr))
            continue;

        item.dlc = parseDlc(dlcStr);

        double d = item.discount;
        if (d > 1.0) d = d / 100.0;

        totalOriginal += item.price;
        totalDiscounted += item.price * (1.0 - d);
        discountSum += d;
        discountCount++;
        items.push_back(item);
    }

    std::sort(items.begin(), items.end(), [](const CartItem& a, const CartItem& b) {
        return a.name < b.name;
        });

    if (items.empty()) {
        std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] Your cart is empty.\n\n";
        return;
    }

    std::cout << "Your cart: \n";

    std::cout << std::left
        << "\x1B[35m" << std::setw(12) << "ID"       
        << "\x1B[34m" << std::setw(10) << "Discount" 
        << "\x1B[33m" << std::setw(10) << "Price"    
        << "\x1B[31m" << std::setw(8) << "Dlc"       
        << "\x1B[36m" << "Game" << "\x1B[37m\n";     

    for (const auto& item : items) {
        std::cout << std::left
            << "\x1B[95m" << std::setw(12) << item.id
            << "\x1B[94m" << std::setw(10) << item.discount
            << "\x1B[93m" << std::setw(10) << item.price
            << "\x1B[91m" << std::setw(8) << (item.dlc ? "true" : "false")
            << "\x1B[96m" << item.name
            << "\x1B[37m\n";
    }

    double avgDiscount = discountCount > 0 ? (discountSum / discountCount) * 100.0 : 0.0;

    std::cout << "\n\x1B[1m\x1B[32m"
        << "\x1B[92mProduct Prices\x1B[37m: $" << std::fixed << std::setprecision(2) << totalOriginal << "\n"
        << "\x1B[94mDiscount\x1B[37m: " << std::fixed << std::setprecision(2) << avgDiscount << "%\n"
        << "\x1B[32mTotal Price\x1B[37m: $" << std::fixed << std::setprecision(2) << totalDiscounted
        << "\x1B[37m\n\n";
}
#pragma endregion

#pragma region Delete
void deleteFromCart(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "\x1B[31mError\x1B[37m: Usage: .\\Cart.exe delete <9-digit id>\n\n";
        return;
    }

    std::string idStr = argv[2];
    if (!isValidId(idStr)) {
        std::cerr << "\x1B[31mError\x1B[37m: Invalid ID. Must be exactly 9 digits.\n\n";
        return;
    }

    int idToDelete = std::stoi(idStr);

    std::ifstream cartFile("bin/cart.txt");
    if (!cartFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/cart.txt for reading.\n\n";
        return;
    }

    std::vector<CartItem> items;
    std::string line;
    bool found = false;


    while (std::getline(cartFile, line)) {
        std::istringstream iss(line);
        CartItem item;
        std::string dlcStr;
        if (!(iss >> item.id >> item.name >> item.price >> item.discount >> dlcStr)) {
            continue;
        }
        item.dlc = parseDlc(dlcStr);

        if (item.id == idToDelete) {
            found = true;
            continue;
        }
        items.push_back(item);
    }
    cartFile.close();

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: ID " << idToDelete << " not found in cart.\n\n";
        return;
    }

    std::ofstream outFile("bin/cart.txt", std::ios::trunc);
    if (!outFile) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/cart.txt for writing.\n\n";
        return;
    }

    for (const auto& item : items) {
        outFile << item.id << ' ' << item.name << ' ' << item.price << ' ' << item.discount << ' ' << dlcToString(item.dlc) << '\n';
    }

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Item with the \x1B[93mID\x1B[37m:\x1B[33m" << idToDelete << "\x1B[37m deleted from cart.\n\n";
}
#pragma endregion

#pragma region Buy
void buyItems() {
    std::ifstream cartFile("bin/cart.txt");
    if (!cartFile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/cart.txt for reading.\n\n";
        return;
    }

    std::vector<CartItem> cartItems;
    std::string line;

    while (std::getline(cartFile, line)) {
        std::istringstream iss(line);
        CartItem item;
        std::string dlcStr;
        if (!(iss >> item.id >> item.name >> item.price >> item.discount >> dlcStr)) {
            continue;
        }
        item.dlc = (dlcStr == "true");
        cartItems.push_back(item);
    }

    cartFile.close();

    if (cartItems.empty()) {
        std::cerr << "\x1B[31mError\x1B[37m: Cart is empty.\n\n";
        return;
    }

    std::unordered_set<int> libraryIds;
    std::ifstream libIn("library/library.txt");
    std::ofstream libOut("library/library.txt", std::ios::app);

    if (libIn.is_open()) {
        std::string libLine;
        while (std::getline(libIn, libLine)) {
            std::istringstream iss(libLine);
            int id;
            if (iss >> id) {
                libraryIds.insert(id);
            }
        }
        libIn.close();
    }

    bool anyAdded = false;
    std::vector<CartItem> itemsToLog;

    for (const auto& item : cartItems) {
        if (libraryIds.count(item.id)) {
            std::cout << "\x1B[33mWarning\x1B[37m: Game \"" << item.name << "\" with ID " << item.id << " is already in your library.\n\n";
        }
        else {
            if (libOut.is_open()) {
                libOut << item.id << " " << item.name << " " << (item.dlc ? "true" : "false") << " false\n";
                anyAdded = true;
                itemsToLog.push_back(item);
            }
            else {
                std::cerr << "\x1B[31mError\x1B[37m: Could not open library/library.txt for writing.\n\n";
                return;
            }
        }
    }

    libOut.close();

    if (anyAdded) {
        std::ofstream historyOut("bin/orderHistory.txt", std::ios::app);
        if (!historyOut.is_open()) {
            std::cerr << "\x1B[31mError\x1B[37m: Could not open bin/orderHistory.txt for writing.\n\n";
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        localtime_s(&now_tm, &now_c);

        historyOut << "=====  Order from "
            << std::put_time(&now_tm, "%d//%m//%Y at %H//%M")
            << "  =====\n";

        for (const auto& item : itemsToLog) {
            historyOut << std::setfill('0') << std::setw(9) << item.id << " "
                << item.name << " "
                << std::fixed << std::setprecision(2) << item.price << " "
                << std::fixed << std::setprecision(2) << item.discount << " "
                << (item.dlc ? "true" : "false") << "\n";
        }

        historyOut << "_______________________________________________\n\n\n";
        historyOut.close();

        std::ofstream clearCart("bin/cart.txt", std::ios::trunc);
        if (!clearCart.is_open()) {
            std::cerr << "\x1B[31mError\x1B[37m: Could not clear bin/cart.txt.\n\n";
            return;
        }

        clearCart.close();
        std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] Items bought from cart.\n\n";
    }
}
#pragma endregion

#pragma region History
void viewHistory() {
    std::ifstream file("bin\\orderHistory.txt"); // Correct file path with escaped backslashes
    if (!file.is_open()) {
        std::cerr << "Error: Could not open order history file.\n";
        return;
    }

    std::string line;

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] Showing Order History...\n===== Order History =====\n\n";
    while (std::getline(file, line)) {
        std::cout << "" << line << "\n";
    }
    std::cout << "=========================\n\n";

    file.close();
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
    case CommandType::HELP:
        help();
        break;

    case CommandType::ADD:
        add(argc, argv);
        break;

    case CommandType::VIEW:
        view();
        break;

    case CommandType::DEL:
        deleteFromCart(argc, argv);
        break;

    case CommandType::BUY:
        buyItems();
        break;

    case CommandType::HISTORY:
        viewHistory();
        break;

    default:
        std::cerr << "\x1B[31mError\x1B[37m: Unknown command: \x1B[93m" << cmdStr << "\x1B[37m\n\n";
        break;
    }
    return 0;
}