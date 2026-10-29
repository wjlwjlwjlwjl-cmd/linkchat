//odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time
// -d mysql，指明数据库类型为 mysql
// --std c++11，生成代码的语言标准
// --generate-query 生成查询sql的代码
// --generate-schema 生成建表 sql
// --profile boost/date-time 让数据库的 DATE、DATETIME 能够正常被映射

#include "person.h"
#include "person-odb.hxx"

#include "person_dao.h"

void Insert(PersonDao& person_dao) {
    for (int i = 0; i < 10; i++) {
        std::string name = "person" + std::to_string(i);
        int age = i + 20;
        Person person(name);
        person.SetAge(age);
        person_dao.Insert(person);
    }
}

void Erase(PersonDao& person_dao) {
    person_dao.Erase("person1");
}

void Update(PersonDao& person_dao) {
    Person person;
    person.SetId(1);
    person.SetName("name_modified");
    person_dao.Update(person);
}

void Select(PersonDao& person_dao) {
    person_dao.Select(10);
}

int main() {
    PersonDao person_dao("root", "123456", "test_db");

    //Insert(person_dao);
    //Erase(person_dao);
    //Update(person_dao);
    Select(person_dao);

    return 0;
}
