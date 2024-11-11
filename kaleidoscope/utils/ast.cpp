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


