/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "SkSLExpression.h"
#include "SkSLFloatLiteral.h"
#include "SkSLIntLiteral.h"
#include "SkSLIRGenerator.h"

namespace SkSL {

/**
 * Represents the construction of a compound type, such as "vec2(x, y)".
 */
struct Constructor : public Expression {
    Constructor(Position position, const Type& type,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(position, kConstructor_Kind, type)
    , fArguments(std::move(arguments)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 && fArguments[0]->fKind == Expression::kIntLiteral_Kind) {
            if (fType == *irGenerator.fContext.fFloat_Type) {
                // promote float(1) to 1.0
                int64_t intValue = ((IntLiteral&) *fArguments[0]).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(irGenerator.fContext,
                                                                    fPosition,
                                                                    intValue));
            } else if (fType == *irGenerator.fContext.fUInt_Type) {
                // promote uint(1) to 1u
                int64_t intValue = ((IntLiteral&) *fArguments[0]).fValue;
                return std::unique_ptr<Expression>(new IntLiteral(irGenerator.fContext,
                                                                  fPosition,
                                                                  intValue,
                                                                  &fType));
            }
        }
        return nullptr;
    }

    bool hasSideEffects() const override {
        for (const auto& arg : fArguments) {
            if (arg->hasSideEffects()) {
                return true;
            }
        }
        return false;
    }

    SkString description() const override {
        SkString result = fType.description() + "(";
        SkString separator;
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isConstant() const override {
        for (size_t i = 0; i < fArguments.size(); i++) {
            if (!fArguments[i]->isConstant()) {
                return false;
            }
        }
        return true;
    }

    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
