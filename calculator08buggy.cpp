
/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.

	COMMENCING BUG FIX
*/

#include "std_lib_facilities.h"

struct Token {
	char kind; //what kind of Token
	double value; //for numbers: a value
	string name;
	Token(char ch)             :kind(ch), value(0) { } //initialize kind with ch
	Token(char ch, double val) :kind(ch), value(val) { } //initialize kind and value
	Token(char ch, string n)   :kind(ch), name(n) { } //bugfix/initialize kind and name
};

//stream of tokens
class Token_stream {
	bool full; //is there a Token in the buffer?
	Token buffer; //here is where we keep a Token put back using unget()
public:
	Token_stream() :full(0), buffer(0) { } //make a Token_stream that reads from cin

	Token get(); //get a Token (get() is defined elsewhere)
	void unget(Token t) { buffer=t; full=true; }

	void ignore(char); //discard characters up to and include a c
};

const char let = 'L'; //declaration Token
const char quit = 'Q'; //t.kind == quit means that t is a quit Token
const char print = ';'; //t.kind == print means that t is a quit Token 
const char number = '8'; //t.kind == number means that t is a number Token
const char name = 'a'; //name Token

Token Token_stream::get() //read a token from cin and compose a Token
{
	if (full) { full=false; return buffer; }
	char ch;
	cin >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
		return Token(ch); //let each character represent itself
	case '.':			  //a floating-point-literal can start with a dot
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': //numeric literal
	{	cin.unget(); //put digit back into the input stream
		double val;
		cin >> val; //read a floating-point number
		return Token(number,val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s=ch;
			cin.unget();
			if (s == "let") return Token(let);	
			if (s == "quit") return Token(quit); //bugfix
			return Token(name,s);
		}
		error("Bad token");
	}
}

//ignore Tokens up to a certain kind
void Token_stream::ignore(char c) //c represents the kind of Token
{
	//first look in buffer
	if (full && c==buffer.kind) {
		full = false;
		return;
	}
	full = false;

	//now search input:
	char ch;
	while (cin>>ch)
		if (ch==c) return;
}

//--------------------------------------------------------------------

Token_stream ts; //provides get() and putback()

//--------------------------------------------------------------------

//type for (name, value pairs)
struct Variable {
	string name;
	double value;
	Variable(string n, double v) :name(n), value(v) { }
};

//--------------------------------------------------------------------

vector<Variable> names;	

//--------------------------------------------------------------------

//return the value of the Variable named s
double get_value(string s)
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ",s);
}

//set the Variable named s to d
void set_value(string s, double d)
{
	for (int i = 0; i<=names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ",s);
}

//is var already in var_table?
bool is_declared(string s)
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

//--------------------------------------------------------------------

double expression();

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	double d = expression();
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
	}
	case '-':
		return - primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
	while(true) {
		Token t = ts.get();
		switch(t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression()
{
	double left = term();
	while(true) {
		Token t = ts.get();
		switch(t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration()
{
	Token t = ts.get();
	if (t.kind != 'a') error ("name expected in declaration");
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of " ,name);
	double d = expression();
	names.push_back(Variable(name,d));
	return d;
}

double statement()
{
	Token t = ts.get();
	switch(t.kind) {
	case let:
		return declaration();
	default:
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while(true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t=ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch(runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

	try {
		calculate();
		return 0;
	}
	catch (exception& e) {
		cerr << "exception: " << e.what() << endl;
		char c;
		while (cin >>c&& c!=';') ;
		return 1;
	}
	catch (...) {
		cerr << "exception\n";
		char c;
		while (cin>>c && c!=';');
		return 2;
	}