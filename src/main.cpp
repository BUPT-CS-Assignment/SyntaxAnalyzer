#include "../include/grammar.hpp"

char basic_filepath[] = "scripts/grammar.in";

int main(int argc, char** argv)
{
    char* path = basic_filepath;
    int mode = MODE_LL1;
    readArgs(argc,argv,&path,&mode);

    /* running */
    Grammar G;
    G.init(path);
    mode == MODE_LL1 ? G.runLL1() : G.runLR1();
    return 0;
}