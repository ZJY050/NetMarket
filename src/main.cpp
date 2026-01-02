#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include "user.h"

namespace {

    void ClearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    void PrintMainMenu() {
        std::cout << "\n1.Register 2.Login 3.Exit\n";
    }

    void PrintUserMenu() {
        std::cout << "\n1.Market 2.Publish 3.MyProducts 4.Buy 5.Confirm 6."
                     "MyOrders 7.UnreadMsg 8.ChangePwd 9.Logout\n";
    }

}  // namespace

int main() {
    std::unique_ptr<User> curr;
    while (true) {
        PrintMainMenu();
        int cmd{};
        if (!(std::cin >> cmd)) {
            ClearInput();
            continue;
        }
        if (cmd == 3) break;
        if (cmd == 1) {
            std::string phone, pwd;
            std::cout << "Phone: ";
            std::cin >> phone;
            std::cout << "Password: ";
            std::cin >> pwd;
            ClearInput();
            User u(phone, pwd);
            if (u.Register()) {
                std::cout << "Register OK\n";
            } else {
                std::cout << "Register failed\n";
            }
            continue;
        }
        if (cmd == 2) {
            std::string phone, pwd;
            std::cout << "Phone: ";
            std::cin >> phone;
            std::cout << "Password: ";
            std::cin >> pwd;
            ClearInput();
            User u(phone, pwd);
            if (!u.Login()) {
                std::cout << "Login failed\n";
                continue;
            }
            curr = std::make_unique<User>(phone, pwd);
            while (true) {
                PrintUserMenu();
                int c{};
                if (!(std::cin >> c)) {
                    ClearInput();
                    continue;
                }
                if (c == 9) break;
                switch (c) {
                case 1:
                    curr->ListOnsale();
                    break;
                case 2:
                    curr->Publish();
                    break;
                case 3:
                    curr->ListMyProducts();
                    break;
                case 4: {
                    int id{};
                    std::cout << "Product ID: ";
                    std::cin >> id;
                    curr->Buy(id);
                    break;
                }
                case 5: {
                    int id{};
                    std::cout << "Order ID: ";
                    std::cin >> id;
                    curr->Confirm(id);
                    break;
                }
                case 6:
                    curr->ListMyOrders();
                    break;
                case 7:
                    ListUnreadMessages(curr->GetPhone());
                    break;
                case 8: {
                    std::string newpwd;
                    std::cout << "New password: ";
                    std::cin >> newpwd;
                    // Update password correctly
                    curr->UpdatePassword(newpwd);
                    std::cout << "Password updated\n";
                    break;
                }
                default:
                    std::cout << "Invalid choice\n";
                    break;
                }
            }
            curr.reset();
        }
    }
    return 0;
}

