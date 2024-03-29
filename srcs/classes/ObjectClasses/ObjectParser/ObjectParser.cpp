#include "ObjectParser.hpp"
#include "../../MaterialClasses/MaterialParser/MaterialParser.hpp"
#include "../../Utils/Utils.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
MapObjectParsingMethods ObjectParser::parsingMethods = {{"v", &ObjectParser::defineVertex},
                                                        {"f", &ObjectParser::defineFace},
                                                        {"s", &ObjectParser::defineSmoothShading},
                                                        {"mtllib", &ObjectParser::saveNewMTL},
                                                        {"usemtl", &ObjectParser::defineMTL}};

std::vector<Material> ObjectParser::materials;

std::vector<Object> ObjectParser::parseObjectFile(const std::string &path)
{
    if (!Utils::checkExtension(path, ".obj"))
        throw(Exception("PARSE_OBJECT_FILE", "INVALID_EXTENSION", path, 0));

    ObjectData objectData;
    std::vector<Object> objects;
    std::stringstream fileStream = Utils::readFile(path);
    fileStream = Utils::readFile(path);
    std::string line;
    unsigned int lineIndex = 1;
    while (std::getline(fileStream, line))
    {
        line = line.substr(0, line.find("#"));
        std::string symbol = line.substr(0, line.find(" "));
        auto it = parsingMethods.find(symbol);
        if (symbol == "o")
        {
            if (objectData.getFaces().size() > 0)
            {
                Object newObject(objectData);
                objects.push_back(newObject);
            }
            objectData.reset();
            defineName(objectData, line, lineIndex);
        }
        else if (it != parsingMethods.end())
            (it->second)(objectData, line, lineIndex);
        else if (symbol.length() != 0)
            throw(Exception("PARSE_OBJECT_FILE", "INVALID_SYMBOL", line, lineIndex));
        lineIndex++;
    }
    if (objectData.getFaces().size() > 0)
    {
        Object newObject(objectData);
        objects.push_back(newObject);
    }
    return (objects);
}

void ObjectParser::defineName(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Exception("DEFINE_NAME", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));
    objectData.setName(words[1]);
}

void ObjectParser::defineVertex(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    Vertex vertex;
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() < 4 || words.size() > 5)
        throw(Exception("DEFINE_VERTEX", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));

    for (size_t i = 1; i < words.size(); i++)
    {
        if (!Utils::isFloat(words[i]))
            throw(Exception("DEFINE_VERTEX", "INVALID_ARGUMENT", line, lineIndex));
        vertex.push_back(std::stof(words[i]));
    }
    if (words.size() == 4)
        vertex.push_back(1.0f);

    if (vertex[3] == 0.0f)
        throw(Exception("DEFINE_VERTEX", "INVALID_ARGUMENT", line, lineIndex));

    for (size_t i = 0; i < 3; i++)
        vertex[i] = vertex[i] / vertex[3];

    objectData.addVertex(vertex);
}

void ObjectParser::defineFace(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    Face face;
    int vertexID;
    const int nbVertices = static_cast<int>(objectData.getVertices().size());
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() < 4)
        throw(Exception("DEFINE_FACE", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));

    for (size_t i = 1; i < words.size(); i++)
    {
        if (!Utils::isInt(words[i]))
            throw(Exception("DEFINE_FACE", "INVALID_ARGUMENT", line, lineIndex));
        vertexID = std::stoi(words[i]);
        if (vertexID < -nbVertices || vertexID > nbVertices || vertexID == 0)
            throw(Exception("DEFINE_FACE", "INVALID_VERTEX_INDEX", line, lineIndex));

        if (vertexID < 0)
            vertexID = nbVertices + 1 + vertexID;
        face.push_back(vertexID - 1);
    }
    triangulate(objectData, face);
}

void ObjectParser::triangulate(ObjectData &objectData, Face &face)
{
    while (face.size() > 3)
    {
        for (size_t i = 1; i < face.size() - 1; i++)
        {
            Vertex a = objectData.getVertices()[face[i]];
            Vertex b = objectData.getVertices()[face[i - 1]];
            Vertex c = objectData.getVertices()[face[i + 1]];

            bool isEar = true;
            for (size_t j = 0; j < objectData.getVertices().size(); j++)
            {
                Vertex p = objectData.getVertices()[j];
                if (p == a || p == b || p == c)
                    continue;
                if (insideTriangle(p, a, b, c))
                {
                    isEar = false;
                    break;
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
                objectData.addFace(newFace);
                break;
            }
        }
    }
    objectData.addFace(face);
}

bool ObjectParser::insideTriangle(const Vertex &p, const Vertex &a, const Vertex &b, const Vertex &c)
{
    float areaABC = triangleArea(a, b, c);
    float areaABP = triangleArea(a, b, p);
    float areaBCP = triangleArea(b, c, p);
    float areaACP = triangleArea(a, c, p);

    if (areaABC == areaABP + areaBCP + areaACP && areaABP != 0 && areaBCP != 0 && areaACP != 0)
        return (true);
    else
        return (false);
}

float ObjectParser::triangleArea(const Vertex &a, const Vertex &b, const Vertex &c)
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

void ObjectParser::defineSmoothShading(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Exception("DEFINE_SMOOTH_SHADING", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));

    if (words[1] == "on" || words[1] == "1")
        objectData.setSmoothShading(true);
    else if (words[1] == "off" || words[1] == "0")
        objectData.setSmoothShading(false);
    else
        throw(Exception("DEFINE_SMOOTH_SHADING", "INVALID_ARGUMENT", line, lineIndex));
}

void ObjectParser::saveNewMTL(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    (void)objectData;
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Exception("CREATE_NEW_MTL", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));

    std::vector<Material> newMaterials = MaterialParser::parseMaterialFile(words[1]);
    materials.insert(materials.end(), newMaterials.begin(), newMaterials.end());
}

void ObjectParser::defineMTL(ObjectData &objectData, const std::string &line, unsigned int lineIndex)
{
    std::vector<std::string> words;

    words = Utils::splitLine(line);
    if (words.size() != 2)
        throw(Exception("DEFINE_MTL", "INVALID_NUMBER_OF_ARGUMENTS", line, lineIndex));

    for (size_t i = 0; i < materials.size(); i++)
    {
        if (materials[i].getName() == words[1])
        {
            objectData.setMaterial(materials[i]);
            return;
        }
    }
    throw(Exception("DEFINE_MTL", "INVALID_ARGUMENT", line, lineIndex));
}

ObjectParser::Exception::Exception(const std::string &functionName, const std::string &errorMessage,
                                   const std::string &line, unsigned int lineIndex)
{
    this->errorMessage = "\nOBJECT_PARSER::" + functionName + "::" + errorMessage;
    this->errorMessage += "\n|\n| " + std::to_string(lineIndex) + ": " + line + "\n|";
}

const char *ObjectParser::Exception::what(void) const throw()
{
    return (errorMessage.c_str());
}