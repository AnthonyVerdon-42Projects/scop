#include "Object.hpp"

ObjectParsingFunctions Object::parsingFunctions = {
        {"s", &Object::defineSmoothShading},
        {"v", &Object::defineVertex},
        {"f", &Object::defineFace},
        {"mtllib", &Object::defineMTL},
        {"usemtl", &Object::useMTL}
};

std::vector<std::unique_ptr<Material> > Object::materials;

Object::Object()
{
}

Object::Object(const std::string &name)
{
    this->name = name;
    actualMaterial = "none";
    smoothShading = false;
    VAOInit = false;
}


Object::Object(const Object &copy)
{
    *this = copy;
}

Object &Object::operator=(const Object &copy)
{
    if (&copy != this)
    {
        vertices = copy.getVertices();
        faces = copy.getFaces();
        name = copy.getName();
        smoothShading = copy.getSmoothShading();
        materialDictionnary = copy.getMaterialDictionnary();
    }
    return (*this);
}

Object::~Object()
{
    if (VAOInit)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

std::vector<std::vector<float> >Object::getVertices() const
{
    return (vertices);
}

std::unique_ptr<float[]> Object::getVerticesIntoArray() const
{
    size_t j;
    std::unique_ptr<float[]> array;

    j = 0;
    array = std::make_unique<float[]>(vertices.size() * 4);
    for (size_t i = 0; i < vertices.size(); i++)
    {
        array[j] = vertices[i][0];
        array[j + 1] = vertices[i][1];
        array[j + 2] = vertices[i][2];
        array[j + 3] = vertices[i][3];
        j += 4;
    }
    return (array);
}

std::unique_ptr<unsigned int[]> Object::getFacesIntoArray() const
{
    size_t j;
    std::unique_ptr<unsigned int[]> array;

    j = 0;
    array = std::make_unique<unsigned int[]>(faces.size() * 3);
    for (size_t i = 0; i < faces.size(); i++)
    {
        array[j] = faces[i][0];
        array[j + 1] = faces[i][1];
        array[j + 2] = faces[i][2];
        j += 3;
    }

    return (array);
}

std::vector<Face> Object::getFaces() const
{
    return (faces);
}

std::string Object::getName() const
{
    return (name);
}

bool Object::getSmoothShading() const
{
    return (smoothShading);
}

unsigned int Object::getVAO() const
{
    return (VAO);
}

void Object::setName(const std::string &name)
{
    this->name = name;
}

void Object::setVertices(const Vertices &vertices)
{
    this->vertices = vertices;
}

void Object::setFaces(const std::vector<Face> &faces)
{
    this->faces = faces;
}

void Object::setSmoothShading(bool smoothShading)
{
    this->smoothShading = smoothShading;
}

void Object::defineVertex(std::string line, unsigned int lineIndex)
{
    Vertex vertex;
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() < 4 || words.size() > 5)
        throw(Utils::Exception("OBJECT::DEFINE_VERTEX::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 1; i < words.size(); i++)
        vertex.push_back(std::stof(words[i]));
    if (words.size() == 4)
        vertex.push_back(1.0f);

    vertices.push_back(vertex);
}

float Object::triangleArea(const Vertex &a, const Vertex &b, const Vertex &c) const
{
    Vertex AB;
    AB.push_back(b[0] - a[0]);
    AB.push_back(b[1] - a[1]);
    AB.push_back(b[2] - a[2]);

    Vertex AC;
    AC.push_back(c[0] - a[0]);
    AC.push_back(c[1] - a[1]);
    AC.push_back(c[2] - a[2]);

    float dotProduct = AB[0] * AC[0] + AB[1] * AC[1] + AB[2] * AC[2];
    float magnitudeAB = sqrt(pow(AB[0], 2) + pow(AB[1], 2) + pow(AB[2], 2));
    float magnitudeAC = sqrt(pow(AC[0], 2) + pow(AC[1], 2) + pow(AC[2], 2));
    float cosAngle = dotProduct / (magnitudeAB * magnitudeAC);
    float angle = acos(cosAngle);
    float area = 0.5f * magnitudeAB * magnitudeAC * sin(angle);
    return (area);
}

bool Object::insideTriangle(const Vertex &p, const Vertex &a, const Vertex &b, const Vertex &c) const
{
    float areaABC = triangleArea(a, b, c);
    float areaABP = triangleArea(a, b, p);
    float areaBCP = triangleArea(b, c, p);
    float areaACP = triangleArea(a, c, p);

    if (areaABC == areaABP + areaBCP + areaACP
        && areaABP != 0 && areaBCP != 0 && areaACP != 0)
        return (true);
    else
        return (false);
}

void Object::triangulate(Face &face)
{
    while (face.size() > 3)
    {
        for (size_t i = 1; i < face.size() - 1; i++)
        {
            Vertex a = vertices[face[i]];
            Vertex b = vertices[face[i - 1]];
            Vertex c = vertices[face[i + 1]];

            bool isEar = true;
            for (size_t j = 0; j < vertices.size(); j++)
            {
                Vertex p = vertices[j];
                if (p == a || p == b ||  p == c)
                    continue;
                if (insideTriangle(p, a, b, c))
                {
                    isEar = false;
                    break ;
                }
            }
            if (isEar)
            {
                Face newFace;

                newFace.push_back(face[i - 1]);
                newFace.push_back(face[i]);
                newFace.push_back(face[i + 1]);
                for (std::vector<int>::iterator it = face.begin(); it != face.end(); it++)
                {
                    if (*it == static_cast<int>(face[i]))
                    {
                        face.erase(it);
                        break;
                    }
                }
                if (actualMaterial != "none")
                    materialDictionnary[actualMaterial].push_back(newFace);
                faces.push_back(newFace);
                break ;
            }
        }
    }
    if (actualMaterial != "none")
        materialDictionnary[actualMaterial].push_back(face);
    faces.push_back(face);
}

void Object::defineFace(std::string line, unsigned int lineIndex)
{
    Face face;
    int vertexID;
    const int nbVertices = static_cast<int>(vertices.size());
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() < 4)
        throw(Utils::Exception("OBJECT::DEFINE_FACE::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 1; i < words.size(); i++)
    {
        vertexID = std::stoi(words[i]);
        if (vertexID < -nbVertices || vertexID > nbVertices)
            throw(Utils::Exception("OBJECT::DEFINE_FACE::INVALID_VERTEX_INDEX"
            "\nLINE => " + line + "\n"
            "LINE INDEX => " + std::to_string(lineIndex)));

        if (vertexID < 0)
            vertexID = nbVertices + 1 + vertexID;
        face.push_back(vertexID - 1);
    }
    triangulate(face);
}

void Object::defineSmoothShading(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("OBJECT::DEFINE_SMOOTH_SHADING::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    if (words[1] == "on" || words[1] == "1")
        smoothShading = true;
    else if (words[1] == "off" || words[1] == "0")
        smoothShading = false;
    else
        throw(Utils::Exception("OBJECT::DEFINE_SMOOTH_SHADING::INVALID_ARGUMENT"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));
}

void Object::defineMTL(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("OBJECT::DEFINE_MTL::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    parseMtlFile(words[1]);
}

void Object::useMTL(std::string line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Utils::Exception("OBJECT::USE_MTL::INVALID_NUMBER_OF_ARGUMENTS"
        "\nLINE => " + line + "\n"
        "LINE INDEX => " + std::to_string(lineIndex)));

    for (size_t i = 0; i < materials.size(); i++)
    {
        if (materials[i]->getName() == words[1])
        {
            actualMaterial = words[1];
            return ;
        }
    }
    throw(Utils::Exception("OBJECT::USE_MTL::UNKNOWN_MATERIAL"
    "\nMATERIAL NAME => " + words[1]));
}

std::unique_ptr<float []> Object::convertEBOintoVBO()
{
    std::unique_ptr<float []> verticesArray;

    verticesArray = std::make_unique<float[]>(faces.size() * 3 * 4);
    size_t j = 0;
    for (size_t i = 0; i < faces.size(); i++)
    {
        for (size_t x = 0; x < 3; x++)
        {
            verticesArray[j] = vertices[faces[i][x]][0];
            verticesArray[j + 1] = vertices[faces[i][x]][1];
            verticesArray[j + 2] = vertices[faces[i][x]][2];
            verticesArray[j + 3] = vertices[faces[i][x]][3];
            j += 4;
        }
    }
    return (verticesArray);
}

void Object::initVAO()
{
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //if we want to draw with EBO
    
    std::unique_ptr<float[]> verticesArray;
    std::unique_ptr<unsigned int[]> facesArray;

    verticesArray = getVerticesIntoArray();
    facesArray = getFacesIntoArray();

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() * 4, &verticesArray[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * faces.size() * 3, &facesArray[0], GL_STATIC_DRAW);
    
    /*
    //if we want to draw face by face for multiples materials
    std::unique_ptr<float []> verticesArray;

    verticesArray = convertEBOintoVBO();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * faces.size() * 3 * 4, &verticesArray[0], GL_STATIC_DRAW);
    */

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    VAOInit = true;
}

std::ostream& operator << (std::ostream &os, const Object &instance)
{
    os << "name: " << instance.getName() << std::endl;
    os << "smooth shading: " << instance.getSmoothShading() << std::endl;
    Vertices objectVertices = instance.getVertices();
    os << "vertices: " << std::endl;
    for (size_t i = 0; i < objectVertices.size(); i++)
    {
        os << "vertex nb " << i + 1 << ": ";
        os << objectVertices[i][0] << " " << objectVertices[i][1] << " ";
        os << objectVertices[i][2] << " " << objectVertices[i][3]<< std::endl;
    }
    std::vector<Face> objectFace = instance.getFaces();
    os << "faces: " << std::endl;
    for (size_t i = 0; i < objectFace.size(); i++)
    {
        os << "face nb " << i + 1 << ": ";
        for (size_t j = 0; j < objectFace[i].size(); j++)
            os << objectFace[i][j] << " ";
        os << std::endl;
    }
    return (os);
}

void Object::setMaterialDictionnary(std::map<std::string,std::vector<Face>> materialDictionnary)
{
    this->materialDictionnary = materialDictionnary;
}

std::map<std::string,std::vector<Face>> Object::getMaterialDictionnary() const
{
    return (materialDictionnary);
}

void Object::addMaterial(const std::unique_ptr<Material> &material)
{
    materials.push_back(std::make_unique<Material>(*material));
}

const std::vector<std::unique_ptr<Material> > &Object::getMaterials()
{
    return (materials);
}

Material Object::getMaterial(const std::string &name)
{
    Material material("default");

    for (auto it = materials.begin(); it != materials.end(); it++)
    {
            if ((*it)->getName() == name)
            {
                material = **it;
                break ;
            }
    }
    return (material);
}

Material Object::getMaterial(unsigned int i) const
{
    Material material("default");

    for (auto it = materialDictionnary.begin(); it != materialDictionnary.end(); it++)
    {
        if (std::find(it->second.begin(), it->second.end(), faces[i]) != it->second.end())
        {
            material = Object::getMaterial(it->first);
            break ;
        }
    }
    return (material);
}