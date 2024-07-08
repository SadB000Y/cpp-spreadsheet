#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text)  {
    if (!pos.IsValid()) {
        throw InvalidPositionException("bad set");
    }
    
    auto new_cell = std::make_unique<Cell>();
    new_cell.get()->Set(text, *this, pos);

    if (pos.row > last_actual_row_ || cell_list_.empty()) {
        cell_list_.resize(pos.row + 1);
        last_actual_row_ = pos.row;
        for (size_t i = static_cast<int>(last_actual_row_); i != cell_list_.size(); ++i) {
            cell_list_[i].resize(cell_list_.begin()->size() == 0 ? 1 : cell_list_.begin()->size());
        }
    }

    if (pos.col > last_actual_col_) { 
        last_actual_col_ = pos.col;
        for (size_t i = 0; i != cell_list_.size(); ++i) {
            cell_list_[i].resize(pos.col + 1);
        }
    }

    cell_list_[pos.row][pos.col] = std::move(new_cell);
};

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        return nullptr;
    }
    if (pos.row > last_actual_row_ || pos.col > last_actual_col_) {
        return nullptr;
    }
    if (cell_list_.size() > static_cast<size_t>(pos.row)) {
        if (cell_list_[pos.row].size() > static_cast<size_t>(pos.col) ) {
            return cell_list_.at(pos.row).at(pos.col).get();
        }
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("bad Get");
    }
    if (cell_list_.size() > static_cast<size_t>(pos.row)) {
        if (cell_list_[pos.row].size() > static_cast<size_t>(pos.col) ) {
            return cell_list_.at(pos.row).at(pos.col).get();
        }
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) { 
    if (!pos.IsValid()) {
        throw InvalidPositionException("bad Get");
    }

    int col = 1;
    int row = 1;
    if (pos.col <= last_actual_col_ && pos.row <= last_actual_row_) {
        last_actual_row_ = last_actual_col_ = 0;
        if (cell_list_.size() > static_cast<size_t>(pos.row)) {
            if (cell_list_[pos.row].size() > static_cast<size_t>(pos.col) ) {
                cell_list_.at(pos.row).at(pos.col).release();
            }
        }
        for (const auto & line : cell_list_) {
            col = 1;
            for (const auto & cell : line) {
                if (cell != nullptr) {
                    if (col > last_actual_col_) {
                        last_actual_col_ = col;
                    }
                    if (row > last_actual_row_) {
                        last_actual_row_ = row;
                    }
                }
                col++;
            }
            
            row++;
        }
        cell_list_.resize(last_actual_row_);
        for (auto & line : cell_list_) {
            line.resize(last_actual_col_);
        }
    }
    

}

Size Sheet::GetPrintableSize() const { 
    if (cell_list_.empty()) {
        return {0,0};
    }
    return {static_cast<int>(cell_list_.size()),
            static_cast<int>(cell_list_.begin()->size())};
}



inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}


void Sheet::PrintValues(std::ostream& output) const {

    for (const auto & line : cell_list_) {
        bool first = true;
        for (const auto & cell : line) {
            if (first) {
                first = false;
            } else {
                output << '\t';
            }
            if (cell != nullptr) {
                const auto & value = cell.get()->GetValue();
                output << value;
            }
        }
        output << '\n';
    }

}
void Sheet::PrintTexts(std::ostream& output) const {
        for (const auto & line : cell_list_) {
        bool first = true;
        for (const auto & cell : line) {
            if (first) {
                first = false;
            } else {
                output << '\t';
            }
            if (cell != nullptr) {
                output << cell.get()->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}