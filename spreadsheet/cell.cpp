#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы

void Cell::Set(std::string text, SheetInterface& sheet, Position& pos) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN) {
        if (text.size() == 1) {
            impl_ = std::make_unique<TextImpl>(text);
        } else {
            impl_ = std::make_unique<FormulaImpl>(text.substr(1, text.size() - 1), sheet, pos);
        }
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    /*
        вызов Set требудет ссылку на таблицу, о которой сама ячейка не знает
        поэтому замена имплементации
    */
   impl_ = std::make_unique<EmptyImpl>(); 
}

Cell::Value Cell::GetValue() const {
    if (impl_.get() == nullptr) {
        return { };
    } 
    return impl_.get()->GetValue();
}

std::string Cell::GetText() const {
    if (impl_.get() == nullptr) {
        return { };
    } 
    return impl_.get()->GetText();
}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return {};
}

std::string Cell::EmptyImpl::GetText() const {
    return {};
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

Cell::Value Cell::TextImpl::GetValue() const {
    return text_[0] == '\'' ? text_.substr(1, text_.size() - 1) : text_;
}
std::string Cell::TextImpl::GetText() const {
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_.has_value()) {  
        try {
            auto val = formula_.get()->Evaluate(sheet_);
            cache_ =  val;
        } catch ( ... ) {
            throw FormulaError(FormulaError::Category::Value);
        }    
    }
    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
     } else {
        return std::get<FormulaError>(cache_.value());
    }
    return {};
}
std::string Cell::FormulaImpl::GetText() const {
    return "=" + formula_.get()->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return refered_cells_;
}

bool Cell::FormulaImpl::IsCycle(const SheetInterface& sheet, std::set<Position>& visited, std::set<Position>& stack, Position pos) const {
    if (stack.count(pos)) {
        return true;
    }
    auto cell = sheet.GetCell(pos);

    if (cell && !visited.count(pos)) {
        stack.insert(pos);
        visited.insert(pos);
        const auto & references = sheet.GetCell(pos)->GetReferencedCells();
        for (Position position : references) {
            if (position.IsValid() ){
                auto cell_ = sheet.GetCell(position);
                if (cell_) {
                    auto ref = cell_->GetReferencedCells();
                    if (IsCycle(sheet, visited, stack, position)) {
                        return true;
                    }
                }
            }
        }
        stack.erase(pos);
    }
    
    return false;
}
