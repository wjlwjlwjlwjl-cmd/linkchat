#pragma once

#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#pragma db object table("tb_person")
class Person {
public:
    Person() = default;
    Person(const std::string& name) : name_(name) {}

    size_t Id() { return id_; }
    void SetId(int id) { id_ = id; }

    const std::string& Name() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    odb::nullable<int> Age() const { return age_; }
    void SetAge(odb::nullable<int> age) { age_ = age; }

    odb::nullable<boost::posix_time::ptime> Birthday() const { return birthday_; }
    void SetBirthday(odb::nullable<boost::posix_time::ptime> birthday) { birthday_ = birthday; }
private:
    friend class odb::access; //允许odb访问私有成员
    #pragma db id auto
    size_t id_;

    std::string name_;
    odb::nullable<int> age_;
    odb::nullable<boost::posix_time::ptime> birthday_;
};
