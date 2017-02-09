#ifndef ARGUMENTSMANAGER_H_
#define ARGUMENTSMANAGER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <string>

#include <errno.h>

class ArgumentsManager{
private:
	std::map<std::string, std::string> args_list;
public:
	ArgumentsManager(int argc, char* argv[]);
	int parse_args(int argc, char* argv[]);
	std::map<std::string, std::string> get_args_list();
	const char* get_src_addr();
	void help();
};

#endif /* ARGUMENTSMANAGER_H_ */
