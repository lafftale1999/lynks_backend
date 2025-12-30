//
// network_user.cpp
//

#include "network_user.hpp"
#include "network_crypto.hpp"

#include "nlohmann/json.hpp"
#include <regex>

namespace lynks::network {

    static const std::regex username_validation(R"(^[a-zA-Z\d_]{3,20}$)");

    /* 
    --------------------------- CONSTRUCTORS --------------------------------------
    */

    user::user(std::string json_string) {
        
        try {
            auto json = nlohmann::json::parse(json_string);
            id = 0;
            username = json["username"].get<std::string>();
            password = json["password"].get<std::string>();

            hash_password();
            validate_user();
        } catch (const std::exception& e) {
            std::cerr << 
                "[SERVER] unable to parse incoming json to object:\n" <<
                json_string << std::endl <<
                "Error: " << e.what() << std::endl;
        }
    }

    user::user(std::string _username, std::string plain_password)
    : id(0), username(std::move(_username)), password(std::move(plain_password))
    {
        hash_password();
        validate_user();
    }

    user::user(int64_t id, std::string _username, std::string _password) 
    : id(id), username(std::move(_username)), password(std::move(_password))
    {
        validate_user();   
    }

    /* 
    --------------------------- PUBLIC MEMBER FUNCTIONS --------------------------------------
    */
    std::optional<std::string> user::to_json() {
        try {
            nlohmann::json json;
            json["users"]["id"] = id;
            json["users"]["username"] = username;
            json["users"]["password"] = password;
            return json.dump();
        } catch (const std::exception& e) {
            std::cerr << 
                "[SERVER] unable to parse object into json:\n" <<
                to_string();
        }
        
        return std::nullopt;
    }

    std::string user::to_string() const {
        std::stringstream ss;

        ss <<
            "id: " << id << "\n" <<
            "username: " << username << "\n" << 
            "password: " << password << "\n";

        return ss.str();
    }

    int64_t user::get_id() const {
        return id;
    }

    const std::string& user::get_username() const {
        return username;
    }

    const std::string& user::get_password() const {
        return password;
    }

    /* 
    --------------------------- PRIVATE MEMBER FUNCTIONS --------------------------------------
    */
    void user::validate_user() {
        if (id < 0) 
            throw std::invalid_argument("ID can't be less than 0");
        if (username.empty() || 
            !std::regex_match(username.begin(), username.end(), username_validation))
            throw std::invalid_argument("Username contains invalid symbols");
        if (password.empty() || password.length() != 64)
            throw std::invalid_argument("Password does not have correct format");
    }

    void user::hash_password() {
        password = crypto::hash256(password);
    }
}
