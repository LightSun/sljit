#pragma once

namespace h7 {

struct RTStatement{
    int reg; //register
    int dt;  //dataType.
    int op;  //load,save, INC...
};

class RuntimeStack
{
public:
    RuntimeStack();

    void addStatement();
};


}
