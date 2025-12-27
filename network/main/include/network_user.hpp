#ifndef NETWORK_USER_HPP_
#define NETWORK_USER_HPP_

#include "network_common.hpp"

namespace lynks {
    namespace network {
        class user {
            public:
                user() = default;

                /* 
                Parses a json string into an user-object. Assumed that this is used to handle client requests,
                therefore not handling the key for field `id` and just setting it to `0`.

                This will hash the `password` in the process.

                @param json_string containing `username` and `password`
                */
                user(std::string json_string);

                /* 
                Takes in a username and unhashed password. The password will be hashed in the process.
                */
                user(std::string _username, std::string plain_password);

                /* 
                Takes in all parameters for the user field. This user is constructed from the DB and therefore
                should the password be hashed at receival.
                */
                user(int64_t id, std::string _username, std::string _password);

                std::optional<std::string> to_json();
                std::string to_string() const;

                int64_t get_id() const;
                const std::string& get_username() const;
                const std::string& get_password() const;

            protected:
                /* 
                Hashes the password using OpenSSL SHA256 method. Will hash the field <password> of the object.
                */
                volatile void hash_password();

                /* 
                Validates the objects fields. Throws an std::invalid_argument exception if the objects doesn't
                pass the assigned parameters.
                */
                volatile void validate_user();

            private:
                int64_t id;
                std::string username;
                std::string password;
        };
    }
}

#endif