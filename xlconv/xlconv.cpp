// ****************************************************************************
//  xlconv.cpp                                                     Tao project
// ****************************************************************************
// 
//   File Description:
// 
//     Convert between text-based XL source and binary-encoded XL source.
//
//
//
//
//
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "flight_recorder.h"
#include "main.h"
#include "parser.h"
#include "serializer.h"
#include "errors.h"
#include "syntax.h"
#include <iostream>
#include <stdlib.h>
#include <QStringList>
#include <QVector>

#ifdef Q_OS_WIN32
#include <fcntl.h>
unsigned int _CRT_fmode = _O_BINARY;
#endif

using namespace XL;

void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "  xlconv [-r] <infile >outfile\n";
    std::cerr << "Convert XL source from text to binary format. The program ";
    std::cerr << "gets its input\nfrom stdin and writes its output to ";
    std::cerr << "stdout.\n";
    std::cerr << "Options:\n";
    std::cerr << "  -h:    This help\n";
    std::cerr << "  -r:    (Reverse mode) Convert from binary to text.\n";
    std::cerr << "Examples:\n";
    std::cerr << "  xlconv    <file.xl     >file.xl.bin\n";
    std::cerr << "  xlconv -r <file.xl.bin >file.xl\n";
    std::cerr << "Bugs:\n";
    std::cerr << "- The files xl.syntax and xl.stylesheet MUST be present ";
    std::cerr << "in the current directory.\n";

}

int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Convert XL source between text and binary formats
// ----------------------------------------------------------------------------
{
    QStringList args;
    for (int i = 0; i < argc; i++)
        args << QString(argv[i]);

    if (args.contains("-h"))
    {
        usage();
        exit(0);
    }

    bool ropt = args.contains("-r");
    if (ropt)
        args.removeAll("-r");

    QVector<char *> xlargs;
    foreach (QString a, args)
        xlargs.append(strdup(a.toAscii().data()));

    FlightRecorder::Initialize();
    XL::MAIN = new Main(xlargs.size(), xlargs.data());

    if (!ropt)
    {
        // Text to binary

        Parser parser(std::cin, MAIN->syntax, MAIN->positions,
                      MAIN->topLevelErrors);
        Tree *tree;
        tree = parser.Parse();
        if (!tree)
        {
            std::cerr << "xlconv: parse error.\n";
            exit(1);
        }

        Serializer *writer;
        writer = new Serializer(std::cout);
        tree->Do(writer);
        std::cout << std::flush;
    }
    else
    {
        // Binary to text

        Deserializer *reader;
        reader = new Deserializer(std::cin);
        Tree *tree = reader->ReadTree();
        if (!reader->IsValid())
        {
            std::cerr << "xlconv: input is not in serialized format.\n";
            exit(1);
        }

        Renderer renderer(std::cout);
        renderer.Render(tree);
        std::cout << std::flush;
    }
    return 0;
}
