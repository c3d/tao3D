#include "treeholder.h"

TAO_BEGIN

TreeHolder::TreeHolder()
    : tree(NULL)
{
}

TreeHolder::TreeHolder(XL::Tree *the_tree)
    : tree(the_tree)
{
}

TreeHolder::TreeHolder(const TreeHolder& other)
{
    this->tree = other.tree;
}

TreeHolder::~TreeHolder()
{
    tree = NULL;
}

TAO_END
