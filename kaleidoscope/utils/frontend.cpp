#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

//===-------------------------------------------------------------===//
// Lexer
//===-------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for the known things.
enum Token
{
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

// gettok - Return the next token from standard input.
static int gettok()
{
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
    {
        LastChar = getchar();
    }

    if (isalpha(LastChar))
    { // identifier:[a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar())))
        {
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "def")
        {
            return tok_def;
        }

        if (IdentifierStr == "extern")
        {
            return tok_extern;
        }
        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.')
    { // Number [0-9.]+
        std::string NumStr;
        do
        {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    if (LastChar == '#')
    {
        // Comment until end of line.
        do
        {
            LastChar = getchar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
        {
            return gettok();
        }
    }

    // Check for the end of file. Don't eat the EOF.
    if (LastChar == EOF)
    {
        return tok_eof;
    }

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}

//===-------------------------------------------------------------===//
// AST
//===-------------------------------------------------------------===//
// ExprAST - Base class for all expression nodes.
namespace
{
    class ExprAST
    {
    private:
        /* data */
    public:
        // Default destructor of the compiler,
        // set it to virtual to properly destroy derived class.
        virtual ~ExprAST() = default;
    };

    // Numrical literals class
    class NumberExprAST : public ExprAST
    {
    private:
        double Val;

    public:
        NumberExprAST(double Val) : Val(Val) {}
        // double get_val() {return Val; }
    };

    // VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST
    {
    private:
        std::string Name;

    public:
        VariableExprAST(const std::string &Name) : Name(Name) {}
        std::string get_val() { return Name; }
    };

    // BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST
    {
    private:
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        // Transfer the ownership of expression node to the class members .
        // e.g. Op(Op) means initialize class member Op with parameter Op.
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op),
                                                                                             LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    };

    class CallExprAST : public ExprAST
    {
    private:
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}
    };

    // PrototypeAST - This class represents the "prototype" for a function,
    // which captures its name, and its argument names (thus implicitly the number
    // of arguments the function takes).
    class PrototypeAST
    {
    private:
        std::string Name;
        std::vector<std::string> Args;

    public:
        PrototypeAST(const std::string &Name, std::vector<std::string> Args) : Name(Name), Args(std::move(Args)) {}

        const std::string &getName() const { return Name; }
    };

    // FunctionAST - This class represents a function definition itself.
    class FunctionAST
    {
    private:
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(std::move(Proto)), Body(std::move(Body)) {}
    };
}
//===-------------------------------------------------------------===//
// Parser
//===-------------------------------------------------------------===//
// Some basic helper routines.
// CurTok/getNextToken - Provide a simple token buffer. CurTok is the current
// token the parser is looking at. getNextToken reads another token from the
// lexer and updates the CurTok with its results.
static int CurTok;
static int getNextToken()
{
    return CurTok = gettok();
}

// LogError* - These are little helper functions for error handling.
// Error handling of the expression node.
std::unique_ptr<ExprAST> LogError(const char *Str)
{
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr; // the nullptr is to propagate the error for signaling, this is common pattern in recursive descent parsers
}
// Error handling of the prototype.
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
    LogError(Str);
    return nullptr;
}

///////////////////////////////////////////////////////////////////////
///// Binary expr parsing
// Binary expression can be ambiguous, x+y*z can be (x+y)*z and x+(y*z), the later is
// mathematically correct. The method to use is the operator precedence parsing, which uses
// the binary operation to guide recursion. We thus need a table of precedences.

// BinopPrecedence - This holds the precedence for each binary operator that is defined.
static std::map<char, int> BinopPrecedence;

// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence()
{
    if (!isascii(CurTok))
    {
        return -1;
    }
    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
    {
        return -1;
    }
    return TokPrec;
}

static std::unique_ptr<ExprAST> ParseExpression();

///////////////////////////////////////////////////////////////////////
///// Base expr parsing
// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr()
{
    auto Result = std::make_unique<NumberExprAST>(NumVal); // make_unique performs allocation and smart pointer construction in one step
    getNextToken();                                        // consume the number, standard in recursive descent parsers
    return Result;
}

// parenexpr ::= '(' expression ')', the parenthesis operator
static std::unique_ptr<ExprAST> ParseParenExpr()
{
    getNextToken(); // eat ( .
    auto V = ParseExpression();
    if (!V)
    {
        return nullptr;
    }
    if (CurTok != ')')
    {
        return LogError("expected ')'");
    }
    getNextToken(); // eat ) .
    return V;
}

// identifierexpr
// ::= identifier
// ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
    std::string IdName = IdentifierStr;
    getNextToken(); // eat identifier. if no (expr), it is a variable.
    if (CurTok != '(')
    {
        return std::make_unique<VariableExprAST>(IdName);
    }
    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')')
    {
        while (true)
        {
            if (auto Arg = ParseExpression())
            {
                Args.push_back(std::move(Arg));
            }
            else
            {
                return nullptr;
            }
            if (CurTok == ')')
            {
                break;
            }
            if (CurTok != ',')
            {
                return LogError("Expected ')' or ',' in argument list");
            }
            getNextToken(); // eat
        }
    }

    // eat the ')'. The Args contain the expr in (expr), it is a call expression.
    getNextToken();
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// Primary expression
// ::= identifierexpr
// ::= numberexpr
// ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary()
{
    switch (CurTok)
    {
    default:
    {
        return LogError("Unknown token when expecting an expression");
    }
    case tok_identifier:
    {
        return ParseIdentifierExpr();
    }
    case tok_number:
    {
        return ParseNumberExpr();
    }
    case '(':
    {
        return ParseParenExpr();
    }
    }
}

// the BinopPrecedence can be initialized
// 1 is the lowest
// BinopPrecedence['<'] = 10;
// BinopPrecedence['+'] = 20;
// BinopPrecedence['-'] = 20;
// BinopPrecedence['*'] = 40;

// Consider expression a+b+(c+d)*e*f+g , the parser will see "a" first,
// then + b ... , so we parse and expression of primary expression
// followed by [binop, primaryexpr] pairs
//  ::= primary binoprhs
/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS)
{
    // If this is a binop, find its precedence.
    while (true)
    {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec)
        {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        // Merge LHS/RHS.
        LHS =
            std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression()
{
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

// Function prototype
// ::=id'('id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype()
{
    if (CurTok != tok_identifier)
    {
        return LogErrorP("Expected function name in prototype");
    }
    std::string FnName = IdentifierStr;
    getNextToken(); // eat function name
    if (CurTok != '(')
    {
        return LogErrorP("Expected '(' in prototype");
    }
    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
    {
        ArgNames.push_back(IdentifierStr);
    }
    if (CurTok != ')')
    {
        return LogErrorP("Expected ')' in prototype");
    }

    getNextToken(); // succeed and eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}