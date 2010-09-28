#include "tao.h"
#include "main.h"

XL_BEGIN
extern void (*refresh_fn)(double delay);
XL_END

TAO_BEGIN

void tao_widget_refresh(double delay);

struct Main : public XL::Main
{
    Main(int argc, char **argv, XL::Compiler *comp,
         text syntax = "xl.syntax",
         text style = "xl.stylesheet",
         text builtins = "builtins.xl")
         : XL::Main(argc, argv, comp, syntax, style, builtins)
    {
        XL::refresh_fn = tao_widget_refresh;
    }

    virtual text SearchFile(text input);
};

TAO_END
