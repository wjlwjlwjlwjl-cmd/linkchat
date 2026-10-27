#pragma once

#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#pragma db object table("tb_person")
class Person{
public:
    Person() = default;
    Person(const std::string& name): _name(name){}

    size_t id() {return _id;}
    void set_id(int id){_id = id;}

    const std::string& name()const { return _name;}
    void set_name(const std::string& name){ _name = name;}

    odb::nullable<int> age() const {return _age;}
    void set_age(odb::nullable<int> age){_age = age;}

    odb::nullable<boost::posix_time::ptime> birthday() const {return _birthday;}
    void set_birthday(odb::nullable<boost::posix_time::ptime> birthday){ _birthday = birthday;}
private:
    friend class odb::access; //允许odb访问私有成员
    #pragma db id auto
    size_t _id;

    std::string _name;
    odb::nullable<int> _age;
    odb::nullable<boost::posix_time::ptime> _birthday;
};