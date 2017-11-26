
/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.

	COMMENCING BUG FIX
*/

#include "std_lib_facilities.h"

class Token {
public:
	char kind; //what kind of Token
	double value; //for numbers: a value
	string name;
	Token(char ch)             :kind(ch), value(0) { } //initialize kind with ch
	Token(char ch, double val) :kind(ch), value(val) { } //initialize kind and value
	Token(char ch, string n)   :kind(ch), name(n) { } //bugfix/initialize kind and name
};

//stream of tokens
class Token_stream {
private:
	bool full; //is there a Token in the buffer?
	Token buffer; //here is where we keep a Token put back using unget()
public:
	Token_stream() :full(0), buffer(0) { } //make a Token_stream that reads from cin

	Token get(); //get a Token (get() is defined elsewhere)
	void unget(Token t) { buffer=t; full=true; }

	void ignore(char); //discard characters up to and include a c
};

const char let = 'L';          //declaration Token
const char quit = 'q';         //t.kind == quit means that t is a quit Token
const char print = ';';        //t.kind == print means that t is a quit Token 
const char number = '8';       //t.kind == number means that t is a number Token
const char name = 'a';         //name Token
const char square_root = 's';  //square root Token
const char power = 'p';        //power function Token
const string declkey = "let";  //declaration keyword
const string sqrtkey = "sqrt"; //keyword for square root
const string powkey = "pow";   //keyword for power function 
const string quitkey = "quit"; //keyword to quit
const string prompt = "> ";    //used to indicate a prompt for entry
const string result = "= ";    //used to indicate that what follows is a result 

// read characters from cin and compose a Token
Token Token_stream::get()
{
	if (full)  // check if we already have a Token ready
    {    
		full = false;
        return buffer;
    }

	char ch;
	cin >> ch;

	switch (ch) {
    //case 'H':
    //    return Token(help);
    //case help:
    //case quit:
    case print:
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
    case '%':
    case '=':
    case ',':
		return Token(ch);   // let each character represent itself
	case '.':   // a floating-point-literal can start with a dot
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':   // numeric literal
	{	cin.putback(ch);    // put digit back into the input stream
		double val;
		cin >> val;         // read a floating-point number
		return Token(number,val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
			cin.putback(ch);
            if (s == declkey) return Token(let);            // declaration keyword
            if (s == sqrtkey) return Token(square_root);    // square root keyword
            if (s == powkey) return Token(power);           // power function keyword
            if (s == quitkey) return Token(quit);           // quit keyword
			return Token(name,s);
		}
		error("Bad token");
		return Token(' '); //Line missing
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
class Variable {
public:
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
	for (int i = 0; i<=names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ",s);
	return 0.0; //Line missing 
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

double define_name(string s, double d)
{
	if (is_declared(s)) error(s, " declared twice ");
	names.push_back(Variable(s, d));
	return d;
}

//--------------------------------------------------------------------

double expression();

//--------------------------------------------------------------------

// simple power function
// handles only integers >= 0 as exponents
double my_pow(double base, int expo)
{
    if (expo == 0)
	{
    	if (base == 0) return 0;    // special case: pow(0,0)
        return 1;                   // something to power of 0
    }
    double res = base;              // corresponds to power of 1
    for (int i = 2; i<=expo; ++i)   // powers of 2 and more
        res *= base;
    return res;
}

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	double d = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		{
			return d; //Line missing 
		}
	}
	case '-':
		return - primary();
	case '+':
		return primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);
	case square_root: //handle sqrt(expression)
	{
		t = ts.get();
		if(t.kind != '(') error("'(' expected");
		double d = expression();
		if (d < 0) error("Square root of negative numbers... nope!");
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		return sqrt(d);
	}
	case power: // handle 'pow(' expression ',' integer ')'
    {   
		t = ts.get();
        if (t.kind != '(') error("'(' expected");
        double d = expression();
        t = ts.get();
        if (t.kind != ',') error("',' expected");
        t = ts.get();
        if (t.kind != number) error("second argument of 'pow' is not a number");
        int i = int(t.value);
        if (i != t.value) error("second argument of 'pow' is not an integer");
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return my_pow(d,i);
    }
	default:
		error("primary expected");
		return 0.0; //Line missing 
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

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

void calculate()
{
	while(cin) try {  //bugfix
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
		define_name("pi", 3.1415926535);
		define_name("e", 2.7182818284);
		define_name("k", 1000);
		calculate();

		keep_window_open();
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