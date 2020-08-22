#include <GL/glew.h>
#include <fstream>
#include <sstream>

namespace shader
{
    struct source {
        std::string VertexSource;
        std::string FragmentSource;
    };

    source parse(const std::string& filepath);

    unsigned int compile(unsigned int type, const std::string& source);

    unsigned int create(const source& shader);
}