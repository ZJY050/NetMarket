#include "user.h"
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <experimental/filesystem>

void injected_memory_leak_1();
void injected_memory_leak_2();
void injected_double_free_1();
void injected_double_free_2();
void injected_null_deref_1();
void injected_null_deref_2();

namespace fs = std::experimental::filesystem;

namespace {

    // Use DATA_DIR macro (set by CMake) as prefix for data files.
    // DATA_DIR ends with a slash according to CMake definition.
    const std::string kDataDir = DATA_DIR;

    void EnsureFileExists(const std::string& path) {
        fs::path p(path);
        if (!p.has_parent_path()) return;
        fs::path parent = p.parent_path();
        if (!parent.empty() && !fs::exists(parent)) {
            fs::create_directories(parent);
        }
        if (!fs::exists(p)) {
            std::ofstream ofs(p.string());
        }
    }

    // Read all non-empty lines from file.
    std::vector<std::string> ReadLines(const std::string& relpath) {
        std::vector<std::string> out;
        std::string path = kDataDir + relpath;
        EnsureFileExists(path);
        std::ifstream fin(path);
        if (!fin) return out;
        std::string line;
        while (std::getline(fin, line)) {
            if (!line.empty()) out.push_back(line);
        }
        return out;
    }

    // Write lines. If append true, append; otherwise overwrite.
    void WriteLines(const std::string& relpath,
        const std::vector<std::string>& lines,
        bool append = false) {
        std::string path = kDataDir + relpath;
        EnsureFileExists(path);
        std::ios::openmode mode = append ? std::ios::app : std::ios::out;
        std::ofstream fout(path, mode);
        for (const auto& l : lines) fout << l << '\n';
    }

    std::vector<std::string> SplitCSV(const std::string& line) {
        std::vector<std::string> out;
        std::istringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            out.push_back(token);
        }
        return out;
    }

    int NextMsgId() {
        const auto lines = ReadLines("messages.txt");
        int mx = 0;
        for (const auto& l : lines) {
            auto parts = SplitCSV(l);
            if (!parts.empty()) {
                try {
                    int id = std::stoi(parts[0]);
                    mx = std::max(mx, id);
                }
                catch (...) {}
            }
        }
        return mx + 1;
    }

}  // namespace

bool User::Register() const {
    if (!IsValidPhone(phone_)) {
        std::cout << "Phone must be 11 digits.\n";
        return false;
    }
    const auto users = ReadLines("users.txt");
    for (const auto& l : users) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 1 && parts[0] == phone_) {
            std::cout << "User already exists: " << phone_ << '\n';
            return false;
        }
    }
    WriteLines("users.txt", { phone_ + ',' + pwd_ }, true);
    return true;
}

bool User::Login() const {
    const auto users = ReadLines("users.txt");
    for (const auto& l : users) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 2 && parts[0] == phone_ && parts[1] == pwd_) {
            return true;
        }
    }
    return false;
}

void User::UpdatePassword(const std::string& new_pwd) {
    // Update in-memory value (this object) and rewrite users file.
    // Note: this function is non-const on purpose.
    // But here we have access to pwd_ member, so just assign.
    // Update file:
    auto users = ReadLines("users.txt");
    for (auto& l : users) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 1 && parts[0] == phone_) {
            l = phone_ + ',' + new_pwd;
            // continue (in case multiple lines, we update first match)
            break;
        }
    }
    WriteLines("users.txt", users, false);
    // update this object's pwd_
    const_cast<std::string&>(pwd_) = new_pwd;
}

int User::NextProductId() {
    const auto lines = ReadLines("products.txt");
    int max_id = 0;
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        if (!parts.empty()) {
            try {
                int id = std::stoi(parts[0]);
                max_id = std::max(max_id, id);
            }
            catch (...) {}
        }
    }
    return max_id + 1;
}

void User::Publish() const {
    std::string name;
    double price{};
    std::cout << "Product name (<=10 chars): ";
    std::cin >> name;
    std::cout << "Price: ";
    std::cin >> price;

    if (!IsValidName(name) || !IsValidPrice(price)) {
        std::cout << "Invalid input!\n";
        return;
    }
    int id = NextProductId();
    WriteLines("products.txt",
        { std::to_string(id) + ',' + name + ',' +
         std::to_string(price) + ',' + phone_ + ",ONSALE" },
        true);
    std::cout << "Publish success, ID=" << id << '\n';
}

void User::ListOnsale() const {
    const auto lines = ReadLines("products.txt");
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        // expected: id,name,price,seller,status
        if (parts.size() >= 5) {
            const std::string& idstr = parts[0];
            const std::string& name = parts[1];
            const std::string& price = parts[2];
            const std::string& seller = parts[3];
            const std::string& status = parts[4];
            if (status == "ONSALE") {
                std::cout << idstr << ". " << name << " CNY" << price
                    << " seller:" << seller << '\n';
            }
        }
    }
}

void User::ListMyProducts() const {
    const auto lines = ReadLines("products.txt");
    std::cout << "----My Products----\n";
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 5) {
            const std::string& idstr = parts[0];
            const std::string& name = parts[1];
            const std::string& price = parts[2];
            const std::string& seller = parts[3];
            const std::string& status = parts[4];
            if (seller == phone_) {
                std::cout << idstr << ". " << name << " CNY" << price
                    << ' ' << status << '\n';
            }
        }
    }
}

int User::NextOrderId() {
    const auto lines = ReadLines("orders.txt");
    int max_id = 0;
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        if (!parts.empty()) {
            try {
                int id = std::stoi(parts[0]);
                max_id = std::max(max_id, id);
            }
            catch (...) {}
        }
    }
    return max_id + 1;
}

void User::Buy(int product_id) const {
    auto lines = ReadLines("products.txt");
    bool found = false;
    std::string name, seller;
    double price{};
    for (auto& l : lines) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 5) {
            int id = 0;
            try { id = std::stoi(parts[0]); }
            catch (...) { continue; }
            std::string pname = parts[1];
            double pprice = 0;
            try { pprice = std::stod(parts[2]); }
            catch (...) {}
            std::string pseller = parts[3];
            std::string status = parts[4];
            if (id == product_id && status == "ONSALE") {
                found = true;
                name = pname;
                seller = pseller;
                price = pprice;
                // mark as SOLD
                l = std::to_string(id) + ',' + pname + ',' +
                    std::to_string(price) + ',' + pseller + ",SOLD";
                break;
            }
        }
    }
    if (!found) {
        std::cout << "Product not found or already sold.\n";
        return;
    }
    WriteLines("products.txt", lines, false);
    int oid = NextOrderId();
    WriteLines("orders.txt",
        { std::to_string(oid) + ',' + phone_ + ',' + seller + ',' +
         std::to_string(product_id) + ",UNCONFIRMED" },
        true);
    Notify(seller, "Your product \"" + name + "\" was ordered by " + phone_);
    std::cout << "Order created, ID=" << oid << '\n';
}

void User::Confirm(int order_id) const {
    auto lines = ReadLines("orders.txt");
    bool ok = false;
    std::string seller;
    for (auto& l : lines) {
        auto parts = SplitCSV(l);
        // expected: id,buyer,seller,pid,status
        if (parts.size() >= 5) {
            int id = 0;
            try { id = std::stoi(parts[0]); }
            catch (...) { continue; }
            std::string buyer = parts[1];
            std::string s = parts[2];
            std::string pid = parts[3];
            std::string st = parts[4];
            if (id == order_id && st == "UNCONFIRMED" && buyer == phone_) {
                l = std::to_string(id) + ',' + buyer + ',' + s + ',' + pid +
                    ",DONE";
                seller = s;
                ok = true;
                break;
            }
        }
    }
    if (!ok) {
        std::cout << "Order not found or already confirmed.\n";
        return;
    }
    WriteLines("orders.txt", lines, false);
    Notify(seller, "Buyer " + phone_ + " confirmed receipt of order " +
        std::to_string(order_id));
    std::cout << "Confirmation done.\n";
}

void User::ListMyOrders() const {
    const auto lines = ReadLines("orders.txt");
    std::cout << "----Bought----\n";
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 5) {
            const std::string& id = parts[0];
            const std::string& buyer = parts[1];
            const std::string& seller = parts[2];
            const std::string& pid = parts[3];
            const std::string& st = parts[4];
            if (buyer == phone_) {
                std::cout << id << ". product" << pid << " seller:" << seller
                    << ' ' << st << '\n';
            }
        }
    }
    std::cout << "----Sold----\n";
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        if (parts.size() >= 5) {
            const std::string& id = parts[0];
            const std::string& buyer = parts[1];
            const std::string& seller = parts[2];
            const std::string& pid = parts[3];
            const std::string& st = parts[4];
            if (seller == phone_) {
                std::cout << id << ". product" << pid << " buyer:" << buyer
                    << ' ' << st << '\n';
            }
        }
    }
}

bool IsValidPhone(const std::string& phone) {
    if (phone.size() != 11) return false;
    return std::all_of(phone.begin(), phone.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

bool IsValidPrice(double price) { return price > 0; }

bool IsValidName(const std::string& name) {
    return !name.empty() && name.size() <= 10;
}

void Notify(const std::string& to, const std::string& content) {
    int id = NextMsgId();
    WriteLines("messages.txt",
        { std::to_string(id) + ',' + to + ',' + content + ",0" }, true);
}

void ListUnreadMessages(const std::string& phone) {
    const auto lines = ReadLines("messages.txt");
    bool empty = true;
    for (const auto& l : lines) {
        auto parts = SplitCSV(l);
        // expected: id,to,content,read
        if (parts.size() >= 4) {
            const std::string& to = parts[1];
            const std::string& cont = parts[2];
            const std::string& read = parts[3];
            if (to == phone && read == "0") {
                std::cout << "[UNREAD] " << cont << '\n';
                empty = false;
            }
        }
    }
    if (empty) std::cout << "No unread messages.\n";
}

#include <cstdlib>
#include <cstring>

void injected_flaws_demo()
{
    injected_memory_leak_1();
    injected_memory_leak_2();
    injected_double_free_1();
    injected_double_free_2();
    injected_null_deref_1();
    injected_null_deref_2();
}

void injected_memory_leak_1() {
    int* p = new int[100];
}

void injected_memory_leak_2() {
    char* buf = (char*)malloc(256);
}

void injected_double_free_1() {
    int* q = (int*)malloc(sizeof(int));
    free(q);
    free(q);
}

void injected_double_free_2() {
    double* d = new double;
    delete d;
    delete d;
}

void injected_null_deref_1() {
    std::string* ps = nullptr;
    ps->clear();
}

void injected_null_deref_2() {
    int* pi = 0;
    *pi = 42;
}
