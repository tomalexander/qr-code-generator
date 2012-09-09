#include <iostream>
#include <string>
#include "qr_code.h"
#include "bitmap.h"

using namespace std;

int main(int argc, char** argv)
{
    qr_code code("HELLO WORLD");
    bitmap bm(code.get_grid());
    bm.write_to_file("test.bmp");
    return 0;
}
