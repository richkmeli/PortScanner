#include "ArgumentsManager.h"

ArgumentsManager::ArgumentsManager(int argc, char* argv[]){

	//DEFAULT
	args_list["src"] = get_src_addr();
	args_list["dst"] = "127.0.0.1";
	args_list["port"] = "80";

	parse_args(argc, argv);

}

int ArgumentsManager::parse_args(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
	  if(strcmp(argv[i],"-h") == 0){
	    		help();
			return 1;
	  }else if(strcmp(argv[i],"-v") == 0){
	    		args_list["verbose"] = "true";
	  }else if(strcmp(argv[i],"-s") == 0){
	  		std::string s = argv[++i];
			args_list["src"] = s;
	  }else if(strcmp(argv[i],"-d")	== 0){
	  		std::string s = argv[++i];
			args_list["dst"] = s;
	  }else if(strcmp(argv[i],"-p") == 0){
	    		std::string s = argv[++i];
			int a = std::stoi(s);
			if (a < 0 && a > 65535) {
			  std::cout << "Error: wrong port [0 <-> 65535]";
				help();
				return 1;
			} else
			  args_list["port"] = s;
	  }else{
	    std::cout << "Error: wrong arguments\n";
			help();
			return 1;
	  }
	}
	return 0;
}

std::map<std::string, std::string> ArgumentsManager::get_args_list() {
	return args_list;
}

void ArgumentsManager::help() {
std::cout << "help...";}

const char* ArgumentsManager::get_src_addr(){
  setenv("LANG","C",1);
	        FILE * fp = popen("ifconfig eth0", "r");
		std::string s;	                
	        if (fp) {
			char *p=NULL; 
			size_t n;
	                while ((getline(&p, &n, fp) > 0) && p) {
	                   	s.append(p);
			}
			int pos1 = s.find("inet");
			int pos2 = s.find("netmask");
			s = s.substr((pos1+5), ((pos2-2)-(pos1+5)));
	        	pclose(fp);
		}
	        return s.c_str();
}

