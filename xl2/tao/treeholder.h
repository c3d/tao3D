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
    TreeHolder(XL::Tree *copy);
    TreeHolder(const TreeHolder& other);
    ~TreeHolder();

    XL::TreeRoot tree;

};
TAO_END

#define TREEHOLDER_TYPE 383 // (QVariant::UserType | 0x100)

Q_DECLARE_METATYPE(Tao::TreeHolder)

#endif // TREEHOLDER_H
