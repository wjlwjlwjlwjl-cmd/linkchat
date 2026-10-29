#pragma once

#include "mysql_client.h"

class PersonDao {
public:
    PersonDao(const char* user,
              const char* passwd,
              const char* db_name,
              const char* host = "mysql-service",
              unsigned int port = 3306,
              int connection_pool_size = 5) {
        db_ = MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);
    }

    size_t Insert(Person& person) {
        try {
            auto transaction = MysqlClient::Transaction(db_);
            auto& db = transaction->database();
            db.persist(person);
            MysqlClient::Commit(transaction);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return person.Id();
    }

    void Erase(const std::string& name) {
        try {
            auto transaction = MysqlClient::Transaction(db_);
            auto& db = transaction->database();
            using Query = odb::query<Person>;
            db.erase_query<Person>(Query::name == name);
            MysqlClient::Commit(transaction);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void Update(Person& person) {
        try {
            auto transaction = MysqlClient::Transaction(db_);
            auto& db = transaction->database();
            db.update(person);
            MysqlClient::Commit(transaction);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    std::vector<Person> Select(int count) {
        std::vector<Person> persons;
        try {
            auto transaction = MysqlClient::Transaction(db_);
            auto& db = transaction->database();
            using Result = odb::result<Person>;
            Result result(db.query<Person>("order by age desc limit " + std::to_string(count)));
            for (auto person : result) {
                std::cout << person.Name() << " " << person.Age() << person.Birthday() << std::endl;
                persons.push_back(person);
            }
            MysqlClient::Commit(transaction);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return persons;
    }
private:
    std::shared_ptr<odb::database> db_;
};
