#pragma once

#include "../MaterialData/MaterialData.hpp"

class Material : public MaterialData
{
  public:
    Material();
    Material(const MaterialData &materialData);
    Material &operator=(const Material &copy);
    Material(const Material &copy);
    ~Material();
};

std::ostream &operator<<(std::ostream &os, const Material &instance);