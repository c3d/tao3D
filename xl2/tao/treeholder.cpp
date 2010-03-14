#include "treeholder.h"

TAO_BEGIN

TreeHolder::TreeHolder()
    : tree(NULL)
{
}

TreeHolder::TreeHolder(XL::Tree *copy)
    : tree(copy)
{
}

TreeHolder::TreeHolder(const TreeHolder& other)
    : tree(other.tree)
{
}

TreeHolder::~TreeHolder()
{
    tree = NULL;
}

TAO_END
