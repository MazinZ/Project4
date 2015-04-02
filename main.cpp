#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
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

string typeGet(string token);
vector<Token> scanner(string s);
string upper(string s);
bool parser(vector<Token> &scanned);

bool showTokens = true;
//string PATH = "";


int main(){
	string command;
	getline(cin, command);
	vector<Token> commandLine;
	bool curLineError;

	commandLine = scanner(command);
	curLineError = parser(commandLine);
	if (showTokens)
	{
		for (int i = 0; i < commandLine.size(); i++){
			cout << "Token Type = ";
			cout << setw(10) << left << commandLine[i].get_type();
			cout << "Token = ";
			cout << setw(20) << left << commandLine[i].get_token();
			cout << "Usage = ";
			cout << setw(15) << left << commandLine[i].get_usage() << endl;
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
		if (scannedLength < 2)
			return founderror;
		// a command can only be a string, variable or word. Otherwise return an error.
		if (scanned[1].get_type()!="string" || scanned[1].get_type()!="variable" || scanned[1].get_type()!="word")
			return founderror;
		if (scanned[0].get_token() == "assignto"){
			// <bg> option isn't allowed with assignto.
			if (scannedLength<3 || scanned.back().get_token() == "<bg>")
				return founderror;
		}
	
	}
	// no error found
	return !founderror;
	
}


