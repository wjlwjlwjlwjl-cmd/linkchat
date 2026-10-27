//odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time
// -d mysql，指明数据库类型为 mysql
// --std c++11，生成代码的语言标准
// --generate-query 生成查询sql的代码
// --generate-schema 生成建表 sql
// --profile boost/date-time 让数据库的 DATE、DATETIME 能够正常被映射

#include "person.hxx"
#include "person-odb.hxx"

#include "person_dao.hpp"

void insert(PersonDao& personDao){
    for(int i = 0; i < 10; i++){
        std::string name = "person" + std::to_string(i);
        int age = i + 20;
        Person p(name);
        p.set_age(age);
        personDao.insert(p);
    }
}

void erase(PersonDao& personDao){
    personDao.erase("person1");
}

void update(PersonDao& personDao){
    Person person;
    person.set_id(1);
    person.set_name("name_modified");
    personDao.update(person);
}

void select(PersonDao& personDao){
    personDao.select(10);
}

int main(){
    PersonDao personDao("root", "123456", "test_db");

    //insert(personDao);
    //erase(personDao);
    //update(personDao);
    select(personDao);
    
    return 0;
}