#ifndef _HOME_Z____CODE_INCLUDE_USER_H_
#define _HOME_Z____CODE_INCLUDE_USER_H_

#include <string>
#include <utility>  // for std::move
#include <vector>

// Forward declaration to reduce include dependencies.
struct Product;
struct Order;

// Represents a user in the network market system.
class User {
 public:
    User() = default;
    User(std::string phone, std::string pwd)
        : phone_(std::move(phone)), pwd_(std::move(pwd)) {}

    // Account API.
    bool Register() const;
    bool Login() const;
    void UpdatePassword(const std::string& new_pwd);

    // Business API.
    void Publish() const;
    void ListOnsale() const;
    void ListMyProducts() const;
    void Buy(int product_id) const;
    void Confirm(int order_id) const;
    void ListMyOrders() const;

    // Static helpers.
    static int NextProductId();
    static int NextOrderId();

    // Getter.
    const std::string& GetPhone() const { return phone_; }

 private:
    std::string phone_;
    std::string pwd_;
};

// Utility functions for validation.
bool IsValidPhone(const std::string& phone);
bool IsValidPrice(double price);
bool IsValidName(const std::string& name);

// Message utilities.
void Notify(const std::string& to, const std::string& content);
void ListUnreadMessages(const std::string& phone);

#endif  // _HOME_Z____CODE_INCLUDE_USER_H_

