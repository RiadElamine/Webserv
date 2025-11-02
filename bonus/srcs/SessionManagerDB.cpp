
#include "../includes/SessionManagerDB.hpp"

sqlite3 *SessionManagerDB::db = NULL;
const char *SessionManagerDB::db_name = NULL;

SessionManagerDB::SessionManagerDB(const char *filename)
{
    try
    {
        db_name = filename;
        getInstance();

        Create_Table_Users();
        Create_Session_Table();

        closeInstance();
    }
    catch (const std::exception &e)
    {
        closeInstance();
        throw e;
    }
}

void SessionManagerDB::Create_Table_Users()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS Users (username VARCHAR(15) PRIMARY KEY, password VARCHAR(15) NOT NULL);";
    char *err_msg = NULL;

    int exit = sqlite3_exec(db, sql, NULL, 0, &err_msg);
    if (exit != SQLITE_OK)
    {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create table for user");
    }
    std::cout << "Table of user is created successfully" << std::endl;
}

void SessionManagerDB::Create_Session_Table()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS Sessions (session_token VARCHAR(64) PRIMARY KEY,"
                      "username VARCHAR(15),"
                      "created_at TIMESTAMP DEFAULT (DATETIME('now','localtime')),"
                      "expires_at TIMESTAMP DEFAULT (DATETIME('now', '+1 minute' , 'localtime')),"
                      "FOREIGN KEY(username) REFERENCES Users(username) ON DELETE CASCADE);";
    char *err_msg = NULL;

    int exit = sqlite3_exec(db, sql, NULL, 0, &err_msg);
    if (exit != SQLITE_OK)
    {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create table for session");
    }
    std::cout << "Table of session is created successfully" << std::endl;
}

bool SessionManagerDB::Insert_User(const char *username, const char *password)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Users (username, password) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind username: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind password: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        if (rc == SQLITE_CONSTRAINT)
        {
            return (false);
        }
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (true);
}

bool SessionManagerDB::Insert_Session(const char *username, const char *session_token)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Sessions (session_token, username) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind session_token: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind username: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        if (rc == SQLITE_CONSTRAINT)
        {
            return (false);
        }
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (true);
}

bool SessionManagerDB::Validate_User(const char *username, const char *password)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM Users WHERE username = ? AND password = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind username: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind password: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    bool is_valid = false;
    if (rc == SQLITE_ROW)
        is_valid = true;

    sqlite3_finalize(stmt);
    return (is_valid);
}

bool SessionManagerDB::Session_Exists(const char *session_token)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM Sessions WHERE session_token = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind session_token: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    bool exists = false;
    if (rc == SQLITE_ROW)
        exists = true;

    sqlite3_finalize(stmt);
    return (exists);
}

bool SessionManagerDB::Delete_Session(const char *session_token)
{
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Sessions WHERE session_token = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind session_token: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    std::cout << "Session for this user deleted successfully." << std::endl;
    return true;
}

void SessionManagerDB::getInstance()
{
    int exit = sqlite3_open(db_name, &db);
    if (exit)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("Failed to open database");
    }
    std::cout << "Opened database successfully" << std::endl;
}

void SessionManagerDB::closeInstance()
{
    if (db)
    {
        sqlite3_close(db);
        db = NULL;
        std::cout << "Database connection closed." << std::endl;
    }
}

bool SessionManagerDB::check_expired_sessions(const char *session_token)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM Sessions WHERE session_token = ? AND expires_at <= DATETIME('now','localtime');";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to bind session_token: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(db));
    }

    bool is_expired = false;
    if (rc == SQLITE_ROW)
        is_expired = true;

    sqlite3_finalize(stmt);
    return (is_expired);
}

std::string SessionManagerDB::Generate_token()
{
    const char chars[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int num_chars = sizeof(chars) - 1;

    std::srand(std::time(0));

    std::string token;
    for (int i = 0; i < 50; ++i)
    {
        token += chars[std::rand() % num_chars];
    }

    std::stringstream ss;
    ss << std::hex << &num_chars;
    token += ss.str();

    return (token);
}
