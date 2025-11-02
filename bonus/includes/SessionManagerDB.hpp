#pragma once

#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <ctime>


class SessionManagerDB {
    public:
        SessionManagerDB(const char *filename);

        void Create_Table_Users();
        void Create_Session_Table();

        bool Insert_User(const char* username, const char* password);
        bool Validate_User(const char* username, const char* password);

        bool Insert_Session(const char* username, const char* session_token);
        std::string Generate_token();
        bool Session_Exists(const char* session_token);
        bool Delete_Session(const char* session_token);
        bool check_expired_sessions(const char* session_token);

        static void getInstance();
        static void closeInstance();

    private:
        static sqlite3* db;
        static const char *db_name;
};

