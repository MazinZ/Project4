// CS485, project 4
// Mazin Zakaria, Ethan Gill

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
// Gets the type of each token
	// takes token, returns type
string typeGet(string token);
// Converts a string to an uppercase string for input checking purposes
string upper(string s);
// Returns a tokenized vector
vector<Token> scanner(string s);
// Parses the input, checks for correctness
bool parser(vector<Token> &scanned);
// Runs the given function
void programRun(vector<Token> &parsed);
// Shows info about the variables
void showInfo(vector<Token> tokens);
// Returns the variable value given a variable name
string variableValue(string variable);
// Executes a function when the command is run
bool execute(const char *program, vector<const char*> arguments, bool background);
// Converts a string to a char *
char * convertToCharStar(string argument);
// Parses the PATH variable and returns a tokenized vector
vector<string> pathScanner(string s);
// Executes a function when the command is assignto
bool assigntoExecute(const char *program, vector<const char*> arguments);
// Checks if the variable exists
bool variableExists(string variableName);
// Reads the temporary data file for assignto
string readDataFile();
// Gets arguments to run a function
vector<const char*> getArgs( vector<Token> &parsed, const char * path);

vector<Variable> variableList; // List of all variables
vector<pid_t> processList;
vector<string> PATHVECTOR; // Tokenized "PATH" variable
bool showTokens = true;
string promptToken = "sish >";
//not needed for now, but might be useful later
pid_t statusCode;



int main(){
	string command;
	vector<Token> commandLine;
	bool curLineError = false;

	//take care of PATH

	PATHVECTOR.push_back("/bin");
	PATHVECTOR.push_back("/usr/bin");

	if (!variableExists("PATH")){
		variableList.push_back(Variable("PATH","/bin:/usr/bin"));
	}
	
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
		//if (*command.rbegin()=='\n'){
			//command = command.substr(0, command.size()-1);
		//}
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
		pid_t childpid = waitpid(-1,&statusCode,WNOHANG);

        if(childpid > 0){
            processList.erase(remove(processList.begin(), processList.end(), childpid), processList.end());
            statusCode = 0;
        }

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
		for (size_t i = 1; i < token.length(); i++)
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

   for(size_t i = 0; i < s.length(); i++)
       uppercase += toupper(s[i]);

   return uppercase;
}


////////////////////// Parsing ///////////////////////////

bool parser(vector<Token> &scanned){
	bool founderror = true;
	int scannedLength = (int)scanned.size();
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
			
			for (int i = 3; i < scannedLength; i++)
			{
				ostringstream parameterCount;
				parameterCount << "parameter " << i-2;
				scanned[i].set_usage(parameterCount.str());
			}
			
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

void programRun(vector<Token> &parsed){
	const char * newDirectory;

	if (parsed[0].get_usage()=="listprocs"){
		if(processList.size() == 0){
			cout << "No running processes." << endl;
		} else {
			for (size_t i = 0; i < processList.size(); i++)
			{
				cout << "Process " << i+1 << ": PID " << processList[i] << endl;
			}
		}
	}

	else if (parsed[0].get_usage()=="cd"){
			if (parsed[1].get_token().c_str()[0] == '$'){
				newDirectory = variableValue(parsed[1].get_token()).c_str();
			}
			else
				newDirectory = parsed[1].get_token().c_str();
			
		
		if(chdir(newDirectory) != 0)
			perror("cd failed");
	}
	
	else if (parsed[0].get_usage()=="defprompt") {
		if (parsed[1].get_token().c_str()[0] == '$'){
			promptToken = variableValue(parsed[1].get_token());
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
			
			PATHVECTOR.clear();
			PATHVECTOR = pathScanner(parsed[2].get_token());
			for (size_t i = 0; i < variableList.size(); i++){
				if (variableList[i].get_name()=="PATH")
					variableList[i].set_value(parsed[2].get_token());
			}
		}

		bool exists = false;
		for (size_t i = 0; i < variableList.size(); i++){
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
	else if (parsed[0].get_token() == "run" && parsed[1].get_token()=="echo" && parsed[2].get_token()=="$PATH"){
		vector<const char*> arguments;
		arguments.push_back("/bin/echo");
		arguments.push_back(variableValue("$PATH").c_str());
		arguments.push_back(0);

		execute("/bin/echo", arguments, false);
	}
	else if (parsed[0].get_usage()=="run"){
		bool backgrounded = false;

		//take care of background forking
		if(parsed.back().get_usage() == "<bg>"){
			backgrounded = true;
			parsed.pop_back();
		}

		//it turns out we have to fork regardless of <bg>
	
		char finalPath[1024];
		vector<const char*> arguments;

		// copy arguments (not run or filename) into the arguments array

		    if(parsed[1].get_token().c_str()[0] == '/'){
		    	//run directly, passing arguments
						arguments = getArgs(parsed,parsed[1].get_token().c_str());
						execute(parsed[1].get_token().c_str(),arguments,backgrounded);
		    	
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
				arguments = getArgs(parsed,finalPath);
				execute(finalPath,arguments, backgrounded);
				//const char * newDirectory = parsed[1].get_token().c_str();
				
			}
			
			//search PATH for program of that name, run, passing arguments
			else {
				bool pathFound = false;
				string correctPath = "";
				for (size_t i = 0; i<PATHVECTOR.size(); i++){
					correctPath = PATHVECTOR[i]+"/"+parsed[1].get_token();
					if(fileExists(correctPath)){
						
						pathFound = true;
						break;
					}
				}
				if (pathFound){
					//arguments[0]=convertToCharStar(correctPath.c_str());
					//arguments[numArgs] = NULL;	
					arguments = getArgs(parsed,correctPath.c_str());
					execute(correctPath.c_str(),arguments, backgrounded);

				}
			
							
			else {
				// if it's not a variable then it must be an error
				if (parsed[1].get_token().c_str()[0] != '$'){
					//file not found
					errno = ENOENT;
					perror("run");
					}
				else {
					string varval = variableValue((parsed[1].get_token()));
					if(varval!=""){
						arguments = getArgs(parsed, varval.c_str());
						
						execute(varval.c_str(),arguments,backgrounded);
					
					}
				}
				
				}
		    	
		    }

	}
	
	else if (parsed[0].get_usage()=="assignto"){
		char finalPath[1024];
		bool success = false;
		vector<const char*> arguments;
	 		if((parsed[2].get_token()).c_str()[0] == '/'){
					success = true;
					arguments = getArgs(parsed,parsed[2].get_token().c_str());

					assigntoExecute(parsed[2].get_token().c_str(),arguments);
		    } 
			
			//search Current Working Directory for program, run, passing arguments
			else if (parsed[2].get_token().c_str()[0] == '.' && parsed[2].get_token().c_str()[1] == '/'){
				char currentDirectory[512];
				string programName = parsed[2].get_token();
				programName.erase(0,1);
				strcpy(finalPath,getcwd(currentDirectory, sizeof(currentDirectory)));
				strcat(finalPath,programName.c_str());
				success = true;
				arguments = getArgs(parsed,finalPath);

				assigntoExecute(finalPath,arguments);
				
			}
			else {
				bool pathFound = false;
				string correctPath = "";
				for (size_t i = 0; i<PATHVECTOR.size(); i++){
					correctPath = PATHVECTOR[i]+"/"+parsed[2].get_token();
					if(fileExists(correctPath)){
						pathFound = true;
						break;
					}
				}
				if (pathFound){
					success = true;
					arguments = getArgs(parsed,correctPath.c_str());
					assigntoExecute(correctPath.c_str(),arguments);

				}
				else {
					//file not found
					errno = ENOENT;
					perror("assignto");
				}
				
		    	
		    }

		
	if (success){
		bool exists = false;
		string varName = parsed[1].get_token();
		string varName2 = varName.erase(0,1);

		if (variableExists(parsed[1].get_token())) {
			for (size_t i = 0; i < variableList.size(); i++){
				if (varName2 == variableList[i].get_name()){
					variableList[i].set_value(readDataFile());
					exists = true;
					break;
				}
					//remove("./tmpdata");
			}
		}
		if(!exists) {
			variableList.push_back(Variable(varName2, readDataFile()));
				//remove("./tmpdata");
			}
				
		}
	  
	}
	
	
		
		
	}


void showInfo(vector<Token> tokens){
	if (showTokens){
 		for (size_t i = 0; i < tokens.size(); i++){
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
	for (size_t i = 0; i < variableList.size(); i++){
		if (variableList[i].get_name()==variableName)
			return variableList[i].get_value();
	}
	return "";
}

// This is only used for the run command
bool execute(const char *program, vector<const char*> arguments, bool background){
    pid_t pid;
    int state;
	bool failed = true;
	
	if ((pid = fork()) < 0)
				return failed;
	else if (pid == 0) {          /* child process       */
		if (execv(program, (char**)&arguments[0]) < 0)     /* execute  */
				return failed;
		}
	else {                     
		if(!background){             
			while (wait(&state) != pid);     /* parent waits for completion (we don't execute this if bg is enabled)  */
		} else {
			//bg is enabled. Add the PID to the list.
			processList.push_back(pid);
		}	 
			 
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
	s+=":";
	vector<string> pathVector;
	size_t pos = 0;
	string token;
	while ((pos = s.find(":")) != string::npos) {
		token = s.substr(0, pos);
		if (token!=" " && token!="" && token!="\n")
			pathVector.push_back(token);
		
		s.erase(0, pos + 1);
		if (token == "")
			s.erase(0,1);
	}
	return pathVector;
}

bool assigntoExecute(const char *program, vector<const char*> arguments){
	
	pid_t pid;
	int state;
	const char * file = "./tmpdata";
	bool failed = true;
	
	remove("./tmpdata");
	if ((pid = fork()) < 0)      
				return failed;
	else if (pid == 0) { 
		int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		dup2(fd, 1);   
		dup2(fd, 2);   
		close(fd);
		
		if (execv(program, (char**)&arguments[0]) < 0)   /* execute  */
				return failed;
		
		}
	else {                                  
		while (wait(&state) != pid);     /* parent waits for completion always with assignto */
								 
			 
	}
	return !failed;
}

string readDataFile(){
	stringstream fileContents;
	ifstream stream("./tmpdata");
	if(stream.is_open()){
		while(stream.peek() != EOF) {
			fileContents << (char) stream.get();
		}
	stream.close();
	return fileContents.str();
	}
	return "";
}

bool variableExists(string variableName){
	string variablenam = variableName;
	if (variableName.c_str()[0] == '$')
		variablenam.erase(0,1);
	for (size_t i = 0; i < variableList.size(); i++){
		if (variablenam==variableList[i].get_name()){
			return true;
	}
}
return false;
}

vector< const char*> getArgs( vector<Token> &parsed, const char * path){
	vector< const char*> arguments;
	string command = "";
	size_t i = 0;
	char *arg;
	arguments.push_back(path);

	if (parsed[0].get_usage()=="run")
		i = 2;
	if (parsed[0].get_usage()=="assignto" || (parsed[0].get_usage()=="run" && parsed[1].get_type()=="variable" && parsed[1].get_usage()=="cmd" && parsed[1].get_token().c_str()[0]!='#' ))
		i = 2;

	for (; i < parsed.size(); i++){
		if (parsed[i].get_usage()!="run" && parsed[i].get_usage()!="cmd" && parsed[i].get_usage()!="assignto" && parsed[i].get_type()!="variable" && parsed[i].get_usage()!="variable"  && parsed[1].get_token().c_str()[0]!='#'){
			if (parsed[i].get_token()!="$PATH"){ 

			arg = new char [parsed[i].get_token().length() + 1];

			strcpy(arg,parsed[i].get_token().c_str());
			arguments.push_back(arg);
			}

		}
		if ((parsed[i].get_type()=="variable" || parsed[i].get_usage()=="variable" )){
			if (parsed[i].get_token()!="$PATH"){ 
				arg = new char [variableValue(parsed[i].get_token()).length() + 1];
				strcpy(arg,variableValue(parsed[i].get_token()).c_str());
				arguments.push_back(arg);
			}
		}
	}
	
		
		arguments.push_back(0);
	
	//delete arg;
	return arguments;
}

