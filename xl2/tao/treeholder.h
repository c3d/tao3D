#ifndef TREEHOLDER_H
#define TREEHOLDER_H
#include "tao.h"
#include "tree.h"
#include <QMetaType>

TAO_BEGIN
struct TreeHolder
{
public:
    TreeHolder();
    TreeHolder(XL::Tree *the_tree);
    TreeHolder(const TreeHolder& other);
    ~TreeHolder();

    XL::Tree *tree;
};
TAO_END

Q_DECLARE_METATYPE(Tao::TreeHolder)

#endif // TREEHOLDER_H
