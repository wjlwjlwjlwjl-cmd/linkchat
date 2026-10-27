#pragma once

#include "mysql_client.hpp"

class PersonDao{
public:
    PersonDao(const char* user,
        const char* passwd,
        const char* db_name,
        const char* host = "mysql-service",
        unsigned int port = 3306, 
        int connection_pool_size = 5){
        _db = MysqlClient::Create(user, passwd, db_name, host, port, 5);
    }

    size_t insert(Person& person){
        try{
            auto ta = MysqlClient::Transaction(_db);            
            auto& db = ta->database();
            db.persist(person);
            MysqlClient::Commit(ta);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        return person.id();
    }

    void erase(const std::string& name){
        try{
            auto ta = MysqlClient::Transaction(_db);            
            auto& db = ta->database();
            using query = odb::query<Person>;
            db.erase_query<Person>(query::name == name);
            MysqlClient::Commit(ta);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }

    void update(Person& person){
        try{
            auto ta = MysqlClient::Transaction(_db);            
            auto& db = ta->database();
            db.update(person);
            MysqlClient::Commit(ta);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }

    std::vector<Person> select(int n){
        std::vector<Person> persons;
        try{
            auto ta = MysqlClient::Transaction(_db);            
            auto& db = ta->database();
            using result = odb::result<Person>;
            result r(db.query<Person>("order by age desc limit " + std::to_string(n)));
            for(auto e: r){
                std::cout << e.name() << " " << e.age() << e.birthday() << std::endl;
                persons.push_back(e);
            }
            MysqlClient::Commit(ta);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        return persons;
    }
private:
    std::shared_ptr<odb::database> _db;
};