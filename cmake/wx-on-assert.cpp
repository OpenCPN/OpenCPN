// Compiling this one fails when using wxWidgets libraries
// without debug support in place.
#include "wx/debug.h"

int main(int argc, char** argv)
{
    wxOnAssert("path", 0, "func", "cond", "message");
}
