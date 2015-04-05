#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sys/wait.h>
#include <iomanip>
#include <fstream>
#include <errno.h>
#include <fcntl.h>
using namespace std;

struct Token{
public:
	Token(string t1, string t2)
	{
		type = t2;	
		token = t1;
		usage = "";
	}
	Token(string t1, string t2, string t3)
	{
		type = t2;	
		token = t1;
		usage = t3;
	}
	void set_usage(string t3){
		usage = t3;
	}
	string get_token()
	{
		return token;
	}
	string get_type(){
		return type;
	}
	string get_usage(){
		return usage;
	}
private:
	string type;
	string token;
	string usage;
};

struct Variable{
	public:
		Variable(string n, string v)
		{
			name = n;
			value = v;	
		}
		string get_name()
		{
			return name;
		}
		string get_value() {
			return value;
		}
		void set_value(string v){
			value = v;
		}
	private:
		string name;
		string value;
		
};

string typeGet(string token);
string upper(string s);
vector<Token> scanner(string s);
bool parser(vector<Token> &scanned);
void programRun(vector<Token> parsed);
void showInfo(vector<Token> tokens);
string variableValue(string variable);
bool execute(const char *program, char *const *arguments, bool background);
char * convertToCharStar(string argument);
vector<string> pathScanner(string s);
bool assigntoExecute(const char *program, char *const *arguments);
bool variableExists(string variableName);
string readfile();


vector<Variable> variableList;
vector<string> PATH;
bool showTokens = true;
string promptToken = "sish >";
//not needed for now, but might be useful later
int childStatusCode = 0;

int main(){
	string command;
	vector<Token> commandLine;
	bool curLineError = false;

	//take care of PATH
	PATH.push_back("/bin");
	PATH.push_back("/usr/bin");
	
	//initial prompt
	cout << promptToken;

	while(getline(cin, command)){

		if (cin.bad()) {
	    	// IO error
		} else if (!cin.eof()) {
	    // format error (not possible with getline but possible with operator>>)
		} else {
	    	// format error (not possible with getline but possible with operator>>)
	    	// or end of file (can't make the difference)
	    	return 0;
		}

		//scanner
		commandLine = scanner(command);

		//parser
		curLineError = parser(commandLine);

		if(!curLineError){
			//show tokens
			showInfo(commandLine);

			//runner
			programRun(commandLine);
		}

		//prompt for next run of while
		cout << promptToken;

	}
	return 0;
}


vector<Token> scanner(string s) {
	vector<Token> parsed;
	bool stringFound = false;
	s+=" ";
	replace(s.begin(),s.end(), '\t', ' ');
	string token = s.substr(0, s.find(" "));
	size_t pos = 0;
	string delimiter = " ";
	while ((pos = s.find(delimiter)) != string::npos ) {
		token = s.substr(0, pos);
		size_t stringCheck = s.find("\"");
			if (stringCheck < pos && stringCheck!=string::npos){
				s.erase(0,stringCheck+1);
				size_t endString = s.find("\"");
				parsed.push_back(Token(s.substr(stringCheck,endString),"string"));
				s.erase(0,endString+1);
				stringFound = true;
			}
			if (token != "" && !stringFound){ 
				parsed.push_back(Token(token, typeGet(token)));
				s.erase(0, pos + (delimiter).length());
			} 
			stringFound = false;
			if (token == "")
				s.erase(0,1);
	}
	return parsed;
}

string typeGet(string token){
	bool validVariable = true;
	if (upper(token) == "DEFPROMPT" || upper(token) == "CD" || upper(token) == "LISTPROCS" || upper(token) ==
		"BYE" || upper(token) == "RUN" || upper(token) == "ASSIGNTO" || upper(token) == "<BG>") {
		return "keyword";
	}
	else if (token.length() == 1 && (token.c_str()[0] == '#' || token.c_str()[0] == '=')) {
		return "metaChar";
	}
	else if (token.c_str()[0] == '$' && token.length() > 1)
	{
		for (int i = 1; i < token.length(); i++)
		{
			if (!isalnum(token[i]))
			{
				validVariable = false;
				return "word";
			}			
		}
		if (validVariable)
			return "variable";
	}
	//else
	return "word";
	
}

string upper(string s){
   string uppercase = "";

   for(int i = 0; i < s.length(); i++)
       uppercase += toupper(s[i]);

   return uppercase;
}


////////////////////// Parsing ///////////////////////////

bool parser(vector<Token> &scanned){
	bool founderror = true;
	int scannedLength = scanned.size();
	if (scannedLength == 0)
		return founderror;
		
	// if the first token is #, it's a comment
	// assign the comment usage to the # and the anyText usage to the rest of the line
	if (scanned[0].get_token() == "#"){
		scanned[0].set_usage("comment");
		for (int i = 1; i < scannedLength; i++)
		{
			scanned[i].set_usage("anyText");
		}
		return !founderror;
	}

	// if the first token is a keyword
	if (scanned[0].get_type()== "keyword") {
		if (upper(scanned[0].get_token())== "DEFPROMPT")
		{
			//defprompt expects 2 parameters
			if (scannedLength < 2){
				errno = EINVAL;
				perror("defprompt");
				return founderror;
			}
			if (scannedLength > 2){
				errno = E2BIG;
				perror("defprompt");
				return founderror;
			}
			scanned[0].set_usage("defprompt");
			scanned[1].set_usage("prompt");
			//promptToken = scanned[1].get_token();
			return !founderror;
		}
		if (upper(scanned[0].get_token())== "CD")
		{
			if (scannedLength < 2){
				errno = EINVAL;
				perror("cd");
				return founderror;
			}
			if (scannedLength > 2){
				errno = E2BIG;
				perror("cd");
				return founderror;
			}

			scanned[0].set_usage("cd");
			scanned[1].set_usage("directoryName");
			return !founderror;
		}
		if (upper(scanned[0].get_token())== "LISTPROCS")
		{
			if (scannedLength != 1){
				errno = E2BIG;
				perror("listprocs");
				return founderror;
			}
			scanned[0].set_usage("listprocs");
			return !founderror;
		}
		if (upper(scanned[0].get_token())== "BYE")
		{
			if (scannedLength != 1){
				errno = E2BIG;
				perror("bye");
				return founderror;
			}
			scanned[0].set_usage("bye");
			return !founderror;
		}
		if (upper(scanned[0].get_token())== "RUN")
		{
			if (scannedLength < 2){
				errno = EINVAL;
				perror("run");
				return founderror;
			}
			scanned[0].set_usage("run");
			scanned[1].set_usage("cmd");
			string parameterCount;
			
			for (int i = 2; i < scannedLength; i++)
			{
				ostringstream parameterCount;
				parameterCount << "parameter " << i-1;
				scanned[i].set_usage(parameterCount.str());
			}

			if (upper(scanned[scannedLength-1].get_token()) == "<BG>")
			{
				//mark the ending token usage as <bg>
				scanned[scannedLength-1].set_usage("<bg>"); 
			}
			return !founderror;
		}
		if (upper(scanned[0].get_token())== "ASSIGNTO")
		{
			if (scannedLength < 3){
				//too few arguments
				errno = EINVAL;
				perror("assignto");
				return founderror;
			}
			scanned[0].set_usage("assignto");
			if (scanned[1].get_type()!="variable")
			{
				//invalid variable
				errno = EINVAL;
				perror("assignto");
				return founderror;
			}
			scanned[1].set_usage("variable");
			if (scanned[2].get_type()!="string" && scanned[2].get_type()!="variable" && scanned[2].get_type()!="word"){
				//Commands must be in the form of a string, variable, or word.
				errno = EINVAL;
				perror("assignto");
				return founderror;
			}
			scanned[2].set_usage("cmd");
			return !founderror;
		}
    }

	// variable form [variable, =, value]
	if(scanned[0].get_type()== "word" && scannedLength > 1 && scanned[1].get_token()== "="){
		// if there aren't exactly 3 tokens (variable, assignment, value), it's an error
		if (scannedLength != 3){
			errno = EINVAL;
			perror("sish variable assignment");
			return founderror;
		}

		scanned[0].set_usage("variable");
		scanned[1].set_usage("assignment");
		scanned[2].set_usage("variableDef");
		return !founderror;
	}

	//if we reach this point, we have no idea how to parse the command. Set all usages to anyText, show an error
	for (int i = 0; i < scannedLength; i++)
	{
		scanned[i].set_usage("anyText");
	}
	errno = ENOSYS;
	perror("sish");
	return founderror;
		
}

bool fileExists(string filepath)
{
	ifstream ifile(filepath.c_str());
	if (ifile) 
		return true;
	else {
		return false;
	}
}

void programRun(vector<Token> parsed){
	int parsedLength = parsed.size();
	const char * newDirectory;

	if (parsed[0].get_usage()=="cd"){
			char directory[1024];	
			if (parsed[1].get_token().c_str()[0] == '$'){
				newDirectory = variableValue(parsed[1].get_token()).c_str();
				//TODO: Check if newDirectory is equal to "". If so, the variable supplied does not exist.
			}
			else
				newDirectory = parsed[1].get_token().c_str();
			
		
		if(chdir(newDirectory) != 0)
			perror("cd failed");
	}
	
	else if (parsed[0].get_usage()=="defprompt") {
		if (parsed[1].get_token().c_str()[0] == '$'){
			promptToken = variableValue(parsed[1].get_token());
			//TODO: Check if promptToken is equal to "". If so, the variable supplied does not exist.
		}
		else
			promptToken = parsed[1].get_token();
	}
	else if (parsed[0].get_usage()=="variable" && parsed[1].get_usage()=="assignment"){

		if (parsed[0].get_token() == "ShowTokens")
		{
			if(parsed[2].get_token() == "1"){
				showTokens = true;
			} else {
				showTokens = false;
			}
		} else if (parsed[0].get_token() == "PATH")
		{
			PATH.clear();
			PATH = pathScanner(parsed[2].get_token());

		}

		bool exists = false;
		for (int i = 0; i < variableList.size(); i++){
			if (parsed[0].get_token() == variableList[i].get_name()){
				variableList[i].set_value(parsed[2].get_token());
				exists = true;
				break;
			}
			
		}
		if (!exists){
			variableList.push_back(Variable(parsed[0].get_token(),parsed[2].get_token()));
		}
	}
	else if (parsed[0].get_usage()=="bye"){
		exit(0);
	}
	else if (parsed[0].get_usage()=="run"){
		bool backgrounded = false;

		//take care of background forking
		if(parsed.back().get_usage() == "<bg>"){
			backgrounded = true;
			parsed.pop_back();
		}

		//it turns out we have to fork regardless of <bg>
	
		int numArgs = parsed.size()-1;
		char *arguments[numArgs+1];
		char finalPath[1024];
		
		// copy arguments (not run or filename) into the arguments array
		for (int i = 1; i < parsed.size(); i++){
			char *converted = convertToCharStar(parsed[i+1].get_token());
			arguments[i] = converted;
		}
		arguments[0] = convertToCharStar(parsed[1].get_token());
		arguments[numArgs] = NULL;

	   // if(forkResult == 0){
			
	      	//I am the child process. Run the code
	      	//TODO: check for <bg> and act accordingly

		    if(parsed[1].get_token().c_str()[0] == '/'){
		    	//run directly, passing arguments

		
			/*for (int i = 1; i < parsed.size(); i++){
					char *converted = convertToCharStar(parsed[i+1].get_token());
					arguments[i] = converted;
				}
				arguments[numArgs] = NULL;*/

				execute(parsed[1].get_token().c_str(),arguments,false);
		    	
		    } 
			
			//search Current Working Directory for program, run, passing arguments
			else if (parsed[1].get_token().c_str()[0] == '.' && parsed[1].get_token().c_str()[1] == '/'){
				char currentDirectory[512];
				// program name in form of ./name
				string programName = parsed[1].get_token();
				// remove the "." in front
				programName.erase(0,1);
				// copy the current working directory to the final path
				strcpy(finalPath,getcwd(currentDirectory, sizeof(currentDirectory)));
				// concatenate path above with the program name
				strcat(finalPath,programName.c_str());
				
				/*for (int i = 2; i < parsed.size(); i++){
					char *converted = convertToCharStar(parsed[i].get_token());
					arguments[i] = converted;
					
				}
				arguments[numArgs] = NULL;*/

				// execute using final path and arguments
				execute(finalPath,arguments, false);
				//const char * newDirectory = parsed[1].get_token().c_str();
				
			}
			
			//search PATH for program of that name, run, passing arguments
			else {
				bool pathFound = false;
				string correctPath = "";
				for (int i = 0; i<PATH.size(); i++){
					correctPath = PATH[i]+"/"+parsed[1].get_token();
					if(fileExists(correctPath)){
						
						pathFound = true;
						break;
					}
				}
				if (pathFound){
					//arguments[0]=convertToCharStar(correctPath.c_str());
					//arguments[numArgs] = NULL;					
					execute(correctPath.c_str(),arguments, false);

				}
				else {
					//file not found
					errno = ENOENT;
					perror("run");
				}
		    	
		    }

	    /* Goes with the fork==0 line above} 
	
		else {
	    	//I am the parent process. 
	    	//Wait for the child if <bg> is called or continue with prompt if <bg> is not called
	    	//I'll also need to add the child process to the list of running processes
	    	if(backgrounded == true){
	    		//TODO: keep track of the process
	    	}else{
	    		//wait for the process to end
	    		waitpid(forkResult, &childStatusCode, 0);
	    	}
	    }*/

	}
	
	else if (parsed[0].get_usage()=="assignto"){	
		int numArgs = parsed.size()-1;
		char *arguments[numArgs+1];
		char finalPath[1024];
		bool success = false;
		
		for (int i = 1; i < parsed.size(); i++){
			char *converted = convertToCharStar(parsed[i+2].get_token());
			arguments[i] = converted;
		}
		arguments[0] = convertToCharStar(parsed[2].get_token());
		arguments[numArgs] = NULL;

	 		if(parsed[1].get_token().c_str()[0] == '/'){
					success = true;
					assigntoExecute(parsed[2].get_token().c_str(),arguments);
		    } 
			
			//search Current Working Directory for program, run, passing arguments
			else if (parsed[2].get_token().c_str()[0] == '.' && parsed[1].get_token().c_str()[2] == '/'){
				char currentDirectory[512];
				string programName = parsed[2].get_token();
				programName.erase(0,1);
				strcpy(finalPath,getcwd(currentDirectory, sizeof(currentDirectory)));
				strcat(finalPath,programName.c_str());
				success = true;
				assigntoExecute(finalPath,arguments);
				
			}
			else {
				bool pathFound = false;
				string correctPath = "";
				for (int i = 0; i<PATH.size(); i++){
					correctPath = PATH[i]+"/"+parsed[2].get_token();
					if(fileExists(correctPath)){
						
						pathFound = true;
						break;
					}
				}
				if (pathFound){
					success = true;
					assigntoExecute(correctPath.c_str(),arguments);

				}
				else {
					//file not found
					errno = ENOENT;
					perror("assignto");
				}
		    	
		    }

	  
	}
	
	if (success){
		if (variableExists(parsed[1].get_token()) {
			for (int i = 0; i < variableList.size(); i++){
				if (variableName == variableList[i].get_name())
					variableList[i].set_value(readfile());
					remove("./tmpdata");
			}
		}
		else 
			variableList.push_back(Variable(parsed[1].get_token(), readfile()));{
				remove("./tmpdata");
			}
				
			}
		
		
	}


void showInfo(vector<Token> tokens){
	if (showTokens){
 		for (int i = 0; i < tokens.size(); i++){
 			cout << "Token Type = ";
 			cout << setw(10) << left << tokens[i].get_type();
 			cout << "Token = ";
 			cout << setw(20) << left << tokens[i].get_token();
 			cout << "Usage = ";
 			cout << setw(15) << left << tokens[i].get_usage() << endl;
 		}
 	}
}

// When a parameter with a $ in front is detected, it is passed to this function
// and the value of the variable is returned.
string variableValue(string variable) {
	string variableName = variable.erase(0,1);
	for (int i = 0; i < variableList.size(); i++){
		if (variableList[i].get_name()==variableName)
			return variableList[i].get_value();
	}
	return "";
}

bool execute(const char *program, char *const *arguments, bool background) {
	pid_t pid;
	int state;
	bool failed = true;
	
	if ((pid = fork()) < 0)      
				return failed;
	else if (pid == 0) {          /* child process       */
		if (execv(program, arguments) < 0)     /* execute  */
				return failed;
		}
	else {                                  
		while (wait(&state) != pid);     /* parent waits for completion (I guess we don't execute this if bg is enabled?)  */
								 
			 
	}
	return !failed;
	
}

char * convertToCharStar(string argument){
	//char returnval[(argument).length()+1];
	char* returnval = (char *)malloc(argument.length()+1);
	strcpy(returnval, argument.c_str());
	return returnval;
}

vector<string> pathScanner(string s) {
	s+=";";
	vector<string> pathVector;
	int pos = 0;
	string token;
	while ((pos = s.find(";")) != string::npos) {
		token = s.substr(0, pos);
		if (token!=" " && token!="" && token!="\n")
			pathVector.push_back(token);
		
		s.erase(0, pos + 1);
		if (token == "")
			s.erase(0,1);
	}
	return pathVector;
}

bool assigntoExecute(const char *program, char *const *arguments){
	
	pid_t pid;
	int state;
	string file = "./tmpdata";
	char *filename;
	strcpy(filename,file.c_str());
	bool failed = true;
	
	if ((pid = fork()) < 0)      
				return failed;
	else if (pid == 0) { 
		int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		dup2(fd, 1);   
		dup2(fd, 2);   
		close(fd);
		
		if (execv(program, arguments) < 0)     /* execute  */
				return failed;
		}
	else {                                  
		while (wait(&state) != pid);     /* parent waits for completion (I guess we don't execute this if bg is enabled?)  */
								 
			 
	}
	return !failed;
}

string readFile(){
	stringstream fileContents;
	ifstream stream("./tmpdata");
	if(stream.is_open()){
		while(stream.peek() != EOF) {
			fileContents << (char) stream.get();
		}
	stream.close();
	return fileContents.str();
	}
}

bool variableExists(string variableName){
	for (int i = 0; i < variableList.size(); i++){
		if (variableName == variableList[i].get_name()){
			return true;
	}
	return false;
}

