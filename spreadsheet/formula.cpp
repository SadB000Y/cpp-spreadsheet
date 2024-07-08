#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) 
        : ast_(ParseFormulaAST(expression)) 
    {   }
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (const FormulaError& exc) {
            return exc;
        }
        
     };
    std::string GetExpression() const override  { 
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    };

    std::vector<Position> GetReferencedCells() const override {
        auto ast_cells = ast_.GetCells();
        std::vector<Position> vec = {ast_cells.begin(), ast_cells.end()};
        vec.erase( unique( vec.begin(), vec.end() ), vec.end() );
        return vec;
    }

private:
    FormulaAST ast_;
    // std::vector<Position> referencedcell_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        auto cell_pointer = std::make_unique<Formula>(std::move(expression));
        cell_pointer.get();
        return cell_pointer;
    } catch (...) {
        throw FormulaException("Invalid position");
    }
}