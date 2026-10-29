#include "test.pb.h"
#include <google/protobuf/util/json_util.h>

int main() {
    test::Person person;
    person.set_name("Joe");
    person.set_age(19);
    person.add_skills("C++");
    person.add_skills("Java");
    person.add_skills("Python");

    //对于 map 的修改要通过字段指针进行
    auto score_ptr = person.mutable_scores();
    score_ptr->insert({"Chinese", 99});
    score_ptr->insert({"Math", 98});

    google::protobuf::util::JsonOptions options; // 实例化配置对象
    options.add_whitespace = true;

    std::string json_str;
    auto status = google::protobuf::util::MessageToJsonString(person, &json_str, options);
    if (!status.ok()) {
        std::cout << "Parsing failed: " << status.error_message() << std::endl;
        return 1;
    }
    std::cout << json_str << std::endl;
    std::cout << "===========" << std::endl;

    test::Person person2;
    status = google::protobuf::util::JsonStringToMessage(json_str, &person2);
    if (!status.ok()) {
        std::cout << "Parsing failed: " << status.error_message() << std::endl;
        return 1;
    }
    status = google::protobuf::util::MessageToJsonString(person2, &json_str, options);

    if (!status.ok()) {
        std::cout << "Parsing failed: " << status.error_message() << std::endl;
        return 1;
    }

    std::cout << json_str << std::endl;
    return 0;
}
