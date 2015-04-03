#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
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

vector<Variable> variableList;
vector<string> PATH;
bool showTokens = true;
string promptToken = "sish >";

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
	if (upper(token) == "DEFPROMPT" || upper(token) == "CD" || upper(token) == "LISTJOBS" || upper(token) ==
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
	
	// if the first token is a word
	if (scanned[0].get_type()== "word") {

		// variable form [variable, =, value]
		if(scanned[1].get_token()== "="){
			// if there aren't exactly 3 tokens (variable, assignment, value), it's an error
			if (scannedLength != 3){
				return founderror;
			}

			scanned[0].set_usage("variable");
			scanned[1].set_usage("assignment");
			scanned[2].set_usage("variableDef");
		}
		return !founderror;
	}

	// if the first token is a word
	if (scanned[0].get_type()== "keyword") {
		if (upper(scanned[0].get_token())== "DEFPROMPT")
		{
			//defprompt expects 2 parameters
			if (scannedLength != 2){
				perror("defprompt expects 2 parameters");
				return founderror;
			}
			scanned[0].set_usage("defprompt");
			scanned[1].set_usage("prompt");
			//promptToken = scanned[1].get_token();

		}
		if (upper(scanned[0].get_token())== "CD")
		{
			if (scannedLength < 2){
				perror("cd must be accompanied by a directory");
				return founderror;
			}
			if (scannedLength > 2){
				perror("cd expects 2 parameters");
				return founderror;
			}

			scanned[0].set_usage("cd");
			scanned[1].set_usage("directoryName");

		}
		if (upper(scanned[0].get_token())== "LISTPROCS")
		{
			if (scannedLength != 1){
				perror("listprocs does not expect more than 1 parameter");
				return founderror;
			}
			scanned[0].set_usage("listprocs");
		}
		if (upper(scanned[0].get_token())== "BYE")
		{
			if (scannedLength != 1){
				perror("bye does not expect more than 1 parameter");
				return founderror;
			}
			scanned[0].set_usage("bye");
		}
		if (upper(scanned[0].get_token())== "RUN")
		{
			if (scannedLength < 2){
				perror("run expects 2 or more parameters");
				return founderror;
			}
			scanned[0].set_usage("run");
			//TODO: make sure cmd is a valid command
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

		}
		if (upper(scanned[0].get_token())== "ASSIGNTO")
		{
			if (scannedLength < 3){
				perror("assignto expects 3 or more parameters");
				return founderror;
			}
			scanned[0].set_usage("assignto");
			if (scanned[1].get_type()!="variable")
			{
				perror("invalid variable");
				return founderror;
			}
			scanned[1].set_usage("variable");
			if (scanned[2].get_type()!="string" && scanned[2].get_type()!="variable" && scanned[2].get_type()!="word"){
				perror("Commands must be in the form of a string, variable, or word.");
				return founderror;
			}
			scanned[2].set_usage("cmd");
		}

		return !founderror;
	}
	
	if (scanned[0].get_token()=="cd"){
		// we don't need to handle cd by itself, so return error
		if (scannedLength==1)
			return founderror;
		// a variable can only have a value that is a word or a string, otherwise return error
		if (scanned[1].get_type()!= "word" || scanned[1].get_type()!="string")
			return founderror;
		// listprocs and bye must be alone on the line
		if ((scanned[0].get_token() == "listprocs" || scanned[0].get_token() == "bye") && scannedLength>1)
			return founderror;
	}
	
	if (scanned[0].get_token() == "run"){
		// run has to at least have two tokens, run and a command
		// a command can only be a string, variable or word. Otherwise return an error.

		if (scanned[0].get_token() == "assignto"){
			// <bg> option isn't allowed with assignto.
			if (scannedLength<3 || scanned.back().get_token() == "<bg>")
				return founderror;
		}
	
	}
	// no error found
	return !founderror;
	
}

void programRun(vector<Token> parsed){
	int parsedLength = parsed.size();

	if (parsed[0].get_usage()=="cd"){
			char directory[1024];			
			const char * newDirectory = parsed[1].get_token().c_str();
			if(chdir(newDirectory) == 0) {
				
			  }
				
	}
	else if (parsed[0].get_usage()=="defprompt") {
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
			
		}

		bool exists = false;
		for (int i = 0; i < variableList.size(); i++){
			if (parsed[0].get_token() == variableList[i].get_name()){
				variableList[i].set_value(parsed[2].get_token());
				exists = true;
				break;
			}
			if (!exists){
				variableList.push_back(Variable(parsed[0].get_token(),parsed[2].get_token()));
			}
		}
	}
	else if (parsed[0].get_usage()=="bye"){
		exit(0);
	}
	else if (parsed[0].get_usage()=="run"){
		bool backgrounded = false;
		pid_t forkResult = 0;
		if(parsed.back().get_usage() == "<bg>"){
			backgrounded = true;
			parsed.pop_back();
			forkResult = fork();
		}
	}

    if(forkResult == 0){
      	//I am the child process, or the parent without <bg>. Run the code
	        
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

