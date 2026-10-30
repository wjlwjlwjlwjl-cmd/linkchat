#include <iostream>
#include <gflags/gflags.h>

int main(int argc, char* argv[]){
    google::ParseCommandLineFlags(&argc, &argv, true);
    return 0;
}