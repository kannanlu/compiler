#include <cstdio>
#include <string>

// ExprAST - Base class for all expression nodes.
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

// The following is for some testing
// int main(int argc, char *argv[])
// {
//     VariableExprAST v("abt");
//     printf("Expression is %s\n", v.get_val());
// }