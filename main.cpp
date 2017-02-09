#include <iostream>
#include <cstdlib>
#include <string>

#include "scan_functions.h"
#include "ArgumentsManager.h"

using namespace std;

void args_manager(int argc, char * argv[]);

int main(int argc, char * argv[]) {

	ArgumentsManager argumentsManager = ArgumentsManager(argc, argv);	
	std::string src = (((argumentsManager.get_args_list()).find("src"))->second);
	std::string dst = (((argumentsManager.get_args_list()).find("dst"))->second);
	std::string port = (((argumentsManager.get_args_list()).find("port"))->second);

	SYN_scan(src.c_str(), dst.c_str(), port.c_str());

	return 0;

}

