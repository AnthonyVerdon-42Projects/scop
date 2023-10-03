#include "Material.hpp"

MaterialParsingFunctions Material::parsingFunctions = {
        {"Ka", &Material::defineAmbiantColor},
        {"Ks", &Material::defineSpecularColor},
        {"Kd", &Material::defineDiffuseColor},
        {"Ns", &Material::defineSpecularExponent},
        {"Ni", &Material::defineRefractionIndex},
        {"d", &Material::defineOpacity},
        {"illum", &Material::defineIllum},
};

Material::Material()
{
}

Material::Material(const std::string &name)
{
    this->name = name;
    colors = new float*[3];
    for (size_t i = 0; i < 3; i++)
    {
        colors[i] = new float[3];
        for (size_t j = 0; j < 3; j++)
            colors[i][j] = 0;
    }
    specularExponent = 0;
    refractionIndex = 0;
    opacity = 1;
    illum = 1;
}

Material::Material(const Material &copy)
{
    colors = new float*[3];
    for (size_t j = 0; j < 3; j++)
        colors[j] = new float[3];
    *this = copy;
}

Material &Material::operator=(const Material &copy)
{
    if (this != &copy)
    {
        name = copy.getName();
        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 3; j++)
                colors[i][j] = copy.getColor(i)[j];
        }
        specularExponent = copy.getSpecularExponent();
        opacity = copy.getOpacity();
        illum = copy.getIllum();
    }
    return (*this);
}

Material::~Material()
{
    /*
    for (size_t j = 0; j < 3; j++)
        delete []colors[j];
    delete []colors;
    */
}

std::string Material::getName() const
{
    return (name);
}

float **Material::getColors() const
{
    return (colors);
}

float *Material::getColor(unsigned int index) const
{
    if (index >= 3)
        throw(Utils::Exception("MATERIAL::GET_COLOR::INVALID_INDEX"
        "\nINDEX => " + std::to_string(index)));
    return (colors[index]);
}

float Material::getSpecularExponent() const
{
    return (specularExponent);
}

float Material::getOpacity() const
{
    return (opacity);
}

float Material::getIllum() const
{
    return (illum);
}

void Material::setName(const std::string &name)
{
    this->name = name;
}

void Material::setColors(float colors[3][3])
{
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
            this->colors[i][j] = colors[i][j];
    }
}

void Material::setColor(float color[3], unsigned int index)
{
    if (index >= 3)
        throw(Utils::Exception("MATERIAL::SET_COLOR::INVALID_INDEX"
        "\nINDEX => " + std::to_string(index)));
    for (size_t j = 0; j < 3; j++)
        this->colors[index][j] = color[j];
}

void Material::setSpecularExponent(float specularExponent)
{
    this->specularExponent = specularExponent;
}

void Material::setRefractionIndex(float refractionIndex)
{
    this->refractionIndex = refractionIndex;
}

void Material::setOpacity(float opacity)
{
    this->opacity = opacity;
}

void Material::setIllum(float illum)
{
    this->illum = illum;
}

void Material::defineAmbiantColor(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 4)
        throw(Utils::Exception("MATERIAL::DEFINE_AMBIANT_COLOR::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 0; i < 3; i++)
    {
        colors[AMBIANT_COLOR][i] = std::stof(words[i + 1]);
        if (colors[AMBIANT_COLOR][i] < 0 || colors[AMBIANT_COLOR][i] > 1)
            throw(Utils::Exception("MATERIAL::DEFINE_AMBIANT_COLOR::INVALID_ARGUMENT"
            "\nLINE => " + line + "\n"
            "LINE INDEX => " + std::to_string(lineIndex)));
    }
}

void Material::defineSpecularColor(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 4)
        throw(Utils::Exception("MATERIAL::DEFINE_SPECULAR_COLOR::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 0; i < 3; i++)
    {
        colors[SPECULAR_COLOR][i] = std::stof(words[i + 1]);
        if (colors[SPECULAR_COLOR][i] < 0 || colors[SPECULAR_COLOR][i] > 1)
            throw(Utils::Exception("MATERIAL::DEFINE_SPECULAR_COLOR::INVALID_ARGUMENT"
            "\nLINE => " + line + "\n"
            "LINE INDEX => " + std::to_string(lineIndex)));
    }
}

void Material::defineDiffuseColor(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 4)
        throw(Utils::Exception("MATERIAL::DEFINE_DIFFUSE_COLOR::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 0; i < 3; i++)
    {
        colors[DIFFUSE_COLOR][i] = std::stof(words[i + 1]);
        if (colors[DIFFUSE_COLOR][i] < 0 || colors[DIFFUSE_COLOR][i] > 1)
            throw(Utils::Exception("MATERIAL::DEFINE_DIFFUSE_COLOR::INVALID_ARGUMENT"
            "\nLINE => " + line + "\n"
            "LINE INDEX => " + std::to_string(lineIndex)));
    }
}

void Material::defineSpecularExponent(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("MATERIAL::DEFINE_SPECULAR_EXPONENT::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

   specularExponent = std::stof(words[1]);
   if (specularExponent < 0 || specularExponent > 1000)
        throw(Utils::Exception("MATERIAL::DEFINE_SPECULAR_EXPONENT::INVALID_ARGUMENT"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));
}

void Material::defineRefractionIndex(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("MATERIAL::DEFINE_REFRACTION_INDEX::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

   refractionIndex = std::stof(words[1]);
   if (refractionIndex < 0 || refractionIndex > 10)
        throw(Utils::Exception("MATERIAL::DEFINE_REFRACTION_INDEX::INVALID_ARGUMENT"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));
}

void Material::defineOpacity(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("MATERIAL::DEFINE_OPACITY::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

   refractionIndex = std::stof(words[1]);
   if (refractionIndex < 0 || refractionIndex > 1)
        throw(Utils::Exception("MATERIAL::DEFINE_OPACITY::INVALID_ARGUMENT"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));
}

void Material::defineIllum(std::string line, unsigned int lineIndex)
{
    (void)line, (void)lineIndex;
}