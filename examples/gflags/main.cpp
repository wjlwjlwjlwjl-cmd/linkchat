#include <gflags/gflags.h>
#include <iostream>

//参数行变量名、默认值、帮助信息
DEFINE_bool(bool_val, true, "Enable flag bool_val");
DEFINE_int32(int_val, 100, "Value of flag integer");
DEFINE_string(string_val, "Hello World", "Value o String");

int main(int argc, char** argv) {
    //第三个参数会移除命令行参数中 gflags 的参数，并修改 argc
    google::ParseCommandLineFlags(&argc, &argv, true);

    //命令行指定参数 --bool_val=false，或者指定 --flagfile=path/to/file
    std::cout << FLAGS_bool_val << std::endl;
    std::cout << FLAGS_int_val << std::endl;
    std::cout << FLAGS_string_val << std::endl;

    return 0;
}