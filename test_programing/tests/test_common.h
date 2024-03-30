#pragma once

#include "h7/Instruction.h"
#include "h7/DataStack.h"

namespace h7 {

static void printFloat(float val){
    printf("printFloat >> v = %g\n", val);
}
static void printInt(int val){
    printf("printInt >> val = %d\n", val);
}
static void printChar(char val){
    printf("printChar >> val = %d\n", val);
}

Sentence::Type addStat_printDSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                                   type, kPD_FLAG_DS, idx));
    return sent;
}
Sentence::Type addStat_printLSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                type, kPD_FLAG_LS, idx));
    return sent;
}

Sentence::Type addStat_printLSChar(int type, int idx){
    auto sent = Sentence::NewCall((void*)printChar);
    sent->addFunctionParameter(ParameterInfo::make(
                type, kPD_FLAG_LS, idx));
    return sent;
}

}
